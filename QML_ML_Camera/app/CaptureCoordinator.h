#ifndef CAPTURECOORDINATOR_H
#define CAPTURECOORDINATOR_H

#include <QObject>

class CameraService;
class PictureModel;

// Bridges a finished capture to the gallery: when CameraService reports an
// image saved to disk, it is registered in the picture model's currently
// loaded album so the gallery updates immediately, with no navigation. Keeping
// this out of CameraService leaves the service free of database/model
// knowledge and keeps the wiring testable from one place (main.cpp).
class CaptureCoordinator : public QObject
{
    Q_OBJECT
public:
    CaptureCoordinator(CameraService& camera, PictureModel& pictures,
                       QObject* parent = nullptr);

private:
    void onImageSaved(const QString& filePath);

    PictureModel& m_pictures;
};

#endif // CAPTURECOORDINATOR_H
