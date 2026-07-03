#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <memory>

#include <QString>

#include "camera-core_global.h"

#include "AlbumDAO.h"
#include "PictureDAO.h"
#include "LoggerDAO.h"
#include "MovieDAO.h"
#include "SessionDAO.h"
#include "SpectrumDAO.h"

class QSqlQuery;
class QSqlDatabase;

// Owns one SQLite connection and the DAOs operating on it.
// Construct explicitly and inject by reference into the models;
// pass ":memory:" as path for tests. Each instance uses a unique
// connection name so several databases can coexist in one process.
class CAMERACORESHARED_EXPORT DatabaseManager
{
public:
    explicit DatabaseManager(const QString& path = defaultDatabasePath());
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static QString defaultDatabasePath();
    static void debugQuery(const QSqlQuery& _query);

    bool isOpen() const;
    QSqlDatabase& database();

private:
    // Order matters: the connection name must outlive-initialize the database.
    QString m_connectionName;
    std::unique_ptr<QSqlDatabase> m_sqlDataBase;

public:
    const AlbumDAO m_albumDao;
    const PictureDAO m_pictureDao;
    const LoggerDAO m_loggerDao;
    const MovieDAO m_movieDao;
    const SessionDAO m_sessionDao;
    const SpectrumDAO m_spectrumDao;
};

#endif // DATABASEMANAGER_H
