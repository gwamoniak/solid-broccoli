#include "Movie.h"

#include <QFileInfo>

Movie::Movie(const QUrl& _FileUrl, qint64 _nDurationMs, const QString& _sCreatedAt) :
    m_nMovieID(-1),
    m_nAlbumID(-1),
    m_FileUrl(_FileUrl),
    m_sName(QFileInfo(_FileUrl.fileName()).completeBaseName()),
    m_nDurationMs(_nDurationMs),
    m_sCreatedAt(_sCreatedAt)
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

void Movie::set_FileUrl(const QUrl& _FileUrl)
{
    m_FileUrl = _FileUrl;
}

QString Movie::_sName() const
{
    return m_sName;
}

void Movie::set_sName(const QString& _sName)
{
    m_sName = _sName;
}

qint64 Movie::_nDurationMs() const
{
    return m_nDurationMs;
}

void Movie::set_nDurationMs(qint64 _nDurationMs)
{
    m_nDurationMs = _nDurationMs;
}

QString Movie::_sCreatedAt() const
{
    return m_sCreatedAt;
}

void Movie::set_sCreatedAt(const QString& _sCreatedAt)
{
    m_sCreatedAt = _sCreatedAt;
}
