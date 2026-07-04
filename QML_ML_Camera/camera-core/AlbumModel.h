#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <vector>
#include <memory>

#include "camera-core_global.h"
#include "Album.h"
#include "DatabaseManager.h"

class CAMERACORESHARED_EXPORT AlbumModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum DBRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
    };

    explicit AlbumModel(DatabaseManager& db, QObject* parent = nullptr);

    QModelIndex addAlbum(const Album& _album);
    Q_INVOKABLE void addAlbumFromName(const QString& _name);
    // Creates an album and returns its new database id (for callers that
    // must immediately target it, e.g. the camera destination picker).
    Q_INVOKABLE int createAlbumReturningId(const QString& _name);
    Q_INVOKABLE void rename(int row, const QString& _name);

    // Lookups for callers that address albums by id rather than by row.
    Q_INVOKABLE int albumCount() const;
    Q_INVOKABLE int albumIdAt(int row) const;
    Q_INVOKABLE QString albumNameForId(int _nAlbumID) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& _index, const QVariant& value, int role) override;
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    bool isIndexValid(const QModelIndex& _index) const;

private:
    DatabaseManager& m_sqlDB;
    std::unique_ptr<std::vector<std::unique_ptr<Album>>> m_vAlbums;
};

#endif // ALBUMMODEL_H
