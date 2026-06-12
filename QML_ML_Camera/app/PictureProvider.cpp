#include "PictureProvider.h"
#include "PictureModel.h"
#include "ThumbnailCache.h"

#include <QImage>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QRunnable>

#include "logger.h"

namespace {
const QString PICTURE_SIZE_THUMBNAIL = QStringLiteral("thumbnail");
}

const QSize PictureProvider::THUMBNAIL_SIZE = QSize(240, 240);

// One in-flight request. The heavy decode/scale happens in run() on the pool;
// when finished it emits finished() and the engine pulls the image via
// textureFactory(). The file path is resolved before construction (GUI thread)
// so the worker never touches the model.
class PictureResponse : public QQuickImageResponse, public QRunnable
{
public:
    PictureResponse(const QString& filePath, bool thumbnail)
        : m_filePath(filePath), m_thumbnail(thumbnail)
    {
        setAutoDelete(false);
    }

    QQuickTextureFactory* textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    void run() override
    {
        if (m_filePath.isEmpty()) {
            emit finished();
            return;
        }
        if (m_thumbnail) {
            const QString thumbPath = ThumbnailCache::thumbnailFor(m_filePath);
            m_image.load(thumbPath.isEmpty() ? m_filePath : thumbPath);
        } else {
            m_image.load(m_filePath);
        }
        emit finished();
    }

private:
    QImage m_image;
    QString m_filePath;
    bool m_thumbnail;
};

PictureProvider::PictureProvider(PictureModel* pictureModel)
    : m_pictureModel(pictureModel)
{
}

QQuickImageResponse* PictureProvider::requestImageResponse(const QString& id,
                                                           const QSize& /*requestSize*/)
{
    const QStringList query = id.split('/');
    QString filePath;
    bool thumbnail = true;

    if (m_pictureModel && query.size() >= 2) {
        const int row = query.at(0).toInt();
        thumbnail = (query.at(1) == PICTURE_SIZE_THUMBNAIL);
        filePath = m_pictureModel->data(m_pictureModel->index(row, 0),
                                        PictureModel::DBRoles::FilePathRole).toString();
    }

    auto* response = new PictureResponse(filePath, thumbnail);
    m_pool.start(response);
    return response;
}
