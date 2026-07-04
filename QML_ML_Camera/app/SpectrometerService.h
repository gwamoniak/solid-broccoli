#ifndef SPECTROMETERSERVICE_H
#define SPECTROMETERSERVICE_H

#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QStringList>

#include "Spectrum.h"

class PeakListModel;
class SensorDevice;
class SessionModel;
class SessionSpectrumModel;

// Single C++ owner of the spectrometer pipeline, mirroring CameraService:
// QML never constructs device objects — it binds to these properties and
// calls the invokables. Owns the device list (simulated instruments now,
// BLE hardware from Milestone 9), the active connection, acquisition state,
// and the latest spectrum with derived readouts.
class SpectrometerService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY availableDevicesChanged)
    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool acquiring READ isAcquiring NOTIFY acquiringChanged)
    Q_PROPERTY(int integrationTimeMs READ integrationTimeMs WRITE setIntegrationTimeMs NOTIFY integrationTimeMsChanged)
    Q_PROPERTY(int averaging READ averaging WRITE setAveraging NOTIFY averagingChanged)
    Q_PROPERTY(double peakWavelengthNm READ peakWavelengthNm NOTIFY spectrumUpdated)
    Q_PROPERTY(double peakValue READ peakValue NOTIFY spectrumUpdated)
    Q_PROPERTY(double framesPerSecond READ framesPerSecond NOTIFY spectrumUpdated)
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool hold READ hold WRITE setHold NOTIFY holdChanged)
    Q_PROPERTY(bool hasDark READ hasDark NOTIFY calibrationChanged)
    Q_PROPERTY(bool hasReference READ hasReference NOTIFY calibrationChanged)
    Q_PROPERTY(int smoothingWindow READ smoothingWindow WRITE setSmoothingWindow NOTIFY smoothingWindowChanged)
    Q_PROPERTY(double integrationFromNm READ integrationFromNm NOTIFY integrationRegionChanged)
    Q_PROPERTY(double integrationToNm READ integrationToNm NOTIFY integrationRegionChanged)
    Q_PROPERTY(bool hasIntegrationRegion READ hasIntegrationRegion NOTIFY integrationRegionChanged)
    Q_PROPERTY(double integralValue READ integralValue NOTIFY spectrumUpdated)
    Q_PROPERTY(QObject* peakModel READ peakModel CONSTANT)
    Q_PROPERTY(QVariantList overlayIds READ overlayIds NOTIFY overlaysChanged)

