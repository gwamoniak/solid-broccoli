#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <QObject>
#include <QStringList>
#include <QCamera>
#include <QCameraDevice>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaRecorder>

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

public:
    explicit CameraService(QObject* parent = nullptr);

    QStringList availableCameras() const;
    int currentCameraIndex() const;
    void setCurrentCameraIndex(int index);

    QStringList availableFormats() const;
    int currentFormatIndex() const;
    void setCurrentFormatIndex(int index);

    bool isActive() const;

    Q_INVOKABLE void attachVideoOutput(QObject* videoOutput);
    Q_INVOKABLE void setActive(bool active);
    Q_INVOKABLE void captureImage();

signals:
    void availableCamerasChanged();
    void currentCameraIndexChanged();
    void availableFormatsChanged();
    void currentFormatIndexChanged();
    void activeChanged();
    void imageSaved(const QString& filePath);
    void captureError(const QString& message);

private:
    void refreshDevices();
    void applyCamera(int index);
    void refreshFormats();
    QString nextPicturePath() const;

    QMediaDevices m_mediaDevices;
    QList<QCameraDevice> m_devices;
    QList<QCameraFormat> m_formats;
    int m_currentCameraIndex = -1;
    int m_currentFormatIndex = -1;

    QMediaCaptureSession m_captureSession;
    std::unique_ptr<QCamera> m_camera;
    QImageCapture m_imageCapture;
    QMediaRecorder m_recorder; // attached now, used from Milestone 3 on
};

#endif // CAMERASERVICE_H
