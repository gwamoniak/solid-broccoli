#include <QDir>
#include <QGuiApplication>
#include <QLibrary>
#include <QPluginLoader>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "AlbumModel.h"
#include "PictureModel.h"
#include "MovieModel.h"
#include "SessionModel.h"
#include "SessionSpectrumModel.h"
#include "DatabaseManager.h"
#include "StorageLocations.h"
#include "logger.h"
#include "LoggerModel.h"
#include "PictureProvider.h"
#include "CameraService.h"
#include "DetectionModel.h"
#include "FrameProcessor.h"
#include "GeigerService.h"
#include "SpectrometerService.h"
#include "SpectrumOverlayProcessor.h"
#include "SpectrumView.h"
#include "StripChartView.h"
#include "CaptureCoordinator.h"
#include "PluginManager.h"
#include "AppSettings.h"


// The object-detection plugin lives in <app>/vision/, not the generic
// plugins/ scan directory: it is loaded explicitly here and composed into
// the pipeline by the Settings VISION toggle, like the spectrum overlay —
// one switch, one owner. Returns null when the plugin was not built (ONNX
// Runtime absent) or fails to load; the app runs identically without it.
static FrameProcessor* loadDetectionProcessor()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList dirs = {
        QDir::cleanPath(appDir + "/vision"),
        QDir::cleanPath(appDir + "/../../../vision"),  // beside the .app bundle
    };
    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;
        const QStringList files = dir.entryList(QDir::Files);
        for (const QString& file : files) {
            const QString absolute = dir.absoluteFilePath(file);
            if (!QLibrary::isLibrary(absolute))
                continue;
            auto* loader = new QPluginLoader(absolute, QCoreApplication::instance());
            if (loader->metaData().value("IID").toString()
                != QLatin1String(FrameProcessor_iid)) {
                delete loader;
                continue;
            }
            auto* processor = qobject_cast<FrameProcessor*>(loader->instance());
            QString error;
            if (processor && processor->initialize(&error)) {
                qDebug(logInfo()) << "Vision: detection plugin loaded:" << absolute;
                return processor;
            }
            qWarning(logWarning()) << "Vision: detection plugin rejected:" << absolute
                                   << error << loader->errorString();
            loader->unload();
            delete loader;
        }
    }
    qDebug(logInfo()) << "Vision: no detection plugin present (built without "
                         "ONNX Runtime, or not found).";
    return nullptr;
}

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
    SessionModel sessionModel(db);
    SessionSpectrumModel sessionSpectrumModel(db);

    // The plugin manager must outlive CameraService: the service's worker
    // thread runs processor code that lives in the loaded plugin libraries,
    // and the manager unloads those libraries in its destructor.
    PluginManager pluginManager;

    // Constructed before CameraService so the overlay (and the service whose
    // snapshot it reads) outlive the camera worker thread that calls it.
    AppSettings appSettings;
    SpectrometerService spectrometerService;
    spectrometerService.setSessionStore(&sessionModel, &sessionSpectrumModel);

    GeigerService geigerService;
    // Measurements land in the same lab-notebook session as spectral
    // captures; the provider creates the session on first save.
    geigerService.setMeasurementStore(
        &db.m_measurementDao,
        [&spectrometerService]() { return spectrometerService.ensureActiveSession(); });
    // Tube factor and alert threshold persist across restarts.
    geigerService.setTubeFactor(appSettings.geigerTubeFactor());
    geigerService.setAlertThreshold(appSettings.geigerAlertThreshold());
    QObject::connect(&geigerService, &GeigerService::tubeFactorChanged,
                     &appSettings, [&]() {
                         appSettings.setGeigerTubeFactor(geigerService.tubeFactor());
                     });
    QObject::connect(&geigerService, &GeigerService::alertThresholdChanged,
                     &appSettings, [&]() {
                         appSettings.setGeigerAlertThreshold(geigerService.alertThreshold());
                     });

    SpectrumOverlayProcessor overlayProcessor(
        [&spectrometerService]() { return spectrometerService.latestSpectrumSnapshot(); });
    overlayProcessor.setGeigerStatusProvider([&geigerService]() {
        const GeigerService::OverlayStatus s = geigerService.overlayStatus();
        return SpectrumOverlayProcessor::GeigerStatus{s.active, s.alert,
                                                      s.doseMicroSvPerHour,
                                                      s.countsPerMinute};
    });

    CameraService cameraService;

    QObject::connect(&cameraService, &CameraService::recordingSaved,
                     &movieModel, &MovieModel::addRecording);

    // Documentation videos recorded while a spectroscopy session is active
    // are linked to that session.
    QObject::connect(&cameraService, &CameraService::recordingSaved,
                     &spectrometerService,
                     [&db, &spectrometerService](const QString& filePath, qint64 durationMs) {
                         const int sessionId = spectrometerService.activeSessionId();
                         if (sessionId >= 0)
                             db.m_sessionDao.addVideo(sessionId, filePath, durationMs);
                     });

    // Captured stills are registered into the current album as they are saved.
    CaptureCoordinator captureCoordinator(cameraService, pictureModel);

    // Live detections for the QML chip row; results arrive queued from
    // whatever thread the detector published on.
    DetectionModel detectionModel;
    auto deliverResult = [&detectionModel](const QVariantMap& result) {
        QMetaObject::invokeMethod(
            &detectionModel,
            [&detectionModel, result]() { detectionModel.updateFromResult(result); },
            Qt::QueuedConnection);
    };

    // Discover frame-processor plugins and keep the camera pipeline's active
    // processor list in sync with the user's enable/disable choices.
    pluginManager.setResultSink(deliverResult);
    pluginManager.loadPlugins();

    FrameProcessor* detectionProcessor = loadDetectionProcessor();
    if (detectionProcessor)
        detectionProcessor->setResultSink(deliverResult);

    // The preview/record pipeline is the enabled plugins plus, when toggled,
    // the object detector and the app-internal spectrum overlay.
    auto rebuildPipeline = [&pluginManager, &appSettings, &overlayProcessor,
                            &cameraService, &detectionModel, detectionProcessor]() {
        QList<FrameProcessor*> processors = pluginManager.enabledProcessors();
        if (detectionProcessor && appSettings.objectDetection()) {
            detectionProcessor->configure(
                {{QStringLiteral("modelPath"), appSettings.detectionModelPath()},
                 {QStringLiteral("stride"), appSettings.detectionStride()}});
            processors.append(detectionProcessor);
        } else {
            detectionModel.clear();  // no stale chips once detection is off
        }
        if (appSettings.spectrumOverlay())
            processors.append(&overlayProcessor);
        cameraService.setProcessors(processors);
    };
    QObject::connect(&pluginManager, &PluginManager::enabledProcessorsChanged,
                     &cameraService,
                     [rebuildPipeline](const QList<FrameProcessor*>&) { rebuildPipeline(); });
    QObject::connect(&appSettings, &AppSettings::spectrumOverlayChanged,
                     &cameraService, rebuildPipeline);
    QObject::connect(&appSettings, &AppSettings::objectDetectionChanged,
                     &cameraService, rebuildPipeline);
    QObject::connect(&appSettings, &AppSettings::detectionModelPathChanged,
                     &cameraService, rebuildPipeline);
    QObject::connect(&appSettings, &AppSettings::detectionStrideChanged,
                     &cameraService, rebuildPipeline);
    rebuildPipeline();

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "AppSettings", &appSettings);
    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "SpectrometerService",
                                 &spectrometerService);
    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "GeigerService", &geigerService);
    qmlRegisterType<SpectrumView>("solid.broccoli", 1, 0, "SpectrumView");
    qmlRegisterType<StripChartView>("solid.broccoli", 1, 0, "StripChartView");

    qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "CameraService", &cameraService);
    context->setContextProperty("thumbnailSize", PictureProvider::THUMBNAIL_SIZE.width());
    context->setContextProperty("albumModel",   &albumModel);
    context->setContextProperty("pictureModel", &pictureModel);
    context->setContextProperty("loggerModel",  &loggerModel);
    context->setContextProperty("movieModel",   &movieModel);
    context->setContextProperty("sessionModel", &sessionModel);
    context->setContextProperty("sessionSpectrumModel", &sessionSpectrumModel);
    context->setContextProperty("pluginModel",  &pluginManager);
    context->setContextProperty("detectionModel", &detectionModel);
    context->setContextProperty("visionAvailable", detectionProcessor != nullptr);
    context->setContextProperty("logsPath", QUrl::fromLocalFile(StorageLocations::logsDir()));
    engine.addImageProvider("pictures", new PictureProvider(&pictureModel));

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
