#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <memory>

#include <QString>


#include "AlbumDAO.h"
#include "PictureDAO.h"
#include "MovieDAO.h"
#include "LoggerDAO.h"
#include "MovieAlbumDAO.h"

class QSqlQuery;
class QSqlDatabase;

const QString DATABASE_FILENAME = "solidBroccoli_Gallery.db";

class DatabaseManager
{
public:
    static void debugQuery(const QSqlQuery& _query);

    static DatabaseManager& instance();
    ~DatabaseManager();

protected:
    DatabaseManager(const QString& _path = DATABASE_FILENAME);
    DatabaseManager& operator=(const DatabaseManager& rhs);

private:
    std::unique_ptr<QSqlDatabase> m_sqlDataBase;

public:
    const AlbumDAO m_albumDao;
    const MovieAlbumDAO m_malbumDao;
    const PictureDAO m_pictureDao;
    const MovieDAO m_movieDao;
    const LoggerDAO m_loggerDao;
};

#endif // DATABASEMANAGER_H
