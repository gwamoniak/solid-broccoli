#include "CaptureCoordinator.h"

#include <QUrl>

#include "CameraService.h"
#include "PictureModel.h"
#include "logger.h"

CaptureCoordinator::CaptureCoordinator(CameraService& camera, PictureModel& pictures,
                                       QObject* parent)
    : QObject(parent), m_pictures(pictures)
{
    connect(&camera, &CameraService::imageSaved,
            this, &CaptureCoordinator::onImageSaved);
}

void CaptureCoordinator::onImageSaved(const QString& filePath)
{
    // Pictures are scoped to an album. If the user captured without an album
    // loaded, the file is still on disk but there is nowhere to file it yet;
    // record that rather than inserting an orphan row.
    if (m_pictures.getAlbumId() <= 0) {
        qWarning(logWarning())
            << "Captured image has no active album, not registered in gallery:"
            << filePath;
        return;
    }

    m_pictures.addPictureFromUrl(QUrl::fromLocalFile(filePath));
    qDebug(logInfo()) << "Captured image registered in album"
                      << m_pictures.getAlbumId() << ":" << filePath;
}
