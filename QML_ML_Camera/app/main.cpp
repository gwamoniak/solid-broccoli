#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>

#include "AlbumModel.h"
#include "PictureModel.h"
#include "MovieModel.h"
#include "MovieAlbumModel.h"
#include "logger.h"
#include "LoggerModel.h"
#include "PictureProvider.h"
#include "CameraProcessor.h"
#include "MovieProcessor.h"


int main(int argc, char *argv[])
{
  // qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
   // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    Logger log;
    if(log.InitLogger())
    {
        qDebug(logInfo()) << "Application has started!";

    }

    QUrl appPath(QDir::currentPath());


    AlbumModel   albumModel;
    MovieAlbumModel movieAlbumModel;
    PictureModel pictureModel(albumModel);
    MovieModel   movieModel(movieAlbumModel);

    LoggerModel loggerModel;

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();


    qmlRegisterType<CameraProcessor>("solid.broccoli", 1, 0, "CameraProcessor");
    qmlRegisterType<MovieProcessor>("solid.broccoli", 1, 0,   "MovieProcessor");
    context->setContextProperty("thumbnailSize",PictureProvider::THUMBNAIL_SIZE.width());
    context->setContextProperty("albumModel",   &albumModel);
    context->setContextProperty("movieAlbumModel",   &movieAlbumModel);
    context->setContextProperty("pictureModel", &pictureModel);
    context->setContextProperty("movieModel",   &movieModel);
    context->setContextProperty("loggerModel", &loggerModel);

    context->setContextProperty("appPath", appPath);
    engine.addImageProvider("pictures", new PictureProvider(&pictureModel));
    engine.addImageProvider("movies", new MovieProvider(&movieModel));
    context->setContextProperty("movieProcessor", new MovieProcessor(&movieModel));




    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    qDebug(logInfo()) << "Application has exited!";
    return app.exec();
}
