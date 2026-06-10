#include "CameraService.h"

#include <QDateTime>
#include <QMediaFormat>
#include <QUrl>
#include <QVideoFrameFormat>

#include "StorageLocations.h"
#include "logger.h"

CameraService::CameraService(QObject* parent)
    : QObject(parent)
{
    m_captureSession.setImageCapture(&m_imageCapture);
    m_captureSession.setRecorder(&m_recorder);

    connect(&m_imageCapture, &QImageCapture::imageSaved,
            this, [this](int, const QString& fileName) {
        qDebug(logInfo()) << "Image saved:" << fileName;
        emit imageSaved(fileName);
    });
    connect(&m_imageCapture, &QImageCapture::errorOccurred,
            this, [this](int, QImageCapture::Error, const QString& message) {
        qWarning(logWarning()) << "Image capture error:" << message;
        emit captureError(message);
    });

    connect(&m_recorder, &QMediaRecorder::recorderStateChanged,
            this, [this](QMediaRecorder::RecorderState state) {
        emit recordingChanged();
        if (state == QMediaRecorder::StoppedState
                && m_recorder.error() == QMediaRecorder::NoError
                && m_lastRecordingDurationMs > 0) {
            const QString filePath = m_recorder.actualLocation().toLocalFile();
            qDebug(logInfo()) << "Recording saved:" << filePath
                              << "duration(ms):" << m_lastRecordingDurationMs;
            emit recordingSaved(filePath, m_lastRecordingDurationMs);
        }
    });
    connect(&m_recorder, &QMediaRecorder::durationChanged,
            this, [this](qint64 duration) {
        m_lastRecordingDurationMs = duration;
        emit recordingDurationChanged();
    });
    connect(&m_recorder, &QMediaRecorder::errorOccurred,
            this, [this](QMediaRecorder::Error, const QString& message) {
        qWarning(logWarning()) << "Recording error:" << message;
        emit captureError(message);
    });

    connect(&m_mediaDevices, &QMediaDevices::videoInputsChanged,
            this, &CameraService::refreshDevices);

    refreshDevices();
}

QStringList CameraService::availableCameras() const
{
    QStringList names;
    for (const QCameraDevice& device : m_devices) {
        names.append(device.description());
    }
    return names;
}

int CameraService::currentCameraIndex() const
{
    return m_currentCameraIndex;
}

void CameraService::setCurrentCameraIndex(int index)
{
    if (index == m_currentCameraIndex || index < 0 || index >= m_devices.size()) {
        return;
    }
    applyCamera(index);
}

QStringList CameraService::availableFormats() const
{
    QStringList names;
    for (const QCameraFormat& format : m_formats) {
        names.append(QStringLiteral("%1x%2 @ %3 fps (%4)")
                         .arg(format.resolution().width())
                         .arg(format.resolution().height())
                         .arg(qRound(format.maxFrameRate()))
                         .arg(QVideoFrameFormat::pixelFormatToString(format.pixelFormat())));
    }
    return names;
}

int CameraService::currentFormatIndex() const
{
    return m_currentFormatIndex;
}

void CameraService::setCurrentFormatIndex(int index)
{
    if (!m_camera || index == m_currentFormatIndex
            || index < 0 || index >= m_formats.size()) {
        return;
    }
    m_currentFormatIndex = index;
    m_camera->setCameraFormat(m_formats.at(index));
    qDebug(logInfo()) << "Camera format set:" << availableFormats().at(index);
    emit currentFormatIndexChanged();
}

bool CameraService::isActive() const
{
    return m_camera && m_camera->isActive();
}

void CameraService::attachVideoOutput(QObject* videoOutput)
{
    m_captureSession.setVideoOutput(videoOutput);
}

void CameraService::setActive(bool active)
{
    if (!m_camera) {
        return;
    }
    m_camera->setActive(active);
}

void CameraService::captureImage()
{
    if (m_imageCapture.isReadyForCapture()) {
        m_imageCapture.captureToFile(nextPicturePath());
    } else {
        const QString message = tr("Camera is not ready to capture.");
        qWarning(logWarning()) << message;
        emit captureError(message);
    }
}

