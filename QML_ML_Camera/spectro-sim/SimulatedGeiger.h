#ifndef SIMULATEDGEIGER_H
#define SIMULATEDGEIGER_H

#include <random>

#include <QElapsedTimer>
#include <QTimer>

#include "SensorDevice.h"
#include "spectro-sim_global.h"

// What the simulated tube is exposed to: a mean count rate with an optional
// slow sinusoidal drift (a check source moving relative to the tube).
struct SPECTROSIM_EXPORT GeigerScene
{
    double meanCps = 0.3;          // natural background ≈ 18 CPM
    double driftAmplitude = 0.0;   // fraction of meanCps
    double driftPeriodMs = 60000.0;
    quint32 seed = 1;
};

// A Geiger counter that needs no hardware and no transport. Radioactive
// decay is a Poisson process, so each 500 ms tick draws an event count from
// a seeded std::poisson_distribution at the scene rate and reports it as
// counts-per-second — the modality's ground truth: recovered mean must match
// the scene rate and the count variance must equal the mean.
class SPECTROSIM_EXPORT SimulatedGeiger : public GeigerDevice
{
    Q_OBJECT

public:
    SimulatedGeiger(QString name, GeigerScene scene, QObject* parent = nullptr);

    static SimulatedGeiger* background(QObject* parent = nullptr);   // ~0.3 CPS
    static SimulatedGeiger* checkSource(QObject* parent = nullptr);  // ~50 CPS, slow drift

    bool connectDevice(QString* errorMessage) override;
    void disconnectDevice() override;
    void start() override;
    void stop() override;
    void setParams(const AcquisitionParams& params) override;  // tick rate is fixed
    QString name() const override;

    const GeigerScene& scene() const { return m_scene; }

    // Test hook: emit count readings synchronously on a synthetic clock
    // advancing one tick per reading — fully deterministic under the seed.
    void emitTicks(int count);

    static constexpr int tickMs = 500;  // 2 Hz emission

private:
    void tick();
    GeigerReading draw(qint64 timestampMs);

    QString m_name;
    GeigerScene m_scene;
    std::mt19937 m_rng;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    qint64 m_syntheticClockMs = 0;
};

#endif // SIMULATEDGEIGER_H
