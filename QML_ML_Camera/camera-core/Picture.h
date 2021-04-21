#ifndef PICTURE_H
#define PICTURE_H

#include <QUrl>
#include <QString>

#include "camera-core_global.h"

class CAMERACORESHARED_EXPORT Picture
{
public:
    Picture(const QString& _pictureFilePath = "");
    Picture(const QUrl& _pictureFileUrl);

    int _nPictureID() const;
    void set_nPictureID(int _nPictureID);

    int _nAlbumID() const;
    void set_nAlbumID(int _nAlbumID);

    QUrl _FileUrl() const;
    void set_FileUrl(const QUrl& _FileUrl);
private:
    int m_nPictureID;
    int m_nAlbumId;
    QUrl m_FileUrl;
};

#endif // PICTURE_H
