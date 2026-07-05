#include "SimulatedGeiger.h"

#include <cmath>
#include <utility>

#include "loggingcategories.h"

SimulatedGeiger::SimulatedGeiger(QString name, GeigerScene scene, QObject* parent)
    : GeigerDevice(parent)
    , m_name(std::move(name))
    , m_scene(scene)
    , m_rng(scene.seed)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &SimulatedGeiger::tick);
}

SimulatedGeiger* SimulatedGeiger::background(QObject* parent)
{
    GeigerScene scene;
    scene.meanCps = 0.3;
    return new SimulatedGeiger(QStringLiteral("Simulated Geiger (background)"),
                               scene, parent);
}

SimulatedGeiger* SimulatedGeiger::checkSource(QObject* parent)
{
    GeigerScene scene;
    scene.meanCps = 50.0;
    scene.driftAmplitude = 0.15;  // source slowly moving relative to the tube
    scene.driftPeriodMs = 30000.0;
    return new SimulatedGeiger(QStringLiteral("Simulated Geiger (check source)"),
                               scene, parent);
}

bool SimulatedGeiger::connectDevice(QString*)
{
    setConnected(true);
    return true;
}

void SimulatedGeiger::disconnectDevice()
{
    m_timer.stop();
    setConnected(false);
}

void SimulatedGeiger::start()
{
    if (!isConnected())
        return;
    m_elapsed.start();
    m_timer.start(tickMs);
    qDebug(logInfo()) << "SimulatedGeiger" << m_name << "counting at"
                      << m_scene.meanCps << "CPS mean.";
}

void SimulatedGeiger::stop()
{
    m_timer.stop();
}

void SimulatedGeiger::setParams(const AcquisitionParams&)
{
    // Integration/averaging do not apply to a counter tube; the tick rate
    // is a property of this simulator.
}

QString SimulatedGeiger::name() const
{
    return m_name;
}

GeigerReading SimulatedGeiger::draw(qint64 timestampMs)
{
    double rate = m_scene.meanCps;
    if (m_scene.driftAmplitude > 0.0 && m_scene.driftPeriodMs > 0.0) {
        rate *= 1.0 + m_scene.driftAmplitude
                          * std::sin(2.0 * M_PI * timestampMs / m_scene.driftPeriodMs);
    }

    const double meanCounts = std::max(rate, 0.0) * tickMs / 1000.0;
    int counts = 0;
    if (meanCounts > 0.0) {
        std::poisson_distribution<int> distribution(meanCounts);
        counts = distribution(m_rng);
    }

    GeigerReading reading;
    reading.timestampMs = timestampMs;
    reading.countsPerSecond = counts * 1000.0 / tickMs;
    return reading;
}

void SimulatedGeiger::tick()
{
    emit readingReady(draw(m_elapsed.elapsed()));
}

void SimulatedGeiger::emitTicks(int count)
{
    for (int i = 0; i < count; ++i) {
        m_syntheticClockMs += tickMs;
        emit readingReady(draw(m_syntheticClockMs));
    }
}
