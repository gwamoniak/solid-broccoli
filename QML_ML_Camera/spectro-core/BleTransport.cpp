#include "BleTransport.h"

#include <utility>

#include <QBluetoothUuid>
#include <QLowEnergyController>

#include "BridgeContract.h"
#include "loggingcategories.h"

BleTransport::BleTransport(const QBluetoothDeviceInfo& info, QObject* parent)
    : SensorTransport(parent)
    , m_info(info)
{
}

bool BleTransport::open(QString* errorMessage)
{
    Q_UNUSED(errorMessage)
    if (m_controller)
        return true;

    qDebug(logInfo()) << "BleTransport: connecting to" << m_info.name();
    m_controller = QLowEnergyController::createCentral(m_info, this);

    connect(m_controller, &QLowEnergyController::connected,
            this, &BleTransport::onControllerConnected);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BleTransport::onServiceDiscoveryFinished);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, [this](QLowEnergyController::Error) {
                fail(QStringLiteral("BLE error: %1").arg(m_controller->errorString()));
            });
    connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
        qDebug(logInfo()) << "BleTransport: disconnected from" << m_info.name();
        emit closed();
    });

    m_controller->connectToDevice();
    return true;
}

void BleTransport::onControllerConnected()
{
    qDebug(logInfo()) << "BleTransport: connected, discovering services.";
    m_controller->discoverServices();
}

void BleTransport::onServiceDiscoveryFinished()
{
    const QBluetoothUuid uuid(BridgeContract::serviceUuid());
    m_service = m_controller->createServiceObject(uuid, this);
    if (!m_service) {
        fail(QStringLiteral("Device does not offer the bridge service."));
        m_controller->disconnectFromDevice();
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &BleTransport::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, [this](const QLowEnergyCharacteristic& characteristic,
                         const QByteArray& value) {
                if (characteristic.uuid()
                    == QBluetoothUuid(BridgeContract::dataCharacteristicUuid()))
                    emit bytesReceived(value);
            });
    m_service->discoverDetails();
}

void BleTransport::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    if (state != QLowEnergyService::RemoteServiceDiscovered)
        return;

    const QLowEnergyCharacteristic data = m_service->characteristic(
        QBluetoothUuid(BridgeContract::dataCharacteristicUuid()));
    m_control = m_service->characteristic(
        QBluetoothUuid(BridgeContract::controlCharacteristicUuid()));
    if (!data.isValid() || !m_control.isValid()) {
        fail(QStringLiteral("Bridge service is missing its characteristics."));
        return;
    }

    // Enable notifications on the data characteristic (CCCD = 0x0100).
    const QLowEnergyDescriptor notification = data.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!notification.isValid()) {
        fail(QStringLiteral("Data characteristic cannot notify."));
        return;
    }
    m_service->writeDescriptor(notification, QByteArray::fromHex("0100"));
    qDebug(logInfo()) << "BleTransport: subscribed to data notifications.";

    const auto pending = std::exchange(m_pendingWrites, {});
    for (const QByteArray& bytes : pending)
        writeBytes(bytes);
}

void BleTransport::writeBytes(const QByteArray& bytes)
{
    if (!m_service || !m_control.isValid()) {
        m_pendingWrites.append(bytes);
        return;
    }
    const auto mode = (m_control.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                          ? QLowEnergyService::WriteWithoutResponse
                          : QLowEnergyService::WriteWithResponse;
    m_service->writeCharacteristic(m_control, bytes, mode);
}

void BleTransport::close()
{
    m_pendingWrites.clear();
    m_control = QLowEnergyCharacteristic();
    if (m_service) {
        m_service->deleteLater();
        m_service = nullptr;
    }
    if (m_controller) {
        m_controller->disconnectFromDevice();
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

void BleTransport::fail(const QString& message)
{
    qWarning(logCritical()) << "BleTransport:" << message;
    emit transportError(message);
}
