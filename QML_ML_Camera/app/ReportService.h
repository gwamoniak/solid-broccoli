#ifndef REPORTSERVICE_H
#define REPORTSERVICE_H

#include <QObject>
#include <QString>

class AiAnalyst;
class AppSettings;
class DatabaseManager;
class DetectionModel;
class SpectrometerService;

// The report workflow, QML-facing (only built with -DAI_ANALYST=ON): builds
// the grounded context JSON for a session, feeds it through the prompt
// template into AiAnalyst, streams tokens into streamText for the viewer,
// and persists the finished report (with its exact context, for audit) via
// ReportDAO. Loads the configured GGUF lazily on the first generation.
class ReportService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY modelChanged)
    Q_PROPERTY(bool modelConfigured READ modelConfigured NOTIFY modelChanged)
    Q_PROPERTY(QString streamText READ streamText NOTIFY streamTextChanged)

public:
    ReportService(DatabaseManager& db, SpectrometerService& spectrometer,
                  DetectionModel& detections, AppSettings& settings,
                  QObject* parent = nullptr);

    bool busy() const { return m_busy; }
    QString modelName() const;
    bool modelConfigured() const;
    QString streamText() const { return m_streamText; }

    static QString promptVersion() { return QStringLiteral("report_v1"); }

    Q_INVOKABLE void generateReport(int sessionId);
    Q_INVOKABLE void cancel();

signals:
    void busyChanged();
    void modelChanged();
    void streamTextChanged();
    void reportSaved(int sessionId);
    void errorOccurred(const QString& message);

private:
    QString buildContextJson(int sessionId) const;
    void startGeneration();
    void setBusy(bool busy);

    DatabaseManager& m_db;
    SpectrometerService& m_spectrometer;
    DetectionModel& m_detections;
    AppSettings& m_settings;
    AiAnalyst* m_analyst;

    bool m_busy = false;
    int m_pendingSessionId = -1;
    QString m_pendingContextJson;
    QString m_streamText;
};

#endif // REPORTSERVICE_H
