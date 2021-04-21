#include "MovieDAO.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "logger.h"
#include "Movie.h"
#include "DatabaseManager.h"


using namespace std;

MovieDAO::MovieDAO(QSqlDatabase& _sqldatabase):
    m_sqlDBM(_sqldatabase)
{

}


void MovieDAO::init() const
{
    if(!m_sqlDBM.tables().contains("movies"))
    {
       QSqlQuery  _query(m_sqlDBM);
       _query.exec(QString("CREATE TABLE movies")
        + " (id INTEGER PRIMARY KEY AUTOINCREMENT, "
        + "album_id INTEGER, "
        + "url TEXT,"
        + "thumbnail TEXT)");
       DatabaseManager::debugQuery(_query);
    }

}

void MovieDAO::addMovieInAlbum(int _nAlbumID, Movie& _movie) const
{
    QSqlQuery query(m_sqlDBM);
    query.prepare(QString("INSERT INTO movies")
                  + " (album_id, url, thumbnail)"
                  + " VALUES ("
                  + ":album_id, "
                  + ":url,"
                  + ":thumbnail"
                  + ")");
    query.bindValue(":album_id", _nAlbumID);
    query.bindValue(":url", _movie._FileUrl());
    query.bindValue(":thumbnail",_movie._ThumbnailFileUrl());
    query.exec();
    DatabaseManager::debugQuery(query);
    _movie.set_nMovieID(query.lastInsertId().toInt());
    _movie.set_nAlbumID(_nAlbumID);
}

void MovieDAO::removeMovie(int _nMovieID) const
{
    QSqlQuery query(m_sqlDBM);
    query.prepare("DELETE FROM movies WHERE id = (:id)");
    query.bindValue(":id", _nMovieID);
    query.exec();
    DatabaseManager::debugQuery(query);
}

void MovieDAO::removeMovieFromAlbum(int _nAlbumID) const
{
    QSqlQuery query(m_sqlDBM);
    query.prepare("DELETE FROM movies WHERE album_id = (:album_id)");
    query.bindValue(":album_id", _nAlbumID);
    query.exec();
    DatabaseManager::debugQuery(query);
}

std::unique_ptr<std::vector<std::unique_ptr<Movie>>> MovieDAO::moviesForAlbum(int _nAlbumID) const
{
    QSqlQuery query(m_sqlDBM);
    query.prepare("SELECT * FROM movies WHERE album_id = (:album_id)");
    query.bindValue(":album_id", _nAlbumID);
    query.exec();
    DatabaseManager::debugQuery(query);
    std::unique_ptr<std::vector<std::unique_ptr<Movie>>> list(new std::vector<std::unique_ptr<Movie>>());
    while(query.next()) {
        std::unique_ptr<Movie> movie(new Movie());
        movie->set_nMovieID(query.value("id").toInt());
        movie->set_nAlbumID(query.value("album_id").toInt());
        movie->set_movieFileUrl(query.value("url").toUrl());
        movie->set_ThumbnailmovieFileUrl(query.value("thumbnail").toUrl());
        list->push_back(move(movie));
    }
    return list;
}
