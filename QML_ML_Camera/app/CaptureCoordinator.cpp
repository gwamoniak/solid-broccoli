#include "CaptureCoordinator.h"

#include <QUrl>

#include "AlbumModel.h"
#include "CameraService.h"
#include "PictureModel.h"
#include "logger.h"

CaptureCoordinator::CaptureCoordinator(CameraService& camera, PictureModel& pictures,
                                       AlbumModel& albums, QObject* parent)
    : QObject(parent), m_pictures(pictures), m_albums(albums)
{
    m_targetAlbumId = m_settings.value("camera/targetAlbumId", -1).toInt();
    ensureValidTarget();

    connect(&camera, &CameraService::imageSaved,
            this, &CaptureCoordinator::onImageSaved);
}

QString CaptureCoordinator::targetAlbumName() const
{
    return m_albums.albumNameForId(m_targetAlbumId);
}

void CaptureCoordinator::setTargetAlbum(int albumId)
{
    if (albumId == m_targetAlbumId || m_albums.albumNameForId(albumId).isEmpty()) {
        return;
    }
    m_targetAlbumId = albumId;
    m_settings.setValue("camera/targetAlbumId", albumId);
    qDebug(logInfo()) << "CaptureCoordinator: target album set to" << albumId
                      << targetAlbumName();
    emit targetAlbumChanged();
}

int CaptureCoordinator::createAlbumAndSelect(const QString& name)
{
    const QString trimmed = name.trimmed();
    const int id = m_albums.createAlbumReturningId(
        trimmed.isEmpty() ? QStringLiteral("New Album") : trimmed);
    setTargetAlbum(id);
    return id;
}

void CaptureCoordinator::ensureValidTarget()
{
    // A persisted target that still exists is honored as-is.
    if (m_targetAlbumId > 0 && !m_albums.albumNameForId(m_targetAlbumId).isEmpty()) {
        return;
    }
    // Otherwise fall back to the first album, creating a default one if the
    // gallery is empty so captures always have a home.
    if (m_albums.albumCount() == 0) {
        m_targetAlbumId = m_albums.createAlbumReturningId(QStringLiteral("Camera Roll"));
        qDebug(logInfo()) << "CaptureCoordinator: created default album"
                          << m_targetAlbumId;
    } else {
        m_targetAlbumId = m_albums.albumIdAt(0);
    }
    m_settings.setValue("camera/targetAlbumId", m_targetAlbumId);
    emit targetAlbumChanged();
}

void CaptureCoordinator::onImageSaved(const QString& filePath)
{
    ensureValidTarget();  // an album may have been deleted since last capture
    m_pictures.addPictureToAlbum(m_targetAlbumId, QUrl::fromLocalFile(filePath));
    qDebug(logInfo()) << "Captured image filed into album" << m_targetAlbumId
                      << "(" << targetAlbumName() << "):" << filePath;
    emit captureFiled(targetAlbumName());
}
