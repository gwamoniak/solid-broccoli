#ifndef SPECTROMETERSERVICE_H
#define SPECTROMETERSERVICE_H

#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QStringList>

#include "Spectrum.h"

class SensorDevice;

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

    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void selectNextDevice();
    Q_INVOKABLE bool connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void startAcquisition();
    Q_INVOKABLE void stopAcquisition();

    // Thread-safe copy of the latest spectrum, for readers on other threads
    // (the video-overlay processor in Milestone 8).
    Spectrum latestSpectrumSnapshot() const;

    // GUI-thread-only reference for the plot item (Milestone 5).
    const Spectrum& displaySpectrum() const { return m_liveSpectrum; }

signals:
    void availableDevicesChanged();
    void currentDeviceIndexChanged();
    void connectedChanged();
    void acquiringChanged();
    void integrationTimeMsChanged();
    void averagingChanged();
    void spectrumUpdated();
    void errorOccurred(const QString& message);

private:
    SensorDevice* currentDevice() const;
    void attachDevice(SensorDevice* device);
    void onSpectrum(SensorDevice* device, const Spectrum& spectrum);
    void setAcquiring(bool acquiring);
    AcquisitionParams acquisitionParams() const;

    QList<SensorDevice*> m_devices;
    int m_currentDeviceIndex = 0;
    bool m_acquiring = false;
    int m_integrationTimeMs = 100;
    int m_averaging = 1;

    Spectrum m_liveSpectrum;
    mutable QMutex m_spectrumMutex;

    double m_peakWavelengthNm = 0.0;
    double m_peakValue = 0.0;
    double m_framesPerSecond = 0.0;
    QElapsedTimer m_frameClock;
};

#endif // SPECTROMETERSERVICE_H
