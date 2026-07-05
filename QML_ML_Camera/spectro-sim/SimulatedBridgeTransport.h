#ifndef SIMULATEDBRIDGETRANSPORT_H
#define SIMULATEDBRIDGETRANSPORT_H

#include <random>

#include <QElapsedTimer>
#include <QTimer>

#include "FaultPolicy.h"
#include "InstrumentModel.h"
#include "SensorTransport.h"
#include "SimScene.h"
#include "spectro-sim_global.h"

// Byte-for-byte impersonation of the ESP32 bridge firmware: parses the
// control commands of BridgeContract.h and, while started, emits
// length-prefixed spectrum frames generated from a SimScene through the
// 18-channel AS7265x grid and the InstrumentModel, chunked to a configurable
// MTU to mimic BLE notifications. This class is the executable specification
// of the firmware contract — the firmware is correct exactly when the app
// cannot distinguish it from this transport. It deliberately builds its
// frame bytes itself rather than sharing an encoder with As7265xCodec, so
// the parser and the generator stay independent implementations.
class SPECTROSIM_EXPORT SimulatedBridgeTransport : public SensorTransport
{
    Q_OBJECT

public:
    explicit SimulatedBridgeTransport(SimScene scene, InstrumentProfile profile = {},
                                      QObject* parent = nullptr);

    // The scene the app's loopback device shows: the mercury lamp.
    static SimulatedBridgeTransport* mercuryLamp(QObject* parent = nullptr);
    // Geiger-mode loopback: type-0x02 frames from a Poisson draw at meanCps.
    static SimulatedBridgeTransport* geigerSource(double meanCps,
                                                  QObject* parent = nullptr);

    // Switches outgoing frames to geiger type 0x02 (Milestone 10). The same
    // framing, chunking, and fault policy apply — that is the point.
    void enableGeigerMode(double meanCps);

    bool open(QString* errorMessage) override;
    void close() override;
    void writeBytes(const QByteArray& bytes) override;  // parses control commands

    void setMtu(int bytes) { m_mtu = bytes; }                       // default 20
    void setFrameIntervalMs(int ms) { m_frameIntervalMs = ms; }     // default 200
    void setFaultPolicy(const FaultPolicy& policy);

    bool isStarted() const { return m_started; }
    AcquisitionParams params() const { return m_params; }

    // Test hook: generate and deliver count frames synchronously (no event
    // loop); the frame timestamps advance by the frame interval, so the
    // whole sequence is deterministic under a fixed profile seed.
    void emitFrames(int count);

private:
    void rebuildFlux();
    void startStreaming();
    void stopStreaming();
    QByteArray buildFrame(quint32 timestampMs);
    void deliver(QByteArray frame);
    void tick();

    SimScene m_scene;
    InstrumentModel m_model;
    QVector<double> m_grid;
    QVector<double> m_flux;
    AcquisitionParams m_params;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    QByteArray m_control;        // partial control bytes across writes
    bool m_open = false;
    bool m_started = false;
    int m_mtu = 20;
    int m_frameIntervalMs = 200;
    int m_framesEmitted = 0;
    quint32 m_syntheticClockMs = 0;  // emitFrames() timestamps
    bool m_geigerMode = false;
    double m_geigerMeanCps = 5.0;
    std::mt19937 m_geigerRng{1};
    FaultPolicy m_faults;
    std::mt19937 m_faultRng{1};
};

#endif // SIMULATEDBRIDGETRANSPORT_H
