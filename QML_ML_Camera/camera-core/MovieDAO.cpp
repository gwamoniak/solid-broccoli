#include "MovieDAO.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "Movie.h"
#include "DatabaseManager.h"

using namespace std;

MovieDAO::MovieDAO(QSqlDatabase& _database) :
    m_sqlDB(_database)
{
}

void MovieDAO::init() const
{
    QSqlQuery query(m_sqlDB);
    query.exec(QString("CREATE TABLE IF NOT EXISTS movies")
    + " (id INTEGER PRIMARY KEY AUTOINCREMENT, "
    + "album_id INTEGER, "
    + "url TEXT, "
    + "name TEXT, "
    + "duration INTEGER, "
    + "created_at TEXT)");
    DatabaseManager::debugQuery(query);
}

void MovieDAO::addMovie(Movie& _movie) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare(QString("INSERT INTO movies")
        + " (album_id, url, name, duration, created_at)"
        + " VALUES (:album_id, :url, :name, :duration, :created_at)");
    query.bindValue(":album_id", _movie._nAlbumID());
    query.bindValue(":url", _movie._FileUrl());
    query.bindValue(":name", _movie._sName());
    query.bindValue(":duration", _movie._nDurationMs());
    query.bindValue(":created_at", _movie._sCreatedAt());
    query.exec();
    DatabaseManager::debugQuery(query);
    _movie.set_nMovieID(query.lastInsertId().toInt());
}

void MovieDAO::updateMovie(const Movie& _movie) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("UPDATE movies SET name = (:name) WHERE id = (:id)");
    query.bindValue(":name", _movie._sName());
    query.bindValue(":id", _movie._nMovieID());
    query.exec();
    DatabaseManager::debugQuery(query);
}

void MovieDAO::removeMovie(int _nMovieID) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM movies WHERE id = (:id)");
    query.bindValue(":id", _nMovieID);
    query.exec();
    DatabaseManager::debugQuery(query);
}

unique_ptr<vector<unique_ptr<Movie>>> MovieDAO::movies() const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM movies ORDER BY id DESC");
    query.exec();
    DatabaseManager::debugQuery(query);
    unique_ptr<vector<unique_ptr<Movie>>> list(new vector<unique_ptr<Movie>>());
    while (query.next()) {
        unique_ptr<Movie> movie(new Movie());
        movie->set_nMovieID(query.value("id").toInt());
        movie->set_nAlbumID(query.value("album_id").toInt());
        movie->set_FileUrl(query.value("url").toUrl());
        movie->set_sName(query.value("name").toString());
        movie->set_nDurationMs(query.value("duration").toLongLong());
        movie->set_sCreatedAt(query.value("created_at").toString());
        list->push_back(std::move(movie));
    }
    return list;
}
