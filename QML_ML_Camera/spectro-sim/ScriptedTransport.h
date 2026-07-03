#ifndef SCRIPTEDTRANSPORT_H
#define SCRIPTEDTRANSPORT_H

#include <random>

#include "FaultPolicy.h"
#include "SensorTransport.h"
#include "spectro-sim_global.h"

// Test double for anything that consumes a SensorTransport: the test scripts
// what the "device" sends (delivered in configurable chunk sizes so frame
// reassembly is exercised, optionally corrupted per FaultPolicy) and asserts
// on what the code under test wrote.
class SPECTROSIM_EXPORT ScriptedTransport : public SensorTransport
{
    Q_OBJECT

public:
    explicit ScriptedTransport(QObject* parent = nullptr);

    bool open(QString* errorMessage) override;
    void close() override;
    void writeBytes(const QByteArray& bytes) override;

    // Deliver bytes as if received from the device. delayMs 0 delivers
    // synchronously (assert immediately after); > 0 schedules a timer.
    void enqueueIncoming(const QByteArray& bytes, int delayMs = 0);

    void setChunkSize(int bytesPerEmission) { m_chunkSize = bytesPerEmission; }
    void setFaultPolicy(const FaultPolicy& policy);

    QByteArray writtenBytes() const { return m_written; }
    bool isOpen() const { return m_open; }
    bool failNextOpen = false;

private:
    void deliver(QByteArray bytes);

    bool m_open = false;
    int m_chunkSize = 0;  // 0 = whole payload in one emission
    QByteArray m_written;
    FaultPolicy m_faults;
    std::mt19937 m_rng{1};
};

#endif // SCRIPTEDTRANSPORT_H
