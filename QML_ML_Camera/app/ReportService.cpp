#include "ReportService.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

#include "AiAnalyst.h"
#include "AppSettings.h"
#include "DatabaseManager.h"
#include "DetectionModel.h"
#include "ReportContextBuilder.h"
#include "SpectrometerService.h"
#include "loggingcategories.h"

ReportService::ReportService(DatabaseManager& db, SpectrometerService& spectrometer,
                             DetectionModel& detections, AppSettings& settings,
                             QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_spectrometer(spectrometer)
    , m_detections(detections)
    , m_settings(settings)
    , m_analyst(new AiAnalyst(this))
{
    connect(m_analyst, &AiAnalyst::tokenGenerated, this, [this](const QString& token) {
        m_streamText += token;
        emit streamTextChanged();
    });
    connect(m_analyst, &AiAnalyst::generationFinished, this,
            [this](const QString& fullText) {
                m_db.reportDao().addReport(m_pendingSessionId, m_analyst->modelName(),
                                           promptVersion(), m_pendingContextJson,
                                           fullText);
                qDebug(logInfo()) << "ReportService: report saved for session"
                                  << m_pendingSessionId;
                setBusy(false);
                emit reportSaved(m_pendingSessionId);
            });
    connect(m_analyst, &AiAnalyst::modelLoadFinished, this,
            [this](bool ok, const QString& error) {
                emit modelChanged();
                if (!ok) {
                    setBusy(false);
                    emit errorOccurred(error);
                    return;
                }
                if (m_pendingSessionId >= 0)
                    startGeneration();
            });
    connect(m_analyst, &AiAnalyst::errorOccurred, this, [this](const QString& message) {
        setBusy(false);
        emit errorOccurred(message);
    });
    connect(&m_settings, &AppSettings::aiModelPathChanged, this, [this]() {
        // A newly chosen file takes effect on the next generation.
        m_analyst->unloadModel();
        emit modelChanged();
    });
}

QString ReportService::modelName() const
{
    if (m_analyst->isModelLoaded())
        return m_analyst->modelName();
    return QFileInfo(m_settings.aiModelPath()).fileName();
}

bool ReportService::modelConfigured() const
{
    return !m_settings.aiModelPath().isEmpty();
}

void ReportService::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    if (!busy)
        m_pendingSessionId = -1;
    emit busyChanged();
}

QString ReportService::buildContextJson(int sessionId) const
{
    SessionRecord session;
    for (const SessionRecord& record : m_db.sessionDao().sessions()) {
        if (record.id == sessionId) {
            session = record;
            break;
        }
    }

    const QString instrument =
        m_spectrometer.availableDevices().value(m_spectrometer.currentDeviceIndex());
    const QJsonObject context = ReportContextBuilder::build(
        session, m_db.spectrumDao().spectra(sessionId),
        m_db.measurementDao().measurements(sessionId),
        m_detections.latestDetections(), instrument);
    return QString::fromUtf8(
        QJsonDocument(context).toJson(QJsonDocument::Compact));
}

void ReportService::generateReport(int sessionId)
{
    if (m_busy) {
        emit errorOccurred(tr("A report is already being generated."));
        return;
    }
    if (!modelConfigured()) {
        emit errorOccurred(tr("No AI model configured — pick a GGUF in Settings."));
        return;
    }

    m_pendingSessionId = sessionId;
    m_pendingContextJson = buildContextJson(sessionId);
    m_streamText.clear();
    emit streamTextChanged();
    setBusy(true);

    if (m_analyst->isModelLoaded())
        startGeneration();
    else
        m_analyst->loadModel(m_settings.aiModelPath());  // generation chains after
}

void ReportService::startGeneration()
{
    QFile templateFile(QStringLiteral(":/prompts/report_v1.md"));
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setBusy(false);
        emit errorOccurred(tr("Prompt template missing from resources."));
        return;
    }
    QString prompt = QString::fromUtf8(templateFile.readAll());
    prompt.replace(QStringLiteral("{{CONTEXT}}"), m_pendingContextJson);
    m_analyst->generate(prompt);
}

void ReportService::cancel()
{
    m_analyst->cancel();
    setBusy(false);
}
