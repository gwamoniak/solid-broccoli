#include "PictureModel.h"

#include "Album.h"
#include "DatabaseManager.h"
#include "AlbumModel.h"

#include "logger.h"

using namespace std;

PictureModel::PictureModel(const AlbumModel& _albumModel, QObject* parent) :
    QAbstractListModel(parent),
    m_sqlDB(DatabaseManager::instance()),
    m_nAlbumID(-1),
    m_vPictures(new vector<unique_ptr<Picture>>())
{
    connect(&_albumModel, &AlbumModel::rowsRemoved,
            this, &PictureModel::deletePicturesForAlbum);

    qDebug(logInfo()) << "PictureModel has been created!";
}

QModelIndex PictureModel::addPicture(const Picture& _picture)
{
    int rows = rowCount();
    beginInsertRows(QModelIndex(), rows, rows);
    unique_ptr<Picture>newPicture(new Picture(_picture));
    m_sqlDB.m_pictureDao.addPictureInAlbum(m_nAlbumID, *newPicture);
    m_vPictures->push_back(move(newPicture));
    endInsertRows();
    qDebug(logWarning()) << "index of the picture: " << rows;
    return index(rows, 0);
}

void PictureModel::addPictureFromUrl(const QUrl& _FileUrl)
{
    addPicture(Picture(_FileUrl));
    qDebug(logDebug()) << "Picture name: " << _FileUrl.toString();
}

void PictureModel::rename(int row, const QString &_name)
{
    setData(index(row),_name,DBRoles::NameRole);
    qDebug(logInfo()) << "new name: " << _name;

}

int PictureModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_vPictures->size();
}

QVariant PictureModel::data(const QModelIndex& _index, int role) const
{
    if (!isIndexValid(_index)) {
        return QVariant();
    }

    const Picture& picture = *m_vPictures->at(_index.row());
    switch (role) {
        case Qt::DisplayRole:
            return picture._FileUrl().fileName();
            break;

        case DBRoles::UrlRole:
            return picture._FileUrl();
            break;

        case DBRoles::FilePathRole:
            return picture._FileUrl().toLocalFile();
            break;
        case DBRoles::NameRole:
        default:
            return QVariant();
    }
}

bool PictureModel::removeRows(int _row, int _count, const QModelIndex& parent)
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
        const Picture& picture = *m_vPictures->at(_row + countLeft);
        m_sqlDB.m_pictureDao.removePicture(picture._nPictureID());
    }
    m_vPictures->erase(m_vPictures->begin() + _row,
                    m_vPictures->begin() + _row + _count);
    endRemoveRows();
    return true;
}

QHash<int, QByteArray> PictureModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    //roles[DBRoles::NameRole] = "name";
    roles[DBRoles::FilePathRole] = "filepath";
    roles[DBRoles::UrlRole] = "url";
    return roles;
}

void PictureModel::setAlbumId(int _nAlbumID)
{
    beginResetModel();
    m_nAlbumID = _nAlbumID;
    loadPictures(m_nAlbumID);
    endResetModel();
}

void PictureModel::clearAlbum()
{
    setAlbumId(-1);
}

void PictureModel::deletePicturesForAlbum()
{
    m_sqlDB.m_pictureDao.removePicturesForAlbum(m_nAlbumID);
    clearAlbum();
}

void PictureModel::loadPictures(int _nAlbumID)
{
    if (_nAlbumID <= 0) {
        m_vPictures.reset(new vector<unique_ptr<Picture>>());
        return;
    }
    m_vPictures = m_sqlDB.m_pictureDao.picturesForAlbum(_nAlbumID);
}

bool PictureModel::isIndexValid(const QModelIndex& _index) const
{
    if (_index.row() < 0
            || _index.row() >= rowCount()
            || !_index.isValid()) {
        return false;
    }
    return true;
}
