#ifndef PICTUREDAO_H
#define PICTUREDAO_H

#include <memory>
#include <vector>

class QSqlDatabase;
class Picture;


// CRUD (Create/Read/Update/Delete) class
// https://en.wikipedia.org/wiki/Create,_read,_update_and_delete
// data access object - DAO
class PictureDAO
{
public:
    explicit PictureDAO(QSqlDatabase& _database);
    void init() const;

    void addPictureInAlbum(int _nAlbumId, Picture& _picture) const;
    void removePicture(int _nPictureID) const;
    void removePicturesForAlbum(int _nAlbumID) const;
    std::unique_ptr<std::vector<std::unique_ptr<Picture>>> picturesForAlbum(int _nAlbumID) const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // PICTUREDAO_H
