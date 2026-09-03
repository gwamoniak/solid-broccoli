#include "AlbumModel.h"
#include "logger.h"

using namespace std;

AlbumModel::AlbumModel(DatabaseManager& db, QObject* parent) :
    QAbstractListModel(parent),
    m_sqlDB(db),
    m_vAlbums(m_sqlDB.albumDao().albums())
{
        qDebug(logInfo()) << "Album has been created!";

}

QModelIndex AlbumModel::addAlbum(const Album& _album)
{
    int rowIndex = rowCount();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    unique_ptr<Album> newAlbum(new Album(_album));
    m_sqlDB.albumDao().addAlbum(*newAlbum);
    m_vAlbums->push_back(std::move(newAlbum));
    endInsertRows();
    return index(rowIndex, 0);
}

void AlbumModel::addAlbumFromName(const QString& _name)
{
    addAlbum(Album(_name));
    qDebug(logInfo()) << "Album name is: " << _name;
}

int AlbumModel::createAlbumReturningId(const QString& _name)
{
    const QModelIndex idx = addAlbum(Album(_name));
    return data(idx, DBRoles::IdRole).toInt();
}

int AlbumModel::albumCount() const
{
    return static_cast<int>(m_vAlbums->size());
}

int AlbumModel::albumIdAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_vAlbums->size())) {
        return -1;
    }
    return m_vAlbums->at(row)->_nAlbumID();
}

QString AlbumModel::albumNameForId(int _nAlbumID) const
{
    for (const auto& album : *m_vAlbums) {
        if (album->_nAlbumID() == _nAlbumID) {
            return album->_sAlbumName();
        }
    }
    return QString();
}

void AlbumModel::rename(int row, const QString& _name)
{
    setData(index(row), _name, DBRoles::NameRole);
}

int AlbumModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_vAlbums->size();
}

QVariant AlbumModel::data(const QModelIndex& _index, int role) const
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

bool AlbumModel::setData(const QModelIndex& _index, const QVariant& value, int role)
{
    if (!isIndexValid(_index)
            || role != DBRoles::NameRole) {
        return false;
    }
    Album& album = *m_vAlbums->at(_index.row());
    album.set_sAlbumName(value.toString());
    m_sqlDB.albumDao().updateAlbum(album);
    emit dataChanged(_index, _index);
    return true;
}

bool AlbumModel::removeRows(int row, int count, const QModelIndex& parent)
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
        m_sqlDB.albumDao().removeAlbum(album._nAlbumID());
    }
    m_vAlbums->erase(m_vAlbums->begin() + row,
                  m_vAlbums->begin() + row + count);
    endRemoveRows();
    return true;
}

QHash<int, QByteArray> AlbumModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DBRoles::IdRole] = "id";
    roles[DBRoles::NameRole] = "name";
    return roles;
}

bool AlbumModel::isIndexValid(const QModelIndex& _index) const
{
    return _index.isValid() && _index.row() < rowCount();
}
