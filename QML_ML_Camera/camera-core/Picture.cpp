#include "Picture.h"

Picture::Picture(const QString& _pictureFilePath) :
    Picture(QUrl::fromLocalFile(_pictureFilePath))
{

}

Picture::Picture(const QUrl& _pictureFileUrl) :
    m_nPictureID(-1),
    m_nAlbumId(-1),
    m_FileUrl(_pictureFileUrl)
{
}

QUrl Picture::_FileUrl() const
{
    return m_FileUrl;
}

void Picture::set_FileUrl(const QUrl& _FileUrl)
{
    m_FileUrl = _FileUrl;
}

int Picture::_nPictureID() const
{
    return m_nPictureID;
}

void Picture::set_nPictureID(int _nPictureID)
{
    m_nPictureID = _nPictureID;
}

int Picture::_nAlbumID() const
{
    return m_nAlbumId;
}

void Picture::set_nAlbumID(int _nAlbumID)
{
    m_nAlbumId = _nAlbumID;
}
