#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
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


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("SolidBroccoli");
    QCoreApplication::setApplicationName("SolidBroccoli");

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
    CameraService cameraService;

    QObject::connect(&cameraService, &CameraService::recordingSaved,
                     &movieModel, &MovieModel::addRecording);

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "CameraService", &cameraService);
    context->setContextProperty("thumbnailSize", PictureProvider::THUMBNAIL_SIZE.width());
    context->setContextProperty("albumModel",   &albumModel);
    context->setContextProperty("pictureModel", &pictureModel);
    context->setContextProperty("loggerModel",  &loggerModel);
    context->setContextProperty("movieModel",   &movieModel);
    context->setContextProperty("logsPath", QUrl::fromLocalFile(StorageLocations::logsDir()));
    engine.addImageProvider("pictures", new PictureProvider(&pictureModel));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
