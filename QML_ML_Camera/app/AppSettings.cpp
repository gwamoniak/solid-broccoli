#include "AppSettings.h"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "StorageLocations.h"
#include "logger.h"

// Defaults preserve the app's prior behavior: the shutter flash was always on,
// and the preview was never mirrored.
AppSettings::AppSettings(QObject* parent)
    : QObject(parent),
      m_shutterFlash(m_settings.value("ui/shutterFlash", true).toBool()),
      m_mirrorPreview(m_settings.value("ui/mirrorPreview", false).toBool()),
      m_spectrumOverlay(m_settings.value("ui/spectrumOverlay", false).toBool()),
      // 0.0057 µSv/h per CPM: the common SBM-20 tube approximation.
      m_geigerTubeFactor(m_settings.value("geiger/tubeFactor", 0.0057).toDouble()),
      m_geigerAlertThreshold(m_settings.value("geiger/alertThreshold", 0.5).toDouble()),
      m_objectDetection(m_settings.value("vision/objectDetection", false).toBool()),
      m_detectionModelPath(m_settings.value("vision/modelPath").toString()),
      m_detectionStride(m_settings.value("vision/stride", 3).toInt()),
      m_aiModelPath(m_settings.value("ai/modelPath").toString())
{
}

QString AppSettings::aiModelPath() const
{
    return m_aiModelPath;
}

QString AppSettings::aiModelName() const
{
    return m_aiModelPath.isEmpty() ? QString() : QFileInfo(m_aiModelPath).fileName();
}

void AppSettings::setAiModelFromUrl(const QUrl& source)
{
    const QString path = source.isLocalFile() ? source.toLocalFile() : source.toString();
    if (path == m_aiModelPath) {
        return;
    }
    m_aiModelPath = path;
    m_settings.setValue("ai/modelPath", path);
    qDebug(logInfo()) << "AI model selected:" << path;
    emit aiModelPathChanged();
}

bool AppSettings::objectDetection() const
{
    return m_objectDetection;
}

void AppSettings::setObjectDetection(bool enabled)
{
    if (m_objectDetection == enabled) {
        return;
    }
    m_objectDetection = enabled;
    m_settings.setValue("vision/objectDetection", enabled);
    qDebug(logInfo()) << "Setting objectDetection =" << enabled;
    emit objectDetectionChanged();
}

QString AppSettings::detectionModelPath() const
{
    return m_detectionModelPath;
}

QString AppSettings::detectionModelName() const
{
    return m_detectionModelPath.isEmpty() ? QString()
                                          : QFileInfo(m_detectionModelPath).fileName();
}

int AppSettings::detectionStride() const
{
    return m_detectionStride;
}

void AppSettings::setDetectionStride(int stride)
{
    stride = qBound(1, stride, 10);
    if (m_detectionStride == stride) {
        return;
    }
    m_detectionStride = stride;
    m_settings.setValue("vision/stride", stride);
    qDebug(logInfo()) << "Setting detectionStride =" << stride;
    emit detectionStrideChanged();
}

bool AppSettings::importDetectionModel(const QUrl& source)
{
    const QString sourcePath = source.isLocalFile() ? source.toLocalFile()
                                                    : source.toString();
    QFile input(sourcePath);
    if (!input.open(QIODevice::ReadOnly)) {
        const QString message = tr("Cannot read %1").arg(sourcePath);
        qWarning(logWarning()) << "Model import failed:" << message;
        emit modelImportFailed(message);
        return false;
    }

    const QByteArray contents = input.readAll();
    const QString checksum = QString::fromLatin1(
        QCryptographicHash::hash(contents, QCryptographicHash::Sha256).toHex());

    const QString target = StorageLocations::modelsDir() + QStringLiteral("/")
        + QFileInfo(sourcePath).fileName();
    QFile output(target);
    if (!output.open(QIODevice::WriteOnly) || output.write(contents) != contents.size()) {
        const QString message = tr("Cannot write %1").arg(target);
        qWarning(logWarning()) << "Model import failed:" << message;
        emit modelImportFailed(message);
        return false;
    }
    output.close();

    m_detectionModelPath = target;
    m_settings.setValue("vision/modelPath", target);
    m_settings.setValue("vision/modelSha256", checksum);
    qDebug(logInfo()) << "Detection model imported:" << target << "sha256" << checksum;
    emit detectionModelPathChanged();
    return true;
}

void AppSettings::revealModelsDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(StorageLocations::modelsDir()));
}

double AppSettings::geigerTubeFactor() const
{
    return m_geigerTubeFactor;
}

void AppSettings::setGeigerTubeFactor(double factor)
{
    if (qFuzzyCompare(m_geigerTubeFactor, factor)) {
        return;
    }
    m_geigerTubeFactor = factor;
    m_settings.setValue("geiger/tubeFactor", factor);
    qDebug(logInfo()) << "Setting geigerTubeFactor =" << factor;
    emit geigerTubeFactorChanged();
}

double AppSettings::geigerAlertThreshold() const
{
    return m_geigerAlertThreshold;
}

void AppSettings::setGeigerAlertThreshold(double threshold)
{
    if (qFuzzyCompare(m_geigerAlertThreshold, threshold)) {
        return;
    }
    m_geigerAlertThreshold = threshold;
    m_settings.setValue("geiger/alertThreshold", threshold);
    qDebug(logInfo()) << "Setting geigerAlertThreshold =" << threshold;
    emit geigerAlertThresholdChanged();
}

bool AppSettings::spectrumOverlay() const
{
    return m_spectrumOverlay;
}

void AppSettings::setSpectrumOverlay(bool enabled)
{
    if (m_spectrumOverlay == enabled) {
        return;
    }
    m_spectrumOverlay = enabled;
    m_settings.setValue("ui/spectrumOverlay", enabled);
    qDebug(logInfo()) << "Setting spectrumOverlay =" << enabled;
    emit spectrumOverlayChanged();
}

bool AppSettings::shutterFlash() const
{
    return m_shutterFlash;
}

void AppSettings::setShutterFlash(bool enabled)
{
    if (m_shutterFlash == enabled) {
        return;
    }
    m_shutterFlash = enabled;
    m_settings.setValue("ui/shutterFlash", enabled);
    qDebug(logInfo()) << "Setting shutterFlash =" << enabled;
    emit shutterFlashChanged();
}

bool AppSettings::mirrorPreview() const
{
    return m_mirrorPreview;
}

void AppSettings::setMirrorPreview(bool enabled)
{
    if (m_mirrorPreview == enabled) {
        return;
    }
    m_mirrorPreview = enabled;
    m_settings.setValue("ui/mirrorPreview", enabled);
    qDebug(logInfo()) << "Setting mirrorPreview =" << enabled;
    emit mirrorPreviewChanged();
}
