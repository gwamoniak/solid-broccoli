#ifndef MOVIEDAO_H
#define MOVIEDAO_H

#include <memory>
#include <vector>

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

    void addMovie(Movie& _movie) const;
    void updateMovie(const Movie& _movie) const;
    void removeMovie(int _nMovieID) const;
    std::unique_ptr<std::vector<std::unique_ptr<Movie>>> movies() const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // MOVIEDAO_H