bool CameraService::isRecording() const
{
    return m_recorder.recorderState() == QMediaRecorder::RecordingState;
}

QString CameraService::recordingDuration() const
{
    const int totalSeconds = static_cast<int>(m_lastRecordingDurationMs / 1000);
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'));
}

void CameraService::startRecording()
{
    if (isRecording()) {
        return;
    }
    if (!isActive()) {
        const QString message = tr("Camera is not active, cannot record.");
        qWarning(logWarning()) << message;
        emit captureError(message);
        return;
    }

    QMediaFormat format(QMediaFormat::MPEG4);
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);
    if (!format.isSupported(QMediaFormat::Encode)) {
        qWarning(logWarning()) << "MPEG4/H264 not supported, using platform default format.";
        format = QMediaFormat();
    }
    m_recorder.setMediaFormat(format);

    m_lastRecordingDurationMs = 0;
    emit recordingDurationChanged();

    m_recorder.setOutputLocation(QUrl::fromLocalFile(nextRecordingPath()));
    m_recorder.record();
    qDebug(logInfo()) << "Recording started:" << m_recorder.outputLocation().toLocalFile();
}

void CameraService::stopRecording()
{
    if (isRecording()) {
        m_recorder.stop();
    }
}

void CameraService::refreshDevices()
{
    const QCameraDevice previousDevice =
        (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_devices.size())
            ? m_devices.at(m_currentCameraIndex) : QCameraDevice();

    m_devices = QMediaDevices::videoInputs();
    emit availableCamerasChanged();

    if (m_devices.isEmpty()) {
        qWarning(logWarning()) << "No camera devices available.";
        m_camera.reset();
        m_currentCameraIndex = -1;
        m_formats.clear();
        m_currentFormatIndex = -1;
        emit currentCameraIndexChanged();
        emit availableFormatsChanged();
        emit currentFormatIndexChanged();
        emit activeChanged();
        return;
    }

    int newIndex = m_devices.indexOf(previousDevice);
    if (newIndex < 0) {
        newIndex = m_devices.indexOf(QMediaDevices::defaultVideoInput());
        if (newIndex < 0) {
            newIndex = 0;
        }
        if (!previousDevice.isNull()) {
            qWarning(logWarning()) << "Camera device disappeared, falling back to:"
                                   << m_devices.at(newIndex).description();
        }
    }
    applyCamera(newIndex);
}

void CameraService::applyCamera(int index)
{
    // A recording cannot survive its source device being replaced.
    stopRecording();

    // Preserve the running state across a device switch; the camera stays
    // off at construction until the camera page activates it.
    const bool wasActive = isActive();

    m_currentCameraIndex = index;
    m_camera = std::make_unique<QCamera>(m_devices.at(index));
    m_captureSession.setCamera(m_camera.get());

    connect(m_camera.get(), &QCamera::activeChanged,
            this, &CameraService::activeChanged);
    connect(m_camera.get(), &QCamera::errorOccurred,
            this, [this](QCamera::Error, const QString& message) {
        qWarning(logWarning()) << "Camera error:" << message;
        emit captureError(message);
    });

    qDebug(logInfo()) << "Camera selected:" << m_devices.at(index).description();
    refreshFormats();

    if (wasActive) {
        m_camera->start();
    }
    emit currentCameraIndexChanged();
    emit activeChanged();
}

void CameraService::refreshFormats()
{
    m_formats = m_devices.at(m_currentCameraIndex).videoFormats();
    m_currentFormatIndex = m_formats.isEmpty() ? -1 : 0;
    if (m_currentFormatIndex >= 0) {
        m_camera->setCameraFormat(m_formats.first());
    }
    emit availableFormatsChanged();
    emit currentFormatIndexChanged();
}

QString CameraService::nextPicturePath() const
{
    const QString timestamp =
        QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    return StorageLocations::picturesDir()
           + "/SolidBroccoli_PIC_" + timestamp + ".jpg";
}

QString CameraService::nextRecordingPath() const
{
    const QString timestamp =
        QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    return StorageLocations::recordingsDir()
           + "/SolidBroccoli_VID_" + timestamp + ".mp4";
}
