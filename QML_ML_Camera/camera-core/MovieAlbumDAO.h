#ifndef MOVIEALBUMDAO_H
#define MOVIEALBUMDAO_H

#include <memory>
#include <vector>

class QSqlDatabase;
class Album;

// CRUD (Create/Read/Update/Delete) class
// https://en.wikipedia.org/wiki/Create,_read,_update_and_delete
// data access object - DAO
class MovieAlbumDAO
{
public:
    MovieAlbumDAO(QSqlDatabase& database);
    void init() const;

    void addAlbum(Album& _album) const;
    void updateAlbum(const Album& _album) const;
    void removeAlbum(int _id) const;
    std::unique_ptr<std::vector<std::unique_ptr<Album>>> albums() const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // MOVIEALBUMDAO_H
