#ifndef MOVIE_H
#define MOVIE_H

#include <QString>
#include <QUrl>

#include "camera-core_global.h"

class CAMERACORESHARED_EXPORT Movie
{
public:
    explicit Movie(const QUrl& _FileUrl = QUrl(),
                   qint64 _nDurationMs = 0,
                   const QString& _sCreatedAt = QString());

    int     _nMovieID() const;
    void set_nMovieID(int _nMovieID);

    int     _nAlbumID() const;
    void set_nAlbumID(int _nAlbumID);

    QUrl    _FileUrl() const;
    void set_FileUrl(const QUrl& _FileUrl);

    QString _sName() const;
    void set_sName(const QString& _sName);

    qint64  _nDurationMs() const;
    void set_nDurationMs(qint64 _nDurationMs);

    QString _sCreatedAt() const;
    void set_sCreatedAt(const QString& _sCreatedAt);

private:
    int m_nMovieID;
    int m_nAlbumID; // -1 = not assigned to an album
    QUrl m_FileUrl;
    QString m_sName;
    qint64 m_nDurationMs;
    QString m_sCreatedAt; // ISO 8601
};

#endif // MOVIE_H
