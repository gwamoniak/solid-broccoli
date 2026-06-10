#include "MovieAlbum.h"

MovieAlbum::MovieAlbum(const QString& name) :
    m_nAlbumID(-1),
    m_sAlbumName(name)
{
}

int MovieAlbum::_nAlbumID() const
{
    return m_nAlbumID;
}

void MovieAlbum::set_nAlbumID(int id)
{
    m_nAlbumID = id;
}

QString MovieAlbum::_sAlbumName() const
{
    return m_sAlbumName;
}

void MovieAlbum::set_sAlbumName(const QString& name)
{
    m_sAlbumName = name;
}
