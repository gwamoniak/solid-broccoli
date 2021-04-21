#ifndef PICTUREIMAGEPROVIDER_H
#define PICTUREIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QCache>

class PictureModel;

class PictureProvider : public QQuickImageProvider
{
public:

    static const QSize THUMBNAIL_SIZE;

    PictureProvider(PictureModel* pictureModel);

    QPixmap requestPixmap(const QString& id, QSize* _size, const QSize& requestSize) override;
    QPixmap *pictureFromCache(const QString& filepath, const QString& pictureSize);

private:
    PictureModel *m_PictureModel;
    QCache<QString, QPixmap> m_PictureCache;



};

#endif // PICTUREIMAGEPROVIDER_H
