#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "AlbumModel.h"
#include "PictureModel.h"
#include "MovieModel.h"
#include "DatabaseManager.h"
#include "StorageLocations.h"
#include "logger.h"
#include "LoggerModel.h"
#include "PictureProvider.h"
#include "CameraService.h"
#include "SpectrometerService.h"
#include "SpectrumView.h"
#include "CaptureCoordinator.h"
#include "PluginManager.h"
#include "AppSettings.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("SolidBroccoli");
    QCoreApplication::setApplicationName("SolidBroccoli");

    // The UI is fully custom-themed from Theme.qml tokens; the native macOS
    // style ignores contentItem/background customization, so pin the Basic
    // style on every platform.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    Logger log;
    if (log.InitLogger(StorageLocations::logsDir()))
    {
        qDebug(logInfo()) << "Application has started!";
    }
    qDebug(logInfo()) << "Application data root:" << StorageLocations::root();

    DatabaseManager db;
    if (!db.isOpen()) {
        qWarning(logCritical()) << "Could not open database:"
                                << DatabaseManager::defaultDatabasePath();
    }

    AlbumModel   albumModel(db);
    PictureModel pictureModel(db, albumModel);
    LoggerModel  loggerModel(db);
    MovieModel   movieModel(db);

    // The plugin manager must outlive CameraService: the service's worker
    // thread runs processor code that lives in the loaded plugin libraries,
    // and the manager unloads those libraries in its destructor.
    PluginManager pluginManager;
    CameraService cameraService;

    QObject::connect(&cameraService, &CameraService::recordingSaved,
                     &movieModel, &MovieModel::addRecording);

    // Captured stills are registered into the current album as they are saved.
    CaptureCoordinator captureCoordinator(cameraService, pictureModel);

    // Discover frame-processor plugins and keep the camera pipeline's active
    // processor list in sync with the user's enable/disable choices.
    pluginManager.loadPlugins();
    QObject::connect(&pluginManager, &PluginManager::enabledProcessorsChanged,
                     &cameraService, &CameraService::setProcessors);
    cameraService.setProcessors(pluginManager.enabledProcessors());

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

    AppSettings appSettings;
    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "AppSettings", &appSettings);

    SpectrometerService spectrometerService;
    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "SpectrometerService",
                                 &spectrometerService);
    qmlRegisterType<SpectrumView>("solid.broccoli", 1, 0, "SpectrumView");

    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "CameraService", &cameraService);
    context->setContextProperty("thumbnailSize", PictureProvider::THUMBNAIL_SIZE.width());
    context->setContextProperty("albumModel",   &albumModel);
    context->setContextProperty("pictureModel", &pictureModel);
    context->setContextProperty("loggerModel",  &loggerModel);
    context->setContextProperty("movieModel",   &movieModel);
    context->setContextProperty("pluginModel",  &pluginManager);
    context->setContextProperty("logsPath", QUrl::fromLocalFile(StorageLocations::logsDir()));
    engine.addImageProvider("pictures", new PictureProvider(&pictureModel));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
