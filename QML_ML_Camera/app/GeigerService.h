#ifndef GEIGERSERVICE_H
#define GEIGERSERVICE_H

#include <functional>

#include <QMutex>
#include <QObject>
#include <QPointF>
#include <QStringList>
#include <QVector>

#include "GeigerMath.h"
#include "Spectrum.h"

class MeasurementDAO;
class SensorDevice;

// Third and smallest service, following CameraService/SpectrometerService:
// owns the Geiger device list, the active connection, and the derived
// radiation readouts (rolling CPM, dose rate, alert flag). The codec/device
// layer reports raw counts-per-second; everything shown is derived here.
class GeigerService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY availableDevicesChanged)
    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool acquiring READ isAcquiring NOTIFY acquiringChanged)
    Q_PROPERTY(double countsPerSecond READ countsPerSecond NOTIFY readingUpdated)
    Q_PROPERTY(double countsPerMinute READ countsPerMinute NOTIFY readingUpdated)
    Q_PROPERTY(double doseMicroSvPerHour READ doseMicroSvPerHour NOTIFY readingUpdated)
    Q_PROPERTY(bool aboveThreshold READ aboveThreshold NOTIFY readingUpdated)
    Q_PROPERTY(double tubeFactor READ tubeFactor WRITE setTubeFactor NOTIFY tubeFactorChanged)
    Q_PROPERTY(double alertThreshold READ alertThreshold WRITE setAlertThreshold NOTIFY alertThresholdChanged)

public:
    // Snapshot for the video-overlay processor (read from the camera worker
    // thread via overlayStatus()).
    struct OverlayStatus {
        bool active = false;
        bool alert = false;
        double doseMicroSvPerHour = 0.0;
        double countsPerMinute = 0.0;
    };

    explicit GeigerService(QObject* parent = nullptr);

    QStringList availableDevices() const;
    int currentDeviceIndex() const { return m_currentDeviceIndex; }
    void setCurrentDeviceIndex(int index);
    bool isConnected() const;
    bool isAcquiring() const { return m_acquiring; }

    double countsPerSecond() const { return m_latestCps; }
    double countsPerMinute() const { return m_window.cpm(); }
    double doseMicroSvPerHour() const;
    bool aboveThreshold() const;

    double tubeFactor() const { return m_tubeFactor; }
    void setTubeFactor(double factor);
    double alertThreshold() const { return m_alertThreshold; }
    void setAlertThreshold(double threshold);

    Q_INVOKABLE void selectNextDevice();
    Q_INVOKABLE bool connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void startAcquisition();
    Q_INVOKABLE void stopAcquisition();
    Q_INVOKABLE void saveMeasurement(const QString& name, const QString& tags);
    Q_INVOKABLE bool exportWindowCsv();

    // Persistence wiring from main: the DAO and a provider returning the id
    // of the active spectroscopy session (creating one if needed). Null in
    // headless tests, where saveMeasurement degrades to an error signal.
    void setMeasurementStore(const MeasurementDAO* dao,
                             std::function<int()> activeSessionProvider);

    // GUI-thread series for StripChartView: (timestampMs, dose µSv/h).
    const QVector<QPointF>& doseSeries() const { return m_doseSeries; }

    // Thread-safe copy for the video-overlay processor.
    OverlayStatus overlayStatus() const;

signals:
    void availableDevicesChanged();
    void currentDeviceIndexChanged();
    void connectedChanged();
    void acquiringChanged();
    void tubeFactorChanged();
    void alertThresholdChanged();
    void readingUpdated();
    void measurementSaved();
    void errorOccurred(const QString& message);

private:
    SensorDevice* currentDevice() const;
    void attachDevice(SensorDevice* device);
    void onReading(SensorDevice* device, const GeigerReading& reading);
    void setAcquiring(bool acquiring);
    void resetWindow();

    QList<SensorDevice*> m_devices;
    int m_currentDeviceIndex = 0;
    bool m_acquiring = false;

    double m_tubeFactor = 0.0057;      // SBM-20 approximation, µSv/h per CPM
    double m_alertThreshold = 0.5;     // µSv/h

    GeigerMath::RollingWindow m_window{60000};
    double m_latestCps = 0.0;
    QVector<QPointF> m_doseSeries;     // (timestampMs, µSv/h), window + slack

    const MeasurementDAO* m_measurementDao = nullptr;
    std::function<int()> m_activeSessionProvider;

    mutable QMutex m_statusMutex;
    OverlayStatus m_status;
};

#endif // GEIGERSERVICE_H
