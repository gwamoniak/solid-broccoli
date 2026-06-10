#include "MovieAlbumDAO.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>


#include "DatabaseManager.h"
#include "Album.h"

using namespace std;

MovieAlbumDAO::MovieAlbumDAO(QSqlDatabase& _database) :
    m_sqlDB(_database)
{
}

void MovieAlbumDAO::init() const
{
    if (!m_sqlDB.tables().contains("moviealbums")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE moviealbums (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

void MovieAlbumDAO::addAlbum(Album& _album) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO moviealbums (name) VALUES (:name)");
    query.bindValue(":name", _album._sAlbumName());
    query.exec();
    _album.set_nAlbumID(query.lastInsertId().toInt());
    DatabaseManager::debugQuery(query);
}

void MovieAlbumDAO::updateAlbum(const Album& _album)const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("UPDATE moviealbums SET name = (:name) WHERE id = (:id)");
    query.bindValue(":name", _album._sAlbumName());
    query.bindValue(":id", _album._nAlbumID());
    query.exec();
    DatabaseManager::debugQuery(query);
}

void MovieAlbumDAO::removeAlbum(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM moviealbums WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}

unique_ptr<vector<unique_ptr<Album>>> MovieAlbumDAO::albums() const
{
    QSqlQuery query("SELECT * FROM moviealbums", m_sqlDB);
    query.exec();
    unique_ptr<vector<unique_ptr<Album>>> list(new vector<unique_ptr<Album>>());
    while(query.next()) {
        unique_ptr<Album> album(new Album());
        album->set_nAlbumID(query.value("id").toInt());
        album->set_sAlbumName(query.value("name").toString());
        list->push_back(move(album));
    }
    return list;
}
