#ifndef BLETRANSPORT_H
#define BLETRANSPORT_H

#include <QBluetoothDeviceInfo>
#include <QList>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyService>

#include "SensorTransport.h"
#include "spectro-core_global.h"

class QLowEnergyController;

// SensorTransport over Bluetooth LE: connects to a bridge advertising the
// BridgeContract service, subscribes to the data characteristic (every
// notification payload is emitted as bytesReceived) and writes commands to
// the control characteristic. BLE setup is asynchronous while open() is
// synchronous, so open() only initiates the connection; command bytes
// written before the control characteristic is discovered are queued and
// flushed once it appears — callers never have to care about GATT phases.
class SPECTROCORE_EXPORT BleTransport : public SensorTransport
{
    Q_OBJECT

public:
    explicit BleTransport(const QBluetoothDeviceInfo& info, QObject* parent = nullptr);

    bool open(QString* errorMessage) override;
    void close() override;
    void writeBytes(const QByteArray& bytes) override;

private:
    void onControllerConnected();
    void onServiceDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void fail(const QString& message);

    QBluetoothDeviceInfo m_info;
    QLowEnergyController* m_controller = nullptr;
    QLowEnergyService* m_service = nullptr;
    QLowEnergyCharacteristic m_control;
    QList<QByteArray> m_pendingWrites;
};

#endif // BLETRANSPORT_H
