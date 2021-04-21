#include "PictureDAO.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "Picture.h"
#include "DatabaseManager.h"

using namespace std;

PictureDAO::PictureDAO(QSqlDatabase& _database) :
    m_sqlDB(_database)
{
}

void PictureDAO::init() const
{
    if (!m_sqlDB.tables().contains("pictures")) {
        QSqlQuery query(m_sqlDB);
        query.exec(QString("CREATE TABLE pictures")
        + " (id INTEGER PRIMARY KEY AUTOINCREMENT, "
        + "album_id INTEGER, "
        + "url TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

void PictureDAO::addPictureInAlbum(int _nAlbumID, Picture& _picture) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare(QString("INSERT INTO pictures")
        + " (album_id, url)"
        + " VALUES ("
        + ":album_id, "
        + ":url"
        + ")");
    query.bindValue(":album_id", _nAlbumID);
    query.bindValue(":url", _picture._FileUrl());
    query.exec();
    DatabaseManager::debugQuery(query);
    _picture.set_nPictureID(query.lastInsertId().toInt());
    _picture.set_nAlbumID(_nAlbumID);
}

void PictureDAO::removePicture(int _nPictureID) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM pictures WHERE id = (:id)");
    query.bindValue(":id", _nPictureID);
    query.exec();
    DatabaseManager::debugQuery(query);
}

void PictureDAO::removePicturesForAlbum(int _nAlbumID) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM pictures WHERE album_id = (:album_id)");
    query.bindValue(":album_id", _nAlbumID);
    query.exec();
    DatabaseManager::debugQuery(query);
}

unique_ptr<vector<unique_ptr<Picture>>> PictureDAO::picturesForAlbum(int _nAlbumID) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM pictures WHERE album_id = (:album_id)");
    query.bindValue(":album_id", _nAlbumID);
    query.exec();
    DatabaseManager::debugQuery(query);
    unique_ptr<vector<unique_ptr<Picture>>> list(new vector<unique_ptr<Picture>>());
    while(query.next()) {
        unique_ptr<Picture> picture(new Picture());
        picture->set_nPictureID(query.value("id").toInt());
        picture->set_nAlbumID(query.value("album_id").toInt());
        picture->set_FileUrl(query.value("url").toUrl());
        list->push_back(move(picture));
    }
    return list;
}
