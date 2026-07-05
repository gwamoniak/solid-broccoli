#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <atomic>

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QThread>
#include <QCamera>
#include <QCameraDevice>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QVideoSink>

class FrameProcessor;
class FrameProcessingWorker;
class QVideoFrame;

// Single C++ owner of the capture pipeline. QML never constructs
// camera objects: it binds to these properties, attaches its
// VideoOutput, and calls the invokables.
class CameraService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableCameras READ availableCameras NOTIFY availableCamerasChanged)
    Q_PROPERTY(int currentCameraIndex READ currentCameraIndex WRITE setCurrentCameraIndex NOTIFY currentCameraIndexChanged)
    Q_PROPERTY(QStringList availableFormats READ availableFormats NOTIFY availableFormatsChanged)
    Q_PROPERTY(int currentFormatIndex READ currentFormatIndex WRITE setCurrentFormatIndex NOTIFY currentFormatIndexChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(QString recordingDuration READ recordingDuration NOTIFY recordingDurationChanged)

public:
    explicit CameraService(QObject* parent = nullptr);
    ~CameraService() override;

    QStringList availableCameras() const;
    int currentCameraIndex() const;
    void setCurrentCameraIndex(int index);

    QStringList availableFormats() const;
    int currentFormatIndex() const;
    void setCurrentFormatIndex(int index);

    bool isActive() const;
    bool isRecording() const;
    QString recordingDuration() const;

    Q_INVOKABLE void attachVideoOutput(QObject* videoOutput);
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void captureImage();
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

public slots:
    // Receives the ordered list of enabled processors from PluginManager and
    // rebuilds the preview pipeline. With an empty list the camera writes
    // straight to the QML video output (zero overhead); otherwise frames are
    // tapped, processed on a worker thread, and forwarded to the output.
    void setProcessors(const QList<FrameProcessor*>& processors);

signals:
    void availableCamerasChanged();
    void currentCameraIndexChanged();
    void availableFormatsChanged();
    void currentFormatIndexChanged();
    void activeChanged();
    void recordingChanged();
    void recordingDurationChanged();
    void imageSaved(const QString& filePath);
    void captureError(const QString& message);
    void recordingSaved(const QString& filePath, qint64 durationMs);

private:
    void refreshDevices();
    void applyCamera(int index);
    void refreshFormats();
    void rebuildPipeline();
    void onFrameChanged(const QVideoFrame& frame);
    void onFrameProcessed(const QImage& result);
    QString nextPicturePath() const;
    QString nextRecordingPath() const;

    QMediaDevices m_mediaDevices;
    QList<QCameraDevice> m_devices;
    QList<QCameraFormat> m_formats;
    int m_currentCameraIndex = -1;
    int m_currentFormatIndex = -1;

    QMediaCaptureSession m_captureSession;
    std::unique_ptr<QCamera> m_camera;
    QImageCapture m_imageCapture;
    QMediaRecorder m_recorder;
    qint64 m_lastRecordingDurationMs = 0;

    // Frame-processor pipeline. When m_hasProcessors is true the session
    // renders into m_processingSink (a tap), frames are handed to the worker
    // thread, and processed images are pushed to m_outputSink (the QML
    // VideoOutput's sink). When false, the session renders to m_videoOutput
    // directly and the tap is idle.
    QVideoSink m_processingSink;
    QPointer<QObject> m_videoOutput;
    QPointer<QVideoSink> m_outputSink;
    QThread m_workerThread;
    FrameProcessingWorker* m_worker = nullptr;
    bool m_hasProcessors = false;
    std::atomic<bool> m_frameBusy{false};
};

#endif // CAMERASERVICE_H
