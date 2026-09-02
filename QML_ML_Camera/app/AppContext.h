#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>
#include <QUrl>

#include "AlbumModel.h"
#include "CaptureCoordinator.h"
#include "DetectionModel.h"
#include "LoggerModel.h"
#include "MovieModel.h"
#include "PictureModel.h"
#include "PluginManager.h"
#include "SessionModel.h"
#include "SessionSpectrumModel.h"

// Typed composition-root facade for the objects created in main.cpp. QML
// imports one registered singleton instead of relying on untyped context
// properties that static tooling cannot discover or complete.
class AppContext final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AlbumModel* albumModel READ albumModel CONSTANT)
    Q_PROPERTY(PictureModel* pictureModel READ pictureModel CONSTANT)
    Q_PROPERTY(MovieModel* movieModel READ movieModel CONSTANT)
    Q_PROPERTY(LoggerModel* loggerModel READ loggerModel CONSTANT)
    Q_PROPERTY(SessionModel* sessionModel READ sessionModel CONSTANT)
    Q_PROPERTY(SessionSpectrumModel* sessionSpectrumModel READ sessionSpectrumModel CONSTANT)
    Q_PROPERTY(PluginManager* pluginModel READ pluginModel CONSTANT)
    Q_PROPERTY(CaptureCoordinator* captureCoordinator READ captureCoordinator CONSTANT)
    Q_PROPERTY(DetectionModel* detectionModel READ detectionModel CONSTANT)
    Q_PROPERTY(QObject* reportService READ reportService CONSTANT)
    Q_PROPERTY(QUrl logsUrl READ logsUrl CONSTANT)
    Q_PROPERTY(int thumbnailSize READ thumbnailSize CONSTANT)
    Q_PROPERTY(bool visionAvailable READ visionAvailable CONSTANT)
    Q_PROPERTY(bool aiAvailable READ aiAvailable CONSTANT)

public:
    AppContext(AlbumModel& albumModel, PictureModel& pictureModel,
               MovieModel& movieModel, LoggerModel& loggerModel,
               SessionModel& sessionModel,
               SessionSpectrumModel& sessionSpectrumModel,
               PluginManager& pluginModel,
               CaptureCoordinator& captureCoordinator,
               DetectionModel& detectionModel, QObject* reportService,
               const QUrl& logsUrl, int thumbnailSize,
               bool visionAvailable, bool aiAvailable,
               QObject* parent = nullptr);

    AlbumModel* albumModel() const { return m_albumModel; }
    PictureModel* pictureModel() const { return m_pictureModel; }
    MovieModel* movieModel() const { return m_movieModel; }
    LoggerModel* loggerModel() const { return m_loggerModel; }
    SessionModel* sessionModel() const { return m_sessionModel; }
    SessionSpectrumModel* sessionSpectrumModel() const { return m_sessionSpectrumModel; }
    PluginManager* pluginModel() const { return m_pluginModel; }
    CaptureCoordinator* captureCoordinator() const { return m_captureCoordinator; }
    DetectionModel* detectionModel() const { return m_detectionModel; }
    QObject* reportService() const { return m_reportService; }
    QUrl logsUrl() const { return m_logsUrl; }
    int thumbnailSize() const { return m_thumbnailSize; }
    bool visionAvailable() const { return m_visionAvailable; }
    bool aiAvailable() const { return m_aiAvailable; }

private:
    AlbumModel* m_albumModel;
    PictureModel* m_pictureModel;
    MovieModel* m_movieModel;
    LoggerModel* m_loggerModel;
    SessionModel* m_sessionModel;
    SessionSpectrumModel* m_sessionSpectrumModel;
    PluginManager* m_pluginModel;
    CaptureCoordinator* m_captureCoordinator;
    DetectionModel* m_detectionModel;
    QObject* m_reportService;
    QUrl m_logsUrl;
    int m_thumbnailSize;
    bool m_visionAvailable;
    bool m_aiAvailable;
};

#endif // APPCONTEXT_H
