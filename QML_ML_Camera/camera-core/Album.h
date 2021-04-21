#ifndef ALBUM_H
#define ALBUM_H

#include <QString>
#include <QDebug>

#include "camera-core_global.h"


class CAMERACORESHARED_EXPORT Album
{
public:
    explicit Album(const QString& _sAlbumName = "");

    int _nAlbumID() const;
    void set_nAlbumID(int _nAlbumID);
    QString _sAlbumName() const;
    void set_sAlbumName(const QString& _sAlbumName);

private:
    int m_nAlbumID;
    QString m_sAlbumName;
};

#endif // ALBUM_H
