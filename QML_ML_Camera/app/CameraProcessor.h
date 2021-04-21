#ifndef CAMERAPROCESSOR_H
#define CAMERAPROCESSOR_H

#include <QObject>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QThread>
#include <QThreadPool>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QCamera>




const QString sRecFolderName = "recodings";
const QString sPictureFolderName = "pictures";


class CameraProcessor: public QObject
{
        Q_OBJECT

public:
    explicit CameraProcessor(QObject * parent = nullptr);
    //CameraProcessor(PictureModel pictureModel);
    ~CameraProcessor();

    Q_INVOKABLE void setCamera(QVariant vCamera);
    Q_INVOKABLE void captureImage();
    Q_INVOKABLE QString getURL() const;
    Q_INVOKABLE void recordMovie();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void processEvents()
    {
       qApp->processEvents();
    }


signals:
    void processStarted();
    void imageCaptured(int, const QImage&);

private:

    QCameraImageCapture* m_imageCapture;
    QString m_sUrl;
    QCamera *m_camera;
    QThread *m_workerThread;
    QImage m_image;
    bool m_bRecod;


    //void processSavedImage(const QString &iname);

};

#endif // CAMERAPROCESSOR_H
