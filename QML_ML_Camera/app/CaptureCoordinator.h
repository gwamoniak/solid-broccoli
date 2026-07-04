#ifndef CAPTURECOORDINATOR_H
#define CAPTURECOORDINATOR_H

#include <QObject>
#include <QSettings>
#include <QString>

class AlbumModel;
class CameraService;
class PictureModel;

// Bridges a finished capture to the gallery and owns the capture destination.
// When CameraService reports an image saved to disk, it is filed into the
// camera's target album (chosen on the Camera page, persisted across
// restarts), independent of whichever album the Photos tab is currently
// viewing. A default album is created if none exists, so a capture is never
// silently dropped. Keeping this out of CameraService leaves the service free
// of database/model knowledge.
class CaptureCoordinator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int targetAlbumId READ targetAlbumId WRITE setTargetAlbum NOTIFY targetAlbumChanged)
    Q_PROPERTY(QString targetAlbumName READ targetAlbumName NOTIFY targetAlbumChanged)

public:
    CaptureCoordinator(CameraService& camera, PictureModel& pictures,
                       AlbumModel& albums, QObject* parent = nullptr);

    int targetAlbumId() const { return m_targetAlbumId; }
    QString targetAlbumName() const;
    void setTargetAlbum(int albumId);

    // Creates an album, selects it as the target, and returns its id.
    Q_INVOKABLE int createAlbumAndSelect(const QString& name);

signals:
    void targetAlbumChanged();
    void captureFiled(const QString& albumName);

private:
    void onImageSaved(const QString& filePath);
    // Guarantees m_targetAlbumId names an existing album, creating a default
    // "Camera Roll" when the gallery is empty.
    void ensureValidTarget();

    PictureModel& m_pictures;
    AlbumModel& m_albums;
    int m_targetAlbumId = -1;
    QSettings m_settings;
};

#endif // CAPTURECOORDINATOR_H
