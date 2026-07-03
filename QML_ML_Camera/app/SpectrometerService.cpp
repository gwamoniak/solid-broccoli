#include "SpectrometerService.h"

#include <algorithm>

#include "SensorDevice.h"
#include "SimulatedSpectrometer.h"
#include "SpectroAnalysis.h"
#include "loggingcategories.h"

SpectrometerService::SpectrometerService(QObject* parent)
    : QObject(parent)
{
    refreshDevices();
    qDebug(logInfo()) << "SpectrometerService:" << m_devices.size()
                      << "device(s) available.";
}

QStringList SpectrometerService::availableDevices() const
{
    QStringList names;
    names.reserve(m_devices.size());
    for (const SensorDevice* device : m_devices)
        names.append(device->name());
    return names;
}

SensorDevice* SpectrometerService::currentDevice() const
{
    if (m_currentDeviceIndex < 0 || m_currentDeviceIndex >= m_devices.size())
        return nullptr;
    return m_devices[m_currentDeviceIndex];
}

bool SpectrometerService::isConnected() const
{
    const SensorDevice* device = currentDevice();
    return device && device->isConnected();
}

void SpectrometerService::setCurrentDeviceIndex(int index)
{
    if (index == m_currentDeviceIndex || index < 0 || index >= m_devices.size())
        return;

    if (isConnected())
        disconnectDevice();

    m_currentDeviceIndex = index;
    // Dark/reference belong to the instrument they were captured on.
    resetCalibration();
    qDebug(logInfo()) << "SpectrometerService: device selected:"
                      << currentDevice()->name();
    emit currentDeviceIndexChanged();
    emit connectedChanged();
}

void SpectrometerService::resetCalibration()
{
    m_dark = Spectrum();
    m_reference = Spectrum();
    m_pending = PendingCapture::None;
    if (m_mode != 0) {
        m_mode = 0;
        emit modeChanged();
    }
    if (m_hold) {
        m_hold = false;
        emit holdChanged();
    }
    emit calibrationChanged();
}

void SpectrometerService::setMode(int mode)
{
    mode = std::clamp(mode, 0, 2);
    if (mode == m_mode)
        return;
    if (mode != 0 && (!hasDark() || !hasReference())) {
        emit errorOccurred(tr("Capture Dark and Ref before switching modes."));
        return;
    }
    m_mode = mode;
    qDebug(logInfo()) << "SpectrometerService: display mode" << mode;
    rebuildDisplay();
    emit modeChanged();
    emit spectrumUpdated();
}

void SpectrometerService::setHold(bool hold)
{
    if (hold == m_hold)
        return;
    if (hold)
        m_held = m_display;
    m_hold = hold;
    qDebug(logInfo()) << "SpectrometerService: hold" << hold;
    emit holdChanged();
    emit spectrumUpdated();
}

void SpectrometerService::captureDark()
{
    if (!m_acquiring) {
        emit errorOccurred(tr("Start acquisition before capturing Dark."));
        return;
    }
    // The simulator can physically "block the light path"; a real probe
    // relies on the operator having done so, and we snapshot immediately.
    if (auto* sim = qobject_cast<SimulatedSpectrometer*>(currentDevice())) {
        sim->setLampEnabled(false);
        m_pending = PendingCapture::Dark;
        qDebug(logInfo()) << "SpectrometerService: dark capture pending (lamp off).";
        return;
    }
    m_dark = m_liveSpectrum;
    m_dark.kind = Spectrum::Kind::Dark;
    qDebug(logInfo()) << "SpectrometerService: dark captured.";
    emit calibrationChanged();
}

