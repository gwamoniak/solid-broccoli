#include "AppContext.h"

AppContext::AppContext(AlbumModel& albumModel, PictureModel& pictureModel,
                       MovieModel& movieModel, LoggerModel& loggerModel,
                       SessionModel& sessionModel,
                       SessionSpectrumModel& sessionSpectrumModel,
                       PluginManager& pluginModel,
                       CaptureCoordinator& captureCoordinator,
                       DetectionModel& detectionModel, QObject* reportService,
                       const QUrl& logsUrl, int thumbnailSize,
                       bool visionAvailable, bool aiAvailable, QObject* parent)
    : QObject(parent)
    , m_albumModel(&albumModel)
    , m_pictureModel(&pictureModel)
    , m_movieModel(&movieModel)
    , m_loggerModel(&loggerModel)
    , m_sessionModel(&sessionModel)
    , m_sessionSpectrumModel(&sessionSpectrumModel)
    , m_pluginModel(&pluginModel)
    , m_captureCoordinator(&captureCoordinator)
    , m_detectionModel(&detectionModel)
    , m_reportService(reportService)
    , m_logsUrl(logsUrl)
    , m_thumbnailSize(thumbnailSize)
    , m_visionAvailable(visionAvailable)
    , m_aiAvailable(aiAvailable)
{
}
