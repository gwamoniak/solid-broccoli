#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QString>

#include "camera-core_global.h"

// Generates and serves on-disk thumbnails so the UI never scales a
// full-resolution image on the GUI thread. The first request for a source
// image produces a downscaled JPEG under StorageLocations::thumbnailsDir();
// later requests reuse that file. Safe to call from a worker thread: it only
// touches the filesystem and QImage, no Qt GUI objects.
class CAMERACORESHARED_EXPORT ThumbnailCache
{
public:
    // Longest edge (in pixels) of a generated thumbnail. 400 keeps small
    // grid cells crisp on HiDPI displays while staying cheap to decode.
    static const int THUMBNAIL_MAX_EDGE = 400;

    // Returns the absolute path to a cached thumbnail for sourcePath,
    // generating it on the calling thread if it is missing or older than the
    // source. Returns an empty string if the source cannot be read.
    static QString thumbnailFor(const QString& sourcePath);

private:
    static QString cachePathFor(const QString& sourcePath);
};

#endif // THUMBNAILCACHE_H
