#include "MovieModel.h"

#include "Album.h"
#include "DatabaseManager.h"
#include "MovieAlbumModel.h"

#include "logger.h"

using namespace std;

MovieModel::MovieModel(const MovieAlbumModel& _albumModel, QObject* parent) :
  QAbstractListModel (parent),
  m_sqlDB(DatabaseManager::instance()),
  m_nAlbumID(-1),
  m_vMovies(new std::vector<std::unique_ptr<Movie>>())
{
  connect(&_albumModel, &MovieAlbumModel::rowsRemoved,
            this, &MovieModel::deleteMoviesForAlbum);

  qDebug(logInfo()) << "MovieModel has been created!";
}

QModelIndex MovieModel::addMovie(const Movie& _Movie)
{
    int rows = rowCount();
    beginInsertRows(QModelIndex(), rows, rows);
    unique_ptr<Movie>newMovie(new Movie(_Movie));
    m_sqlDB.m_movieDao.addMovieInAlbum(m_nAlbumID,*newMovie);
    m_vMovies->push_back(move(newMovie));
    endInsertRows();
    return index(rows, 0);
}

void MovieModel::addMovieFromUrl(const QUrl& _FileUrl, const QUrl& _ThumnailFilePath)
{
    addMovie(Movie(_FileUrl, _ThumnailFilePath));
}

void MovieModel::rename(int row, const QString &_name)
{
    setData(index(row),_name,DBRoles::NameRole);
    qDebug(logInfo()) << "new name: " << _name;
}

int MovieModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_vMovies->size();
}

QVariant MovieModel::data(const QModelIndex& _index, int role) const
{
    if (!isIndexValid(_index)) {
        return QVariant();
    }

    const Movie& Movie = *m_vMovies->at(_index.row());
    switch (role) {
    case Qt::DisplayRole:
        return Movie._FileUrl().fileName();
        break;
    case DBRoles::UrlRole:
        return Movie._FileUrl();
        break;
    case DBRoles::FilePathRole:
        return Movie._FileUrl().toLocalFile();
        break;
    case DBRoles::ThumbnailUrlRole:
        return Movie._ThumbnailFileUrl();
        break;
    default:
        return QVariant();
    }
}

bool MovieModel::removeRows(int _row, int _count, const QModelIndex& parent)
{
    if (_row < 0
        || _row >= rowCount()
        || _count < 0
        || (_row + _count) > rowCount()) {
        return false;
    }

    beginRemoveRows(parent, _row, _row + _count - 1);
    int countLeft = _count;
    while(countLeft--) {
        const Movie& Movie = *m_vMovies->at(_row + countLeft);
        m_sqlDB.m_movieDao.removeMovie(Movie._nMovieID());
    }
    m_vMovies->erase(m_vMovies->begin() + _row,
                       m_vMovies->begin() + _row + _count);
    endRemoveRows();


    return true;
}

QHash<int, QByteArray> MovieModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[DBRoles::FilePathRole] = "filepath";
    roles[DBRoles::UrlRole] = "url";
    roles[DBRoles::ThumbnailUrlRole] = "thumbnail";
    return roles;
}

void MovieModel::setAlbumId(int _nAlbumID)
{
    beginResetModel();
    m_nAlbumID = _nAlbumID;
    loadMovies(m_nAlbumID);
    endResetModel();
}

void MovieModel::clearAlbum()
{
    setAlbumId(-1);
}

void MovieModel::deleteMoviesForAlbum()
{
    m_sqlDB.m_movieDao.removeMovieFromAlbum(m_nAlbumID);
    clearAlbum();
}

void MovieModel::loadMovies(int _nAlbumID)
{
    if (_nAlbumID <= 0) {
        m_vMovies.reset(new vector<unique_ptr<Movie>>());
        return;
    }
    m_vMovies = m_sqlDB.m_movieDao.moviesForAlbum(m_nAlbumID);
}

bool MovieModel::isIndexValid(const QModelIndex& _index) const
{
    if (_index.row() < 0
        || _index.row() >= rowCount()
        || !_index.isValid()) {
        return false;
    }
    return true;
}
