#include "movie.h"

Movie::Movie(const QString& _FilePath, const QString& _ThumbnailFilePath) :
       Movie(QUrl::fromLocalFile(_FilePath),QUrl::fromLocalFile(_ThumbnailFilePath))
{

}

Movie::Movie(const QUrl &_FileUrl, const QUrl& _ThumbnailFilePath) :
  m_nMovieID(-1),
  m_nAlbumID(-1),
  m_FileUrl(_FileUrl),
  m_ThumbnailFileUrl(_ThumbnailFilePath)
{

}

int Movie::_nMovieID() const
{
    return m_nMovieID;
}

void Movie::set_nMovieID(int _nMovieID)
{
    m_nMovieID = _nMovieID;
}

int Movie::_nAlbumID() const
{
    return m_nAlbumID;
}

void Movie::set_nAlbumID(int _nAlbumID)
{
    m_nAlbumID = _nAlbumID;
}

QUrl Movie::_FileUrl() const
{
  return m_FileUrl;
}

void Movie::set_movieFileUrl(const QUrl &_FileUrl)
{
    m_FileUrl = _FileUrl;
}

QUrl Movie::_ThumbnailFileUrl() const
{
    return m_ThumbnailFileUrl;
}

void Movie::set_ThumbnailmovieFileUrl(const QUrl &_FileUrl)
{
    m_ThumbnailFileUrl = _FileUrl;
}


