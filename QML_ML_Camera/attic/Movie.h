#ifndef MOVIE_H
#define MOVIE_H
#include <QUrl>
#include <QString>

#include "camera-core_global.h"

class CAMERACORESHARED_EXPORT Movie
{
public:
    Movie(const QString& _FilePath = "", const QString& _ThumbnailFilePath = "");
    Movie(const QUrl&    _FileUrl, const QUrl& _ThumbnailFilePath);

    int     _nMovieID() const;
    void set_nMovieID(int _nMovieID);

    int     _nAlbumID() const; // _nmAlbumId - avoid to have the same variables name. "m" is from Movie class
    void set_nAlbumID(int _nAlbumID);

    QUrl    _FileUrl() const;
    void set_movieFileUrl(const QUrl& _FileUrl);

    QUrl    _ThumbnailFileUrl() const;
    void set_ThumbnailmovieFileUrl(const QUrl& _FileUrl);

private:

  int m_nMovieID;
  int m_nAlbumID;
  QUrl m_FileUrl;
  QUrl m_ThumbnailFileUrl;
};

#endif // MOVIE_H
