#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <memory>

#include <QString>

#include "camera-core_global.h"

#include "AlbumDAO.h"
#include "PictureDAO.h"
#include "LoggerDAO.h"
#include "MovieDAO.h"
#include "MeasurementDAO.h"
#include "ReportDAO.h"
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

    const AlbumDAO& albumDao() const { return m_albumDao; }
    const PictureDAO& pictureDao() const { return m_pictureDao; }
    const LoggerDAO& loggerDao() const { return m_loggerDao; }
    const MovieDAO& movieDao() const { return m_movieDao; }
    const SessionDAO& sessionDao() const { return m_sessionDao; }
    const SpectrumDAO& spectrumDao() const { return m_spectrumDao; }
    const MeasurementDAO& measurementDao() const { return m_measurementDao; }
    const ReportDAO& reportDao() const { return m_reportDao; }

private:
    // Order matters: the connection name must outlive-initialize the database.
    QString m_connectionName;
    std::unique_ptr<QSqlDatabase> m_sqlDataBase;

    const AlbumDAO m_albumDao;
    const PictureDAO m_pictureDao;
    const LoggerDAO m_loggerDao;
    const MovieDAO m_movieDao;
    const SessionDAO m_sessionDao;
    const SpectrumDAO m_spectrumDao;
    const MeasurementDAO m_measurementDao;
    const ReportDAO m_reportDao;
};

#endif // DATABASEMANAGER_H
