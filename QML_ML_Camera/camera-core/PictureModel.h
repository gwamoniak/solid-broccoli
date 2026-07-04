#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include <memory>
#include <vector>

#include <QAbstractListModel>

#include "camera-core_global.h"
#include "Picture.h"

class Album;
class DatabaseManager;
class AlbumModel;

class CAMERACORESHARED_EXPORT PictureModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum DBRoles {
        UrlRole = Qt::UserRole + 1,
        FilePathRole,
        NameRole,
    };
    PictureModel(DatabaseManager& db, const AlbumModel& _albumModel, QObject* parent = nullptr);

    QModelIndex addPicture(const Picture& _picture);
    Q_INVOKABLE void addPictureFromUrl(const QUrl& _FileUrl);
    // Files a picture into a specific album regardless of which album is
    // currently loaded for viewing. If it is the loaded album, the row is
    // also inserted live so the gallery updates without navigation.
    Q_INVOKABLE void addPictureToAlbum(int _nAlbumID, const QUrl& _FileUrl);
    Q_INVOKABLE void rename(int row, const QString& _name);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int role) const override;
    Q_INVOKABLE bool removeRows(int _row, int _count, const QModelIndex& parent = QModelIndex()) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setAlbumId(int _nAlbumID);
    Q_INVOKABLE int getAlbumId()
    {
        return m_nAlbumID;
    }
    void clearAlbum();

public slots:
    void deletePicturesForAlbum();

private:
    void loadPictures(int _nAlbumID);
    bool isIndexValid(const QModelIndex& _index) const;

private:
    DatabaseManager& m_sqlDB;
    int m_nAlbumID;
    std::unique_ptr<std::vector<std::unique_ptr<Picture>>> m_vPictures;
};

#endif // PICTUREMODEL_H
