#ifndef MOVIEMODEL_H
#define MOVIEMODEL_H

#include <memory>
#include <vector>

#include <QAbstractListModel>

#include "camera-core_global.h"
#include "Movie.h"

class Album;
class DatabaseManager;
class MovieAlbumModel;

class CAMERACORESHARED_EXPORT MovieModel : public QAbstractListModel
{
     Q_OBJECT
public:

    enum DBRoles
    {
        UrlRole = Qt::UserRole + 1,
        FilePathRole,
        NameRole,
        ThumbnailUrlRole
    };

    MovieModel(const MovieAlbumModel& _albumModel, QObject* parent = nullptr);

    QModelIndex addMovie(const Movie& _movie);
    Q_INVOKABLE void addMovieFromUrl(const QUrl& _FileUrl, const QUrl& _ThumnailFilePath);
    Q_INVOKABLE void rename(int row, const QString& _name);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int _role) const override;
    Q_INVOKABLE bool removeRows(int _row, int _count, const QModelIndex& parent = QModelIndex()) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setAlbumId(int _nalbumId);
    void clearAlbum();

public slots:
      void deleteMoviesForAlbum();

private:
      void loadMovies(int _albumID);
      bool isIndexValid(const QModelIndex& _index) const;

private:
    DatabaseManager& m_sqlDB;
    int m_nAlbumID;

    std::unique_ptr<std::vector<std::unique_ptr<Movie>>> m_vMovies;


};

#endif // MOVIEMODEL_H
