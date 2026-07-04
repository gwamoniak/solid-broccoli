#include "SimulatedBridgeTransport.h"

#include <cmath>

#include <QDataStream>
#include <QIODevice>
#include <QtEndian>

#include "BridgeContract.h"
#include "SpectrumSynthesizer.h"
#include "loggingcategories.h"

SimulatedBridgeTransport::SimulatedBridgeTransport(SimScene scene,
                                                   InstrumentProfile profile,
                                                   QObject* parent)
    : SensorTransport(parent)
    , m_scene(scene)
    , m_model(profile)
    , m_grid(SpectrumSynthesizer::as7265xGrid())
{
    rebuildFlux();
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &SimulatedBridgeTransport::tick);
}

SimulatedBridgeTransport* SimulatedBridgeTransport::mercuryLamp(QObject* parent)
{
    SimScene scene;
    scene.source = SimScene::Source::Mercury;
    // The 18-channel grid is coarse (25-65 nm spacing); widen the lines so
    // they register on more than one channel.
    scene.lineFwhmNm = 20.0;
    return new SimulatedBridgeTransport(scene, {}, parent);
}

bool SimulatedBridgeTransport::open(QString* errorMessage)
{
    Q_UNUSED(errorMessage)
    m_open = true;
    return true;
}

void SimulatedBridgeTransport::close()
{
    if (!m_open)
        return;
    stopStreaming();
    m_open = false;
    m_control.clear();
    emit closed();
}

void SimulatedBridgeTransport::setFaultPolicy(const FaultPolicy& policy)
{
    m_faults = policy;
    m_faultRng.seed(policy.seed);
}

void SimulatedBridgeTransport::writeBytes(const QByteArray& bytes)
{
    if (!m_open)
        return;
    m_control.append(bytes);

    // Commands may arrive concatenated or split; consume what is complete.
    while (!m_control.isEmpty()) {
        const quint8 opcode = quint8(m_control[0]);
        if (opcode == BridgeContract::cmdStart) {
            m_control.remove(0, 1);
            startStreaming();
        } else if (opcode == BridgeContract::cmdStop) {
            m_control.remove(0, 1);
            stopStreaming();
        } else if (opcode == BridgeContract::cmdSetParams) {
            if (m_control.size() < 4)
                break;  // wait for the rest of the command
            m_params.integrationTimeMs = qFromLittleEndian<quint16>(
                reinterpret_cast<const uchar*>(m_control.constData()) + 1);
            m_params.averaging = quint8(m_control[3]);
            m_control.remove(0, 4);
            qDebug(logInfo()) << "SimulatedBridgeTransport: params"
                              << m_params.integrationTimeMs << "ms x"
                              << m_params.averaging;
        } else {
            qWarning(logCritical()) << "SimulatedBridgeTransport: unknown opcode"
                                    << opcode;
            m_control.remove(0, 1);
        }
    }
}

void SimulatedBridgeTransport::startStreaming()
{
    if (m_started)
        return;
    m_started = true;
    m_elapsed.start();
    m_timer.start(m_frameIntervalMs);
    qDebug(logInfo()) << "SimulatedBridgeTransport: streaming at"
                      << m_frameIntervalMs << "ms interval, MTU" << m_mtu;
}

void SimulatedBridgeTransport::stopStreaming()
{
    m_started = false;
    m_timer.stop();
}

void SimulatedBridgeTransport::rebuildFlux()
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

QByteArray SimulatedBridgeTransport::buildFrame(quint32 timestampMs)
{
    // Drift is a function of the frame timestamp (not wall time), so a
    // sequence produced by emitFrames() is fully deterministic.
    double drift = 1.0;
    if (m_scene.driftAmplitude > 0.0 && m_scene.driftPeriodMs > 0.0) {
        drift = 1.0 + m_scene.driftAmplitude
                          * std::sin(2.0 * M_PI * timestampMs / m_scene.driftPeriodMs);
    }
    QVector<double> flux(m_flux.size());
    for (int i = 0; i < m_flux.size(); ++i)
        flux[i] = m_flux[i] * drift;

    const Spectrum spectrum = m_model.measure(flux, m_grid, m_params);

    QByteArray frame;
    QDataStream out(&frame, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out << quint32(0)  // length placeholder, patched below
        << BridgeContract::magic << BridgeContract::version
        << BridgeContract::frameSpectrum
        << quint8(spectrum.counts.size()) << timestampMs;
    for (double count : spectrum.counts)
        out << float(count);
    qToLittleEndian(quint32(frame.size() - 4), frame.data());
    return frame;
}

void SimulatedBridgeTransport::deliver(QByteArray frame)
{
    if (m_faults.corruptByteProbability > 0.0) {
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        std::uniform_int_distribution<int> flip(1, 255);
        for (char& b : frame) {
            if (chance(m_faultRng) < m_faults.corruptByteProbability)
                b = char(uchar(b) ^ uchar(flip(m_faultRng)));
        }
    }

    if (m_mtu <= 0 || frame.size() <= m_mtu) {
        emit bytesReceived(frame);
    } else {
        for (int offset = 0; offset < frame.size(); offset += m_mtu)
            emit bytesReceived(frame.mid(offset, m_mtu));
    }

    ++m_framesEmitted;
    if (m_faults.dropAfterFrames >= 0 && m_framesEmitted >= m_faults.dropAfterFrames) {
        qWarning(logCritical()) << "SimulatedBridgeTransport: dropping link after"
                                << m_framesEmitted << "frame(s) (fault policy).";
        close();
    }
}

void SimulatedBridgeTransport::tick()
{
    deliver(buildFrame(quint32(m_elapsed.elapsed())));
}

void SimulatedBridgeTransport::emitFrames(int count)
{
    for (int i = 0; i < count && m_open && m_started; ++i) {
        m_syntheticClockMs += quint32(m_frameIntervalMs);
        deliver(buildFrame(m_syntheticClockMs));
    }
}
