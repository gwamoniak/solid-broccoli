#ifndef STORAGELOCATIONS_H
#define STORAGELOCATIONS_H

#include <QString>

#include "camera-core_global.h"

// Single owner of every on-disk location the application writes to.
// All paths live under one application root (QStandardPaths::AppDataLocation
// by default) so the gallery is independent of the process working directory.
class CAMERACORESHARED_EXPORT StorageLocations
{
public:
    static QString root();
    static QString picturesDir();
    static QString recordingsDir();
    static QString thumbnailsDir();
    static QString logsDir();
    static QString exportsDir();
    static QString databasePath();

    // Redirects root() to a temporary directory; tests only.
    static void setRootForTesting(const QString& path);

private:
    static QString ensureDir(const QString& path);
    static QString s_rootOverride;
};

#endif // STORAGELOCATIONS_H