void SpectrometerService::captureReference()
{
    if (!m_acquiring) {
        emit errorOccurred(tr("Start acquisition before capturing Ref."));
        return;
    }
    // A simulated absorbance scene can swap in the blank (concentration 0)
    // for one frame; emission scenes and hardware snapshot the live frame.
    if (auto* sim = qobject_cast<SimulatedSpectrometer*>(currentDevice());
        sim && sim->scene().absorberPeakA > 0.0) {
        m_restoreConcentration = sim->scene().concentration;
        sim->setSceneConcentration(0.0);
        m_pending = PendingCapture::Reference;
        qDebug(logInfo()) << "SpectrometerService: reference capture pending (blank in).";
        return;
    }
    m_reference = m_liveSpectrum;
    m_reference.kind = Spectrum::Kind::Reference;
    qDebug(logInfo()) << "SpectrometerService: reference captured.";
    emit calibrationChanged();
}

void SpectrometerService::rebuildDisplay()
{
    if (m_mode == 0 || !m_liveSpectrum.isValid()
        || m_dark.counts.size() != m_liveSpectrum.counts.size()
        || m_reference.counts.size() != m_liveSpectrum.counts.size()) {
        m_display = m_liveSpectrum;
        return;
    }
    m_display = m_liveSpectrum;
    m_display.counts = (m_mode == 1)
        ? SpectroAnalysis::transmittance(m_liveSpectrum, m_dark, m_reference)
        : SpectroAnalysis::absorbance(m_liveSpectrum, m_dark, m_reference);
}

void SpectrometerService::setIntegrationTimeMs(int ms)
{
    ms = std::clamp(ms, 5, 1000);
    if (ms == m_integrationTimeMs)
        return;
    m_integrationTimeMs = ms;
    qDebug(logInfo()) << "SpectrometerService: integration time" << ms << "ms";
    if (SensorDevice* device = currentDevice(); device && device->isConnected())
        device->setParams(acquisitionParams());
    emit integrationTimeMsChanged();
}

void SpectrometerService::setAveraging(int count)
{
    count = std::clamp(count, 1, 8);
    if (count == m_averaging)
        return;
    m_averaging = count;
    qDebug(logInfo()) << "SpectrometerService: averaging" << count;
    if (SensorDevice* device = currentDevice(); device && device->isConnected())
        device->setParams(acquisitionParams());
    emit averagingChanged();
}

AcquisitionParams SpectrometerService::acquisitionParams() const
{
    return {m_integrationTimeMs, m_averaging};
}

void SpectrometerService::refreshDevices()
{
    // Built-in simulated instruments, created once. Milestone 9 appends BLE
    // scan results after these entries.
    if (m_devices.isEmpty()) {
        attachDevice(SimulatedSpectrometer::mercuryLamp(this));
        attachDevice(SimulatedSpectrometer::dyeSample(this));
        emit availableDevicesChanged();
    }
}

void SpectrometerService::selectNextDevice()
{
    if (m_devices.size() > 1)
        setCurrentDeviceIndex((m_currentDeviceIndex + 1) % m_devices.size());
}

void SpectrometerService::attachDevice(SensorDevice* device)
{
    m_devices.append(device);

    connect(device, &SensorDevice::spectrumReady,
            this, [this, device](const Spectrum& s) { onSpectrum(device, s); });
    connect(device, &SensorDevice::errorOccurred,
            this, [this, device](const QString& message) {
                if (device == currentDevice())
                    emit errorOccurred(message);
            });
    connect(device, &SensorDevice::connectedChanged,
            this, [this, device](bool connected) {
                if (device != currentDevice())
                    return;
                if (!connected)
                    setAcquiring(false);
                emit connectedChanged();
            });
}

bool SpectrometerService::connectDevice()
{
    SensorDevice* device = currentDevice();
    if (!device)
        return false;
    if (device->isConnected())
        return true;

    QString error;
    if (!device->connectDevice(&error)) {
        qWarning(logCritical()) << "SpectrometerService: connect failed:" << error;
        return false;
    }
    device->setParams(acquisitionParams());
    qDebug(logInfo()) << "SpectrometerService: connected to" << device->name();
    return true;
}

