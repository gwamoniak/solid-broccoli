#include "Album.h"

Album::Album(const QString& name) :
    m_nAlbumID(-1),
    m_sAlbumName(name)
{
}

int Album::_nAlbumID() const
{
    return m_nAlbumID;
}

void Album::set_nAlbumID(int id)
{
    m_nAlbumID = id;
}

QString Album::_sAlbumName() const
{
    return m_sAlbumName;
}

void Album::set_sAlbumName(const QString& name)
{
    m_sAlbumName = name;
}
