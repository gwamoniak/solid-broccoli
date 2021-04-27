#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>

#include "AlbumModel.h"
#include "PictureModel.h"
#include "logger.h"
#include "LoggerModel.h"
#include "PictureProvider.h"
#include "CameraProcessor.h"


// icons from
//<div>Icons made by <a href="https://www.flaticon.com/authors/srip" title="srip">srip</a>
//from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>

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
    PictureModel pictureModel(albumModel);


    LoggerModel loggerModel;

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();


    qmlRegisterType<CameraProcessor>("solid.broccoli", 1, 0, "CameraProcessor");
    context->setContextProperty("thumbnailSize",PictureProvider::THUMBNAIL_SIZE.width());
    context->setContextProperty("albumModel",   &albumModel);
    context->setContextProperty("pictureModel", &pictureModel);
    context->setContextProperty("loggerModel", &loggerModel);

    context->setContextProperty("appPath", appPath);
    engine.addImageProvider("pictures", new PictureProvider(&pictureModel));


    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    qDebug(logInfo()) << "Application has exited!";
    return app.exec();
}
