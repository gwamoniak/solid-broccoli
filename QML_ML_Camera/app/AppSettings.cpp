#include "AppSettings.h"

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
      m_geigerAlertThreshold(m_settings.value("geiger/alertThreshold", 0.5).toDouble())
{
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
