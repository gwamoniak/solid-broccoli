#ifndef SENSORTRANSPORT_H
#define SENSORTRANSPORT_H

#include <QByteArray>
#include <QObject>

#include "spectro-core_global.h"

// Byte-level link to a device: BLE, serial, or simulated. A transport moves
// bytes and nothing else — parsing belongs to ProtocolCodec (the sans-IO
// split that makes codecs testable by feeding them byte arrays).
class SPECTROCORE_EXPORT SensorTransport : public QObject
{
    Q_OBJECT

public:
    explicit SensorTransport(QObject* parent = nullptr) : QObject(parent) {}

    virtual bool open(QString* errorMessage) = 0;
    virtual void close() = 0;
    virtual void writeBytes(const QByteArray& bytes) = 0;

signals:
    void bytesReceived(const QByteArray& bytes);
    void transportError(const QString& message);
    void closed();
};

#endif // SENSORTRANSPORT_H
