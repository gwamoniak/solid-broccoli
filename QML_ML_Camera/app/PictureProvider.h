#ifndef PICTUREIMAGEPROVIDER_H
#define PICTUREIMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class PictureModel;

// Serves "image://pictures/<row>/<size>" requests asynchronously so opening a
// large album never stalls the GUI thread. The row is resolved to a file path
// on the calling (GUI) thread; the actual decode/scale runs on a thread pool.
// Thumbnails are produced once by ThumbnailCache and reused from disk.
class PictureProvider : public QQuickAsyncImageProvider
{
public:
    // Display size of a grid thumbnail cell; the cached image itself is larger
    // (see ThumbnailCache) so it stays crisp on HiDPI displays.
    static const QSize THUMBNAIL_SIZE;

    explicit PictureProvider(PictureModel* pictureModel);

    QQuickImageResponse* requestImageResponse(const QString& id,
                                              const QSize& requestSize) override;

private:
    PictureModel* m_pictureModel;
    QThreadPool m_pool;
};

#endif // PICTUREIMAGEPROVIDER_H