public:
    explicit SpectrometerService(QObject* parent = nullptr);

    QStringList availableDevices() const;
    int currentDeviceIndex() const { return m_currentDeviceIndex; }
    void setCurrentDeviceIndex(int index);
    bool isConnected() const;
    bool isAcquiring() const { return m_acquiring; }

    int integrationTimeMs() const { return m_integrationTimeMs; }
    void setIntegrationTimeMs(int ms);
    int averaging() const { return m_averaging; }
    void setAveraging(int count);

    double peakWavelengthNm() const { return m_peakWavelengthNm; }
    double peakValue() const { return m_peakValue; }
    double framesPerSecond() const { return m_framesPerSecond; }

    // Display mode: 0 = raw counts, 1 = transmittance, 2 = absorbance.
    // Modes 1/2 refuse to engage until both dark and reference exist.
    int mode() const { return m_mode; }
    void setMode(int mode);
    bool hold() const { return m_hold; }
    void setHold(bool hold);
    bool hasDark() const { return m_dark.isValid(); }
    bool hasReference() const { return m_reference.isValid(); }

    // Savitzky-Golay smoothing of the display spectrum: 0 = off, else an odd
    // window 5-25 (the setter normalizes).
    int smoothingWindow() const { return m_smoothingWindow; }
    void setSmoothingWindow(int window);

    double integrationFromNm() const { return m_integrationFromNm; }
    double integrationToNm() const { return m_integrationToNm; }
    bool hasIntegrationRegion() const;
    double integralValue() const { return m_integralValue; }
    QObject* peakModel() const;
    double peakProminence() const { return m_peakProminence; }
    void setPeakProminence(double prominence) { m_peakProminence = prominence; }

    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void selectNextDevice();
    Q_INVOKABLE bool connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void startAcquisition();
    Q_INVOKABLE void stopAcquisition();
    Q_INVOKABLE void captureDark();
    Q_INVOKABLE void captureReference();
    Q_INVOKABLE void setIntegrationRegion(double fromNm, double toNm);
    Q_INVOKABLE void clearIntegrationRegion();
    Q_INVOKABLE void saveCapture(const QString& name, const QString& tags);
    Q_INVOKABLE void toggleOverlay(int spectrumId);
    Q_INVOKABLE void clearOverlays();
    Q_INVOKABLE bool exportCapture(int spectrumId);

    // Session persistence (models owned by main; null in headless tests,
    // where saveCapture degrades to an error signal).
    void setSessionStore(SessionModel* sessions, SessionSpectrumModel* spectra);
    int activeSessionId() const { return m_activeSessionId; }
    QVariantList overlayIds() const;
    QVector<Spectrum> overlaySpectra() const;

    // Thread-safe copy of the latest spectrum, for readers on other threads
    // (the video-overlay processor in Milestone 8).
    Spectrum latestSpectrumSnapshot() const;

    // GUI-thread-only accessors for the plot item: the mode-transformed
    // display trace (frozen while hold is on) and the calibration traces.
    const Spectrum& displaySpectrum() const { return m_hold ? m_held : m_display; }
    const Spectrum& darkSpectrum() const { return m_dark; }
    const Spectrum& referenceSpectrum() const { return m_reference; }

signals:
    void availableDevicesChanged();
    void currentDeviceIndexChanged();
    void connectedChanged();
    void acquiringChanged();
    void integrationTimeMsChanged();
    void averagingChanged();
    void modeChanged();
    void holdChanged();
    void calibrationChanged();
    void smoothingWindowChanged();
    void integrationRegionChanged();
    void overlaysChanged();
    void captureSaved(const QString& sessionName);
    void spectrumUpdated();
    void errorOccurred(const QString& message);

private:
    enum class PendingCapture { None, Dark, Reference };

    SensorDevice* currentDevice() const;
    void attachDevice(SensorDevice* device);
    void onSpectrum(SensorDevice* device, const Spectrum& spectrum);
    void setAcquiring(bool acquiring);
    void resetCalibration();
    void rebuildDisplay();
    void recomputeDerived();
    AcquisitionParams acquisitionParams() const;

    QList<SensorDevice*> m_devices;
    int m_currentDeviceIndex = 0;
    bool m_acquiring = false;
    int m_integrationTimeMs = 100;
    int m_averaging = 1;

    int m_mode = 0;
    bool m_hold = false;
    int m_smoothingWindow = 0;
    double m_peakProminence = 0.0;  // 0 = auto (5% of display range)
    double m_integrationFromNm = qQNaN();
    double m_integrationToNm = qQNaN();
    double m_integralValue = qQNaN();
    PeakListModel* m_peakModel = nullptr;

    SessionModel* m_sessionModel = nullptr;
    SessionSpectrumModel* m_spectrumModel = nullptr;
    int m_activeSessionId = -1;
    bool m_darkStored = false;
    bool m_referenceStored = false;
    QVector<QPair<int, Spectrum>> m_overlays;  // stored-spectrum id -> data
    Spectrum m_dark;
    Spectrum m_reference;
    Spectrum m_display;
    Spectrum m_held;
    PendingCapture m_pending = PendingCapture::None;
    double m_restoreConcentration = 1.0;

    Spectrum m_liveSpectrum;
    mutable QMutex m_spectrumMutex;

    double m_peakWavelengthNm = 0.0;
    double m_peakValue = 0.0;
    double m_framesPerSecond = 0.0;
    QElapsedTimer m_frameClock;
};

#endif // SPECTROMETERSERVICE_H
