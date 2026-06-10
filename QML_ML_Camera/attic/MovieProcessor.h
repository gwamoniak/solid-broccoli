#ifndef MOVIEPROCESSOR_H
#define MOVIEPROCESSOR_H

#include <QObject>
#include <QUrl>

#include <QQuickImageProvider>
#include <QCache>
#include <QModelIndex>

class MovieModel;

const QString sThumbnailsFolderName = "VideoThumbnails";


class MovieProcessor : public QObject
{
Q_OBJECT
Q_ENUMS (ThumbnailerState)

public:

    explicit MovieProcessor(MovieModel *movieModel = nullptr);

    bool hasThumbnailForUrl    (QUrl fileUrl);
    Q_INVOKABLE bool getLocalFileForUrl (QUrl fileUrl);
    Q_INVOKABLE QUrl getUrlFromLocalPath   (QString path);
    Q_INVOKABLE QUrl getThumbnail();
    Q_INVOKABLE QVariant getUrlFromIndex(const QString &id);
    bool generateFrame(const QString &fileName);

    QVariant m_urlVariant;

private:
    QString m_thumbnailsPath;
    MovieModel   *m_MovieModel;

};

class MovieProvider : public QQuickImageProvider
{
public:

    static const QSize THUMBNAIL_SIZE;

    MovieProvider(MovieModel* movieModel);

    QPixmap requestPixmap(const QString& id, QSize* _size, const QSize& requestSize) override;
    QPixmap *pictureFromCache(const QString& filepath, const QString& pictureSize);


private:
    MovieModel   *m_MovieModel;
    QCache<QString, QPixmap> m_PictureCache;
    QString m_thumbnailsPath;




};

#endif // MOVIEPROCESSOR_H
