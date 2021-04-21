#ifndef MOVIEDAO_H
#define MOVIEDAO_H

#include <vector>
#include <memory>

class QSqlDatabase;
class Movie;


// CRUD (Create/Read/Update/Delete) class
// https://en.wikipedia.org/wiki/Create,_read,_update_and_delete
// data access object - DAO
class MovieDAO
{
public:
    explicit MovieDAO(QSqlDatabase& _database);
    void init() const;

    void addMovieInAlbum(int _nAlbumID, Movie& _movie) const;
    void removeMovie(int _nMovieID) const;
    void removeMovieFromAlbum(int _nAlbumID) const;

    std::unique_ptr<std::vector<std::unique_ptr<Movie>>> moviesForAlbum(int _nAlbumID) const;

private:
      QSqlDatabase& m_sqlDBM;

};

#endif // MOVIEDAO_H
