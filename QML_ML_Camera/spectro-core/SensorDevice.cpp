#include "SensorDevice.h"

SensorDevice::SensorDevice(QObject* parent)
    : QObject(parent)
{
}

SensorDevice::~SensorDevice() = default;

void SensorDevice::setConnected(bool connected)
{
    if (m_connected == connected)
        return;
    m_connected = connected;
    emit connectedChanged(m_connected);
}
