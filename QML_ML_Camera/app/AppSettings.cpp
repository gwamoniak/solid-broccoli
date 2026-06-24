#include "AppSettings.h"

#include "logger.h"

// Defaults preserve the app's prior behavior: the shutter flash was always on,
// and the preview was never mirrored.
AppSettings::AppSettings(QObject* parent)
    : QObject(parent),
      m_shutterFlash(m_settings.value("ui/shutterFlash", true).toBool()),
      m_mirrorPreview(m_settings.value("ui/mirrorPreview", false).toBool())
{
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
