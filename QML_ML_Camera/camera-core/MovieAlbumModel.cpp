#include "MovieAlbumModel.h"
#include "logger.h"

using namespace std;

MovieAlbumModel::MovieAlbumModel(QObject* parent) :
    QAbstractListModel(parent),
    m_sqlDB(DatabaseManager::instance()),
    m_vAlbums(m_sqlDB.m_malbumDao.albums())
{
        qDebug(logInfo()) << "MovieAlbum has been created!";

}

QModelIndex MovieAlbumModel::addAlbum(const Album& _album)
{
    int rowIndex = rowCount();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    unique_ptr<Album> newAlbum(new Album(_album));
    m_sqlDB.m_malbumDao.addAlbum(*newAlbum);
    m_vAlbums->push_back(move(newAlbum));
    endInsertRows();
    return index(rowIndex, 0);
}

void MovieAlbumModel::addAlbumFromName(const QString& _name)
{
    addAlbum(Album(_name));
    qDebug(logInfo()) << "Album name is: " << _name;
}

void MovieAlbumModel::rename(int row, const QString& _name)
{
    setData(index(row), _name, DBRoles::NameRole);
}

int MovieAlbumModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_vAlbums->size();
}

QVariant MovieAlbumModel::data(const QModelIndex& _index, int role) const
{
    if (!isIndexValid(_index)) {
        return QVariant();
    }
    const Album& album = *m_vAlbums->at(_index.row());

    switch (role) {
        case DBRoles::IdRole:
            return album._nAlbumID();

        case DBRoles::NameRole:
        case Qt::DisplayRole:
            return album._sAlbumName();

        default:
            return QVariant();
    }
}

bool MovieAlbumModel::setData(const QModelIndex& _index, const QVariant& value, int role)
{
    if (!isIndexValid(_index)
            || role != DBRoles::NameRole) {
        return false;
    }
    Album& album = *m_vAlbums->at(_index.row());
    album.set_sAlbumName(value.toString());
    m_sqlDB.m_malbumDao.updateAlbum(album);
    emit dataChanged(_index, _index);
    return true;
}

bool MovieAlbumModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0
            || row >= rowCount()
            || count < 0
            || (row + count) > rowCount()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    int countLeft = count;
    while (countLeft--) {
        const Album& album = *m_vAlbums->at(row + countLeft);
        m_sqlDB.m_malbumDao.removeAlbum(album._nAlbumID());
    }
    m_vAlbums->erase(m_vAlbums->begin() + row,
                  m_vAlbums->begin() + row + count);
    endRemoveRows();
    return true;
}

QHash<int, QByteArray> MovieAlbumModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DBRoles::IdRole] = "id";
    roles[DBRoles::NameRole] = "name";
    return roles;
}

bool MovieAlbumModel::isIndexValid(const QModelIndex& _index) const
{
    return _index.isValid() && _index.row() < rowCount();
}

