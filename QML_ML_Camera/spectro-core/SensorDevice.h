#ifndef SENSORDEVICE_H
#define SENSORDEVICE_H

#include <QObject>

#include "Spectrum.h"
#include "spectro-core_global.h"

// Abstract base for every hardware (or simulated) sensor. Services own a
// device, call the lifecycle methods, and bind to the signals; QML never sees
// this class directly.
//
// The typed ready signals for all modalities live here, not on the
// subclasses: Qt's moc cannot template signals, and the generic CodecDevice
// must emit whichever SensorReading alternative its codec delivers. A device
// simply never emits the signals that don't apply to it. (Decision recorded
// in SPECTRO_TRICORDER_EXECPLAN.md.)
class SPECTROCORE_EXPORT SensorDevice : public QObject
{
    Q_OBJECT

public:
    explicit SensorDevice(QObject* parent = nullptr);
    ~SensorDevice() override;

    virtual bool connectDevice(QString* errorMessage) = 0;
    virtual void disconnectDevice() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setParams(const AcquisitionParams& params) = 0;
    virtual QString name() const = 0;

    bool isConnected() const { return m_connected; }

signals:
    void connectedChanged(bool connected);
    void errorOccurred(const QString& message);
    void spectrumReady(const Spectrum& spectrum);
    void readingReady(const GeigerReading& reading);

protected:
    // Updates the connected flag and emits connectedChanged on change.
    void setConnected(bool connected);

private:
    bool m_connected = false;
};

// Semantic specialization for optical spectrometers (emits spectrumReady).
class SPECTROCORE_EXPORT SpectrometerDevice : public SensorDevice
{
    Q_OBJECT

public:
    using SensorDevice::SensorDevice;
};

// Semantic specialization for radiation counters (emits readingReady).
class SPECTROCORE_EXPORT GeigerDevice : public SensorDevice
{
    Q_OBJECT

public:
    using SensorDevice::SensorDevice;
};

#endif // SENSORDEVICE_H
