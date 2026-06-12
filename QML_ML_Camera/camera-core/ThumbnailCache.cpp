#include "ThumbnailCache.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QImage>

#include "StorageLocations.h"
#include "logger.h"

QString ThumbnailCache::cachePathFor(const QString& sourcePath)
{
    const QByteArray hash =
        QCryptographicHash::hash(sourcePath.toUtf8(), QCryptographicHash::Md5).toHex();
    return StorageLocations::thumbnailsDir() + "/" + QString::fromLatin1(hash) + ".jpg";
}

QString ThumbnailCache::thumbnailFor(const QString& sourcePath)
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        qWarning(logWarning()) << "ThumbnailCache: source missing:" << sourcePath;
        return QString();
    }

    const QString cachePath = cachePathFor(sourcePath);
    const QFileInfo cacheInfo(cachePath);

    // Reuse a cached thumbnail only if it exists and is at least as new as the
    // source, so editing/replacing the original invalidates the thumbnail.
    if (cacheInfo.exists()
            && cacheInfo.lastModified() >= sourceInfo.lastModified()) {
        return cachePath;
    }

    QImage source(sourcePath);
    if (source.isNull()) {
        qWarning(logWarning()) << "ThumbnailCache: could not decode:" << sourcePath;
        return QString();
    }

    const QImage scaled = source.scaled(THUMBNAIL_MAX_EDGE, THUMBNAIL_MAX_EDGE,
                                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!scaled.save(cachePath, "JPEG", 85)) {
        qWarning(logWarning()) << "ThumbnailCache: could not write:" << cachePath;
        return QString();
    }
    return cachePath;
}
