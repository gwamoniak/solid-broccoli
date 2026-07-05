#include "SimulatedSpectrometer.h"

#include <cmath>
#include <utility>

#include "SpectrumSynthesizer.h"
#include "loggingcategories.h"

SimulatedSpectrometer::SimulatedSpectrometer(QString name, SimScene scene,
                                             InstrumentProfile profile,
                                             QObject* parent)
    : SpectrometerDevice(parent)
    , m_name(std::move(name))
    , m_scene(scene)
    , m_model(profile)
    , m_grid(SpectrumSynthesizer::standardGrid())
{
    rebuildFlux();
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &SimulatedSpectrometer::tick);
}

SimulatedSpectrometer* SimulatedSpectrometer::mercuryLamp(QObject* parent)
{
    SimScene scene;
    scene.source = SimScene::Source::Mercury;
    return new SimulatedSpectrometer(
        QStringLiteral("Simulated UV-Vis (mercury lamp)"), scene, {}, parent);
}

SimulatedSpectrometer* SimulatedSpectrometer::dyeSample(QObject* parent)
{
    SimScene scene;
    scene.source = SimScene::Source::Blackbody;
    scene.absorberCenterNm = 664.0;
    scene.absorberFwhmNm = 45.0;
    scene.absorberPeakA = 0.8;
    scene.concentration = 1.0;
    return new SimulatedSpectrometer(
        QStringLiteral("Simulated dye sample (Beer-Lambert)"), scene, {}, parent);
}

bool SimulatedSpectrometer::connectDevice(QString*)
{
    setConnected(true);
    return true;
}

void SimulatedSpectrometer::disconnectDevice()
{
    m_timer.stop();
    setConnected(false);
}

void SimulatedSpectrometer::start()
{
    if (!isConnected())
        return;
    m_elapsed.start();
    m_timer.start(std::max(m_params.integrationTimeMs, 33));
    qDebug(logInfo()) << "SimulatedSpectrometer" << m_name << "acquiring at"
                      << m_timer.interval() << "ms interval";
}

void SimulatedSpectrometer::stop()
{
    m_timer.stop();
}

void SimulatedSpectrometer::setParams(const AcquisitionParams& params)
{
    m_params = params;
    if (m_timer.isActive())
        m_timer.start(std::max(m_params.integrationTimeMs, 33));
}

QString SimulatedSpectrometer::name() const
{
    return m_name;
}

void SimulatedSpectrometer::setLampEnabled(bool on)
{
    if (m_scene.lampOn == on)
        return;
    m_scene.lampOn = on;
    rebuildFlux();
}

void SimulatedSpectrometer::setSceneConcentration(double concentration)
{
    m_scene.concentration = concentration;
    rebuildFlux();
}

void SimulatedSpectrometer::rebuildFlux()
{
    namespace Synth = SpectrumSynthesizer;

    if (!m_scene.lampOn) {
        m_flux = QVector<double>(m_grid.size(), 0.0);
        return;
    }

    switch (m_scene.source) {
    case SimScene::Source::Mercury:
        m_flux = Synth::mercuryLamp(m_grid, m_scene.lineFwhmNm);
        break;
    case SimScene::Source::Neon:
        m_flux = Synth::neonLamp(m_grid, m_scene.lineFwhmNm);
        break;
    case SimScene::Source::Blackbody:
        m_flux = Synth::blackbody(m_grid, m_scene.blackbodyK);
        break;
    }

    if (m_scene.absorberPeakA > 0.0) {
        const QVector<double> a = Synth::gaussianAbsorber(m_grid, m_scene.absorberCenterNm,
                                                          m_scene.absorberFwhmNm,
                                                          m_scene.absorberPeakA);
        m_flux = Synth::applyBeerLambert(m_flux, a, m_scene.concentration);
    }
}

void SimulatedSpectrometer::tick()
{
    double drift = 1.0;
    if (m_scene.driftAmplitude > 0.0 && m_scene.driftPeriodMs > 0.0) {
        drift = 1.0 + m_scene.driftAmplitude
                          * std::sin(2.0 * M_PI * m_elapsed.elapsed() / m_scene.driftPeriodMs);
    }

    QVector<double> flux(m_flux.size());
    for (int i = 0; i < m_flux.size(); ++i)
        flux[i] = m_flux[i] * drift;

    emit spectrumReady(m_model.measure(flux, m_grid, m_params));
}
