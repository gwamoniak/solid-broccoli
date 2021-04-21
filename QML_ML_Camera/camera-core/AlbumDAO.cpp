#include "AlbumDAO.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "Album.h"
#include "DatabaseManager.h"

using namespace std;

AlbumDAO::AlbumDAO(QSqlDatabase& _database) :
    m_sqlDB(_database)
{
}

void AlbumDAO::init() const
{
    if (!m_sqlDB.tables().contains("albums")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE albums (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

void AlbumDAO::addAlbum(Album& _album) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO albums (name) VALUES (:name)");
    query.bindValue(":name", _album._sAlbumName());
    query.exec();
    _album.set_nAlbumID(query.lastInsertId().toInt());
    DatabaseManager::debugQuery(query);
}

void AlbumDAO::updateAlbum(const Album& _album)const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("UPDATE albums SET name = (:name) WHERE id = (:id)");
    query.bindValue(":name", _album._sAlbumName());
    query.bindValue(":id", _album._nAlbumID());
    query.exec();
    DatabaseManager::debugQuery(query);
}

void AlbumDAO::removeAlbum(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM albums WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}

unique_ptr<vector<unique_ptr<Album>>> AlbumDAO::albums() const
{
    QSqlQuery query("SELECT * FROM albums", m_sqlDB);
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