void SpectrometerService::disconnectDevice()
{
    SensorDevice* device = currentDevice();
    if (!device || !device->isConnected())
        return;
    if (m_acquiring)
        stopAcquisition();
    device->disconnectDevice();
    qDebug(logInfo()) << "SpectrometerService: disconnected from" << device->name();
}

void SpectrometerService::startAcquisition()
{
    SensorDevice* device = currentDevice();
    if (!device)
        return;

    // Field convenience: Start on a disconnected instrument connects first.
    if (!device->isConnected()) {
        qDebug(logInfo()) << "SpectrometerService: auto-connecting" << device->name();
        if (!connectDevice())
            return;
    }

    device->setParams(acquisitionParams());
    m_framesPerSecond = 0.0;
    m_frameClock.invalidate();
    device->start();
    setAcquiring(true);
    qDebug(logInfo()) << "SpectrometerService: acquisition started on" << device->name();
}

void SpectrometerService::stopAcquisition()
{
    SensorDevice* device = currentDevice();
    if (!device || !m_acquiring)
        return;
    device->stop();
    setAcquiring(false);
    qDebug(logInfo()) << "SpectrometerService: acquisition stopped.";
}

void SpectrometerService::setAcquiring(bool acquiring)
{
    if (m_acquiring == acquiring)
        return;
    m_acquiring = acquiring;
    emit acquiringChanged();
}

void SpectrometerService::onSpectrum(SensorDevice* device, const Spectrum& spectrum)
{
    if (device != currentDevice() || !spectrum.isValid())
        return;

    {
        QMutexLocker locker(&m_spectrumMutex);
        m_liveSpectrum = spectrum;
    }

    // Resolve a pending simulated dark/reference capture with this frame,
    // then restore the scene.
    if (m_pending != PendingCapture::None) {
        auto* sim = qobject_cast<SimulatedSpectrometer*>(device);
        if (m_pending == PendingCapture::Dark) {
            m_dark = spectrum;
            m_dark.kind = Spectrum::Kind::Dark;
            if (sim)
                sim->setLampEnabled(true);
            qDebug(logInfo()) << "SpectrometerService: dark captured.";
        } else {
            m_reference = spectrum;
            m_reference.kind = Spectrum::Kind::Reference;
            if (sim)
                sim->setSceneConcentration(m_restoreConcentration);
            qDebug(logInfo()) << "SpectrometerService: reference captured.";
        }
        m_pending = PendingCapture::None;
        emit calibrationChanged();
    }

    rebuildDisplay();

    // Hold freezes the display and the readouts without stopping the
    // stream (the mutex snapshot above keeps feeding the video overlay).
    if (m_hold)
        return;

    if (!m_frameClock.isValid()) {
        m_frameClock.start();
    } else {
        const qint64 dtMs = m_frameClock.restart();
        if (dtMs > 0) {
            const double instantaneous = 1000.0 / dtMs;
            m_framesPerSecond = m_framesPerSecond <= 0.0
                                    ? instantaneous
                                    : 0.8 * m_framesPerSecond + 0.2 * instantaneous;
        }
    }

    // Headline peak for the readout strip: prominence threshold scales with
    // the signal so the readout tracks the dominant line at any exposure.
    const double maxCount = *std::max_element(spectrum.counts.cbegin(),
                                              spectrum.counts.cend());
    const auto peaks = SpectroAnalysis::findPeaks(
        spectrum, std::max(50.0, 0.05 * maxCount), 5.0);
    if (!peaks.isEmpty()) {
        m_peakWavelengthNm = peaks.first().wavelengthNm;
        m_peakValue = peaks.first().value;
    } else {
        m_peakWavelengthNm = 0.0;
        m_peakValue = 0.0;
    }

    emit spectrumUpdated();
}

Spectrum SpectrometerService::latestSpectrumSnapshot() const
{
    QMutexLocker locker(&m_spectrumMutex);
    return m_liveSpectrum;
}
