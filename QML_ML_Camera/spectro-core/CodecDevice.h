#ifndef CODECDEVICE_H
#define CODECDEVICE_H

#include <memory>

#include "ProtocolCodec.h"
#include "SensorDevice.h"
#include "SensorTransport.h"
#include "spectro-core_global.h"

// The generic hardware device: any transport + any codec. Bytes arriving on
// the transport are fed to the codec; readings the codec delivers are emitted
// on the typed signal matching the SensorReading alternative; lifecycle calls
// are encoded by the codec and written to the transport. BLE, serial, and the
// simulated bridge all plug in here without a new device class.
class SPECTROCORE_EXPORT CodecDevice : public SensorDevice
{
    Q_OBJECT

public:
    CodecDevice(QString name,
                std::unique_ptr<SensorTransport> transport,
                std::unique_ptr<ProtocolCodec> codec,
                QObject* parent = nullptr);
    ~CodecDevice() override;

    bool connectDevice(QString* errorMessage) override;
    void disconnectDevice() override;
    void start() override;
    void stop() override;
    void setParams(const AcquisitionParams& params) override;
    QString name() const override;

private:
    QString m_name;
    std::unique_ptr<SensorTransport> m_transport;
    std::unique_ptr<ProtocolCodec> m_codec;
    AcquisitionParams m_params;
};

#endif // CODECDEVICE_H
