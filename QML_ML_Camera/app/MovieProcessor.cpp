#include "MovieProcessor.h"
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QStringBuilder>
#include "MovieModel.h"
#include "logger.h"

#include "opencv2/core.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"


using namespace cv;

const QString PICTURE_SIZE_FULL = "full";
const QString PICTURE_SIZE_THUMBNAIL = "thumbnail";
const QSize MovieProvider::THUMBNAIL_SIZE = QSize(240,240);
QString m_ThumbnailFilePath = "";

bool MovieProcessor::generateFrame(const QString &fileName)
{

    VideoCapture cap(fileName.toStdString());

    if(cap.isOpened())  // check if we succeeded
    {
        Mat frame;
        // size 640*480 so the generated thumbnail would not be distored when shown in gridview
        Mat img(Size(640,480),CV_8UC3);


        //collect first 3 frames and save 3rd one, to be sure it is not black/blank

        for (int var = 0; var < 3; ++var)
        {
            cap >> frame;
        }

        std::vector<int> compression_params;
        compression_params.push_back(IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(5);
        cv::resize(frame,img,Size(640,480),0.5,0.5,INTER_LANCZOS4);
        cv::imwrite( m_ThumbnailFilePath.toStdString(), img);

        qDebug(logInfo()) << "Thumbnail created";
        cap.release();
        return true;

    }
    else {
        qDebug(logWarning()) << "The movie file has not been opened!";
         cap.release();
        return false;
    }


}

MovieProcessor::MovieProcessor(MovieModel *movieModel) : m_MovieModel(movieModel)
{
    m_thumbnailsPath = (QDir::currentPath() + "/"+ sThumbnailsFolderName);

    QDir dir;
    dir.mkpath (m_thumbnailsPath);
}

bool MovieProcessor::hasThumbnailForUrl(QUrl fileUrl)
{


// create a content for checking if the thumbnail already exists
        return true;


}

bool MovieProcessor::getLocalFileForUrl(QUrl videoUrl)
{

    QString url = videoUrl.toString ();
    QString md5 = QString::fromLocal8Bit (QCryptographicHash::hash (url.toLocal8Bit (), QCryptographicHash::Md5).toHex ());
    m_thumbnailsPath = (QDir::currentPath() + "/"+ sThumbnailsFolderName);
    m_ThumbnailFilePath = (m_thumbnailsPath % "/" % md5 % ".png");

    if(!generateFrame(videoUrl.toString()))
    {
        qDebug(logWarning()) << "The thumbnail cannot be generated";
        return false;
    }

    return true;

}

QUrl MovieProcessor::getUrlFromLocalPath(QString path)
{
    return QUrl::fromLocalFile (path);
}

QUrl MovieProcessor::getThumbnail()
{
    if(m_ThumbnailFilePath != "")
        return QUrl::fromLocalFile (m_ThumbnailFilePath);
}


QVariant MovieProcessor::getUrlFromIndex(const QString &id)
{
    if(!m_MovieModel )
    {
        qDebug(logWarning()) << "MovieModel error, in movie porcessor";
    }

    int row = id.toInt();
    QUrl fileUrl;

    qDebug(logWarning()) << "before data";

    fileUrl = m_MovieModel->data(m_MovieModel->index(row,0),
                                 MovieModel::DBRoles::UrlRole).toUrl();

    m_urlVariant = fileUrl;

    qDebug(logWarning()) << "variant: " << m_urlVariant;

    return m_urlVariant;

}



MovieProvider::MovieProvider(MovieModel *movieModel):
   QQuickImageProvider(QQuickImageProvider::Pixmap),
   m_MovieModel(movieModel),
   m_PictureCache()
{

}

QPixmap MovieProvider::requestPixmap(const QString &id, QSize *_size, const QSize &requestSize)
{
    QStringList query = id.split('/');

    if(!m_MovieModel || query.size() < 2)
    {
        qDebug(logWarning()) << "query size error";
        return QPixmap();
    }

    qDebug(logInfo()) <<  "request from movieprovider: " << id;
    int row = query[0].toInt();
    QString pictureSize = query[1];
    QUrl fileUrl;

    if(m_MovieModel)
        fileUrl = m_MovieModel->data(m_MovieModel->index(row,0),
                                     MovieModel::DBRoles::ThumbnailUrlRole).toUrl();

       return *pictureFromCache(fileUrl.toLocalFile(), pictureSize);

}

QPixmap *MovieProvider::pictureFromCache(const QString &filepath, const QString &pictureSize)
{
    QString key = pictureSize + "-" + filepath;

    QPixmap *cachePicture = nullptr;
    if(!m_PictureCache.contains(key))
    {
       QPixmap originalPicture(filepath);
       if (pictureSize == PICTURE_SIZE_THUMBNAIL)
       {
           cachePicture = new QPixmap(originalPicture.scaled(THUMBNAIL_SIZE,Qt::KeepAspectRatio,Qt::SmoothTransformation));
       }
       else if(pictureSize == PICTURE_SIZE_FULL)
       {
           cachePicture = new QPixmap(originalPicture);

       }
       m_PictureCache.insert(key, cachePicture);

    }
    else
    {
        cachePicture = m_PictureCache[key];

    }

    return cachePicture;
}

