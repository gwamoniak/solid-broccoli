        #include <QDir>
#include <QUrl>
#include <QBuffer>
#include <QAbstractVideoBuffer>
#include <QCameraInfo>
#include "CameraProcessor.h"
#include "logger.h"

CameraProcessor::CameraProcessor(QObject *parent) : QObject(parent),
                                                    m_camera(nullptr),
                                                    m_workerThread(new QThread),
                                                    m_sUrl("")
{

     // for future useage, employ ->" moveToThread(m_workerThread); " into extra processing class
     {
         QDir().mkdir(sRecFolderName);
         QDir().mkdir(sPictureFolderName);
     }
     m_bRecod = false;
}


CameraProcessor::~CameraProcessor()
{
    delete m_workerThread;
}

void CameraProcessor::setCamera(QVariant vCamera)
{

    QObject *object = qvariant_cast<QObject *>(vCamera);
    m_camera = qvariant_cast<QCamera *>(object->property("mediaObject"));
    m_camera->setCaptureMode(QCamera::CaptureStillImage);
    m_imageCapture =  new QCameraImageCapture(m_camera);
    m_camera->focus()->setFocusMode(QCameraFocus::ContinuousFocus);
    m_camera->focus()->setFocusPointMode(QCameraFocus::FocusPointAuto);

//    QCameraInfo cameraInfo(*m_camera);
//    QString camera_name = cameraInfo.deviceName();

//    int cameraIDX = camera_name.mid(camera_name.length()-1,1).toInt(); //extract camera ID from string
//    qDebug(logInfo()) << "ID: " <<cameraIDX;
//    m_videoCapture.open(cameraIDX);
//    if(!m_videoCapture.isOpened())
//        qDebug(logWarning())<<"Cannot open camera";


}

void CameraProcessor::captureImage()
{
    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setPath(QDir::currentPath() + "/"+ sPictureFolderName);

    QString dateTime = QDateTime::currentDateTime().toString("dddd dd MMMM yyyy, hh-mm-ss");
    QString fileNamePicture = "inSOLID_BROCCOLI_PIC_" + dateTime;

    QUrl url(dir.path() + "/" + fileNamePicture);
    m_sUrl = url.path() + ".jpg";
    m_camera->setCaptureMode(QCamera::CaptureStillImage);
    if(m_imageCapture->isReadyForCapture())
    {
       m_imageCapture->capture(url.path());
    }
    else
    {
        qWarning(logWarning())<< "Camera is not ready to capture.";
    }

}

QString CameraProcessor::getURL() const
{
    return m_sUrl;
}


void CameraProcessor::recordMovie()
{
    qDebug(logWarning()) << "Recording";

    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setPath(sRecFolderName);
    QString dateTime = QDateTime::currentDateTime().toString("dddd dd MMMM yyyy, hh-mm-ss ");
    QString fileNameVideo = sRecFolderName + "/Video_" + dateTime + ".avi";

    // the rest is not developed yet
    // it should be part of the controler module - integration with hardware

}

void CameraProcessor::stopRecording()
{
    qDebug(logWarning()) << "stop recording";
    //m_camera->stop();
    m_bRecod = false;
}




