#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QUrl>

// Small persisted UI-preferences object exposed to QML as a singleton. Each
// property is backed by QSettings so a choice survives restarts, and emits a
// change signal so any view bound to it updates immediately. Keeping this in
// C++ (rather than Qt.labs.settings in QML) follows the project's thin-view
// rule: state and persistence live in C++, QML only binds.
class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool shutterFlash READ shutterFlash WRITE setShutterFlash NOTIFY shutterFlashChanged)
    Q_PROPERTY(bool mirrorPreview READ mirrorPreview WRITE setMirrorPreview NOTIFY mirrorPreviewChanged)
    Q_PROPERTY(bool spectrumOverlay READ spectrumOverlay WRITE setSpectrumOverlay NOTIFY spectrumOverlayChanged)
    Q_PROPERTY(double geigerTubeFactor READ geigerTubeFactor WRITE setGeigerTubeFactor NOTIFY geigerTubeFactorChanged)
    Q_PROPERTY(double geigerAlertThreshold READ geigerAlertThreshold WRITE setGeigerAlertThreshold NOTIFY geigerAlertThresholdChanged)
    Q_PROPERTY(bool objectDetection READ objectDetection WRITE setObjectDetection NOTIFY objectDetectionChanged)
    Q_PROPERTY(QString detectionModelPath READ detectionModelPath NOTIFY detectionModelPathChanged)
    Q_PROPERTY(QString detectionModelName READ detectionModelName NOTIFY detectionModelPathChanged)
    Q_PROPERTY(int detectionStride READ detectionStride WRITE setDetectionStride NOTIFY detectionStrideChanged)

public:
    explicit AppSettings(QObject* parent = nullptr);

    bool shutterFlash() const;
    void setShutterFlash(bool enabled);

    bool mirrorPreview() const;
    void setMirrorPreview(bool enabled);

    bool spectrumOverlay() const;
    void setSpectrumOverlay(bool enabled);

    double geigerTubeFactor() const;
    void setGeigerTubeFactor(double factor);

    double geigerAlertThreshold() const;
    void setGeigerAlertThreshold(double threshold);

    bool objectDetection() const;
    void setObjectDetection(bool enabled);

    QString detectionModelPath() const;
    QString detectionModelName() const;
    int detectionStride() const;
    void setDetectionStride(int stride);

    // Copies a .onnx file into StorageLocations::modelsDir(), records its
    // SHA-256 alongside, and makes it the active detection model. Returns
    // false (with modelImportFailed) on any I/O problem.
    Q_INVOKABLE bool importDetectionModel(const QUrl& source);
    Q_INVOKABLE void revealModelsDir();

signals:
    void shutterFlashChanged();
    void mirrorPreviewChanged();
    void spectrumOverlayChanged();
    void geigerTubeFactorChanged();
    void geigerAlertThresholdChanged();
    void objectDetectionChanged();
    void detectionModelPathChanged();
    void detectionStrideChanged();
    void modelImportFailed(const QString& message);

private:
    QSettings m_settings;
    bool m_shutterFlash;
    bool m_mirrorPreview;
    bool m_spectrumOverlay;
    double m_geigerTubeFactor;
    double m_geigerAlertThreshold;
    bool m_objectDetection;
    QString m_detectionModelPath;
    int m_detectionStride;
};

#endif // APPSETTINGS_H
