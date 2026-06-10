#include "StorageLocations.h"

#include <QDir>
#include <QStandardPaths>

QString StorageLocations::s_rootOverride;

QString StorageLocations::root()
{
    if (!s_rootOverride.isEmpty()) {
        return ensureDir(s_rootOverride);
    }
    return ensureDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

QString StorageLocations::picturesDir()
{
    return ensureDir(root() + "/pictures");
}

QString StorageLocations::recordingsDir()
{
    return ensureDir(root() + "/recordings");
}

QString StorageLocations::thumbnailsDir()
{
    return ensureDir(root() + "/thumbnails");
}

QString StorageLocations::logsDir()
{
    return ensureDir(root() + "/logs");
}

QString StorageLocations::databasePath()
{
    return root() + "/solidBroccoli_Gallery.db";
}

void StorageLocations::setRootForTesting(const QString& path)
{
    s_rootOverride = path;
}

QString StorageLocations::ensureDir(const QString& path)
{
    QDir().mkpath(path);
    return path;
}
