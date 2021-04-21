#include "PictureProvider.h"
#include "PictureModel.h"

#include "logger.h"

const QString PICTURE_SIZE_FULL = "full";
const QString PICTURE_SIZE_THUMBNAIL = "thumbnail";
const QSize PictureProvider::THUMBNAIL_SIZE = QSize(240,240);


PictureProvider::PictureProvider(PictureModel *pictureModel):
         QQuickImageProvider(QQuickImageProvider::Pixmap),
         m_PictureModel(pictureModel),                                                             
         m_PictureCache()
{

}




QPixmap PictureProvider::requestPixmap(const QString &id, QSize* /*_size*/, const QSize& /*requestSize*/)
{
    QStringList query = id.split('/');
    if(!m_PictureModel || query.size() < 2)
    {
        return QPixmap();
    }

    int row = query[0].toInt();
    QString pictureSize = query[1];
    QUrl fileUrl;

    if(m_PictureModel)
        fileUrl = m_PictureModel->data(m_PictureModel->index(row,0),
                                        PictureModel::DBRoles::UrlRole).toUrl();


    return *pictureFromCache(fileUrl.toLocalFile(), pictureSize);

}

QPixmap *PictureProvider::pictureFromCache(const QString &filepath, const QString &pictureSize)
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
