#include "SpectrometerService.h"

#include <algorithm>
#include <memory>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QDateTime>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QUrl>

#include "As7265xCodec.h"
#include "BleTransport.h"
#include "BridgeContract.h"
#include "CodecDevice.h"
#include "PeakListModel.h"
#include "SimulatedBridgeTransport.h"
#include "SensorDevice.h"
#include "SessionModel.h"
#include "SessionSpectrumModel.h"
#include "SimulatedSpectrometer.h"
#include "SpectroAnalysis.h"
#include "SpectrumDAO.h"
#include "SpectrumExporter.h"
#include "StorageLocations.h"
#include "loggingcategories.h"

SpectrometerService::SpectrometerService(QObject* parent)
    : QObject(parent)
    , m_peakModel(new PeakListModel(this))
{
    // Construction creates the built-ins only; the BLE scan (radio power,
    // 5 s, permission prompt on first use) runs solely on an explicit
    // refreshDevices() — never in headless tests, which construct this
    // service directly.
    ensureBuiltInDevices();
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
    recomputeDerived();
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
    } else {
        m_display = m_liveSpectrum;
        m_display.counts = (m_mode == 1)
            ? SpectroAnalysis::transmittance(m_liveSpectrum, m_dark, m_reference)
            : SpectroAnalysis::absorbance(m_liveSpectrum, m_dark, m_reference);
    }
    if (m_smoothingWindow > 0)
        m_display.counts = SpectroAnalysis::savitzkyGolay(m_display.counts,
                                                          m_smoothingWindow, 2);
}

void SpectrometerService::setSmoothingWindow(int window)
{
    // Normalize: 0 disables, anything else becomes an odd window in [5, 25].
    if (window > 0) {
        window = std::clamp(window, 5, 25);
        if (window % 2 == 0)
            ++window;
    } else {
        window = 0;
    }
    if (window == m_smoothingWindow)
        return;
    m_smoothingWindow = window;
    qDebug(logInfo()) << "SpectrometerService: smoothing window" << window;
    rebuildDisplay();
    recomputeDerived();
    emit smoothingWindowChanged();
    emit spectrumUpdated();
}

QObject* SpectrometerService::peakModel() const
{
    return m_peakModel;
}

bool SpectrometerService::hasIntegrationRegion() const
{
    return !qIsNaN(m_integrationFromNm) && !qIsNaN(m_integrationToNm);
}

void SpectrometerService::setIntegrationRegion(double fromNm, double toNm)
{
    if (fromNm > toNm)
        std::swap(fromNm, toNm);
    m_integrationFromNm = fromNm;
    m_integrationToNm = toNm;
    qDebug(logInfo()) << "SpectrometerService: integration region"
                      << fromNm << "-" << toNm << "nm";
    recomputeDerived();
    emit integrationRegionChanged();
    emit spectrumUpdated();
}

void SpectrometerService::clearIntegrationRegion()
{
    m_integrationFromNm = qQNaN();
    m_integrationToNm = qQNaN();
    recomputeDerived();
    emit integrationRegionChanged();
    emit spectrumUpdated();
}

void SpectrometerService::setSessionStore(SessionModel* sessions,
                                          SessionSpectrumModel* spectra)
{
    m_sessionModel = sessions;
    m_spectrumModel = spectra;
}

void SpectrometerService::saveCapture(const QString& name, const QString& tags)
{
    if (!m_sessionModel || !m_spectrumModel) {
        emit errorOccurred(tr("No session store available."));
        return;
    }
    if (!m_liveSpectrum.isValid()) {
        emit errorOccurred(tr("Nothing to save yet — start acquisition first."));
        return;
    }

    if (m_activeSessionId < 0) {
        const QString sessionName = QStringLiteral("Session ")
            + QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        m_activeSessionId = m_sessionModel->addSessionFromName(sessionName);
        m_darkStored = false;
        m_referenceStored = false;
        qDebug(logInfo()) << "SpectrometerService: session created:" << sessionName;
    }

    // The calibration pair belongs with the data it corrects; store each
    // once per session.
    if (hasDark() && !m_darkStored) {
        m_spectrumModel->addSpectrum(m_activeSessionId, m_dark,
                                     QStringLiteral("Dark"), QString());
        m_darkStored = true;
    }
    if (hasReference() && !m_referenceStored) {
        m_spectrumModel->addSpectrum(m_activeSessionId, m_reference,
                                     QStringLiteral("Reference"), QString());
        m_referenceStored = true;
    }

    Spectrum sample = m_hold ? m_held : m_liveSpectrum;
    sample.kind = Spectrum::Kind::Sample;
    const QString captureName = name.isEmpty()
        ? QStringLiteral("Sample ")
              + QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))
        : name;
    m_spectrumModel->addSpectrum(m_activeSessionId, sample, captureName, tags);
    m_sessionModel->refresh();
    qDebug(logInfo()) << "SpectrometerService: capture saved:" << captureName
                      << "tags:" << tags;
    emit captureSaved(captureName);
}

QVariantList SpectrometerService::overlayIds() const
{
    QVariantList ids;
    for (const auto& overlay : m_overlays)
        ids.append(overlay.first);
    return ids;
}

QVector<Spectrum> SpectrometerService::overlaySpectra() const
{
    QVector<Spectrum> spectra;
    spectra.reserve(m_overlays.size());
    for (const auto& overlay : m_overlays)
        spectra.append(overlay.second);
    return spectra;
}

void SpectrometerService::toggleOverlay(int spectrumId)
{
    for (int i = 0; i < m_overlays.size(); ++i) {
        if (m_overlays[i].first == spectrumId) {
            m_overlays.removeAt(i);
            emit overlaysChanged();
            return;
        }
    }
    if (!m_spectrumModel)
        return;
    const SpectrumEntry entry = m_spectrumModel->entryById(spectrumId);
    if (!entry.isValid()) {
        emit errorOccurred(tr("Capture could not be loaded."));
        return;
    }
    if (m_overlays.size() >= 3)
        m_overlays.removeFirst();
    m_overlays.append({spectrumId, entry.spectrum});
    qDebug(logInfo()) << "SpectrometerService: overlay added:" << entry.name;
    emit overlaysChanged();
}

void SpectrometerService::clearOverlays()
{
    if (m_overlays.isEmpty())
        return;
    m_overlays.clear();
    emit overlaysChanged();
}

bool SpectrometerService::exportCapture(int spectrumId)
{
    if (!m_spectrumModel)
        return false;
    const SpectrumEntry entry = m_spectrumModel->entryById(spectrumId);
    if (!entry.isValid()) {
        emit errorOccurred(tr("Capture could not be loaded."));
        return false;
    }

    SpectrumExportMetadata meta;
    meta.name = entry.name;
    meta.kind = SpectrumDAO::kindToString(entry.spectrum.kind);
    meta.createdUtc = entry.createdUtc;
    meta.tags = entry.tags;
    meta.integrationMs = entry.spectrum.params.integrationTimeMs;
    meta.averaging = entry.spectrum.params.averaging;

    QString base = entry.name;
    base.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")),
                 QStringLiteral("_"));
    if (base.isEmpty())
        base = QStringLiteral("spectrum_%1").arg(spectrumId);

    const QString dir = StorageLocations::exportsDir();
    QString error;
    if (!SpectrumExporter::exportCsv(entry.spectrum, meta, dir + "/" + base + ".csv", &error)
        || !SpectrumExporter::exportJson(entry.spectrum, meta, dir + "/" + base + ".json", &error)) {
        qWarning(logCritical()) << "SpectrometerService: export failed:" << error;
        emit errorOccurred(error);
        return false;
    }
    qDebug(logInfo()) << "SpectrometerService: exported" << base << "to" << dir;
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    return true;
}

void SpectrometerService::recomputeDerived()
{
    const Spectrum& display = displaySpectrum();
    if (!display.isValid()) {
        m_peakModel->setPeaks({});
        m_integralValue = qQNaN();
        m_peakWavelengthNm = 0.0;
        m_peakValue = 0.0;
        return;
    }

    const auto [minIt, maxIt] = std::minmax_element(display.counts.cbegin(),
                                                    display.counts.cend());
    // Auto prominence scales with the display range so it works for raw
    // counts and for absorbance alike; an explicit peakProminence overrides.
    const double prominence = m_peakProminence > 0.0
                                  ? m_peakProminence
                                  : std::max(0.05 * (*maxIt - *minIt), 1e-6);
    const auto peaks = SpectroAnalysis::findPeaks(display, prominence, 5.0);
    m_peakModel->setPeaks(peaks);
    if (!peaks.isEmpty()) {
        m_peakWavelengthNm = peaks.first().wavelengthNm;
        m_peakValue = peaks.first().value;
    } else {
        m_peakWavelengthNm = 0.0;
        m_peakValue = 0.0;
    }

    m_integralValue = hasIntegrationRegion()
                          ? SpectroAnalysis::integrate(display, m_integrationFromNm,
                                                       m_integrationToNm)
                          : qQNaN();
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
    ensureBuiltInDevices();
    startBleScan();
}

void SpectrometerService::ensureBuiltInDevices()
{
    if (!m_devices.isEmpty())
        return;
    attachDevice(SimulatedSpectrometer::mercuryLamp(this));
    attachDevice(SimulatedSpectrometer::dyeSample(this));
    // The full byte path with no radio: bridge impersonation -> chunked
    // "notifications" -> codec -> CodecDevice.
    attachDevice(new CodecDevice(
        QStringLiteral("Simulated AS7265x bridge (loopback)"),
        std::unique_ptr<SensorTransport>(SimulatedBridgeTransport::mercuryLamp()),
        std::make_unique<As7265xCodec>(), this));
    emit availableDevicesChanged();
}

void SpectrometerService::startBleScan()
{
    if (!m_discoveryAgent) {
        m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &SpectrometerService::onBleDeviceDiscovered);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, [this]() {
                    qDebug(logInfo()) << "SpectrometerService: BLE scan finished,"
                                      << m_bleDeviceKeys.size() << "bridge(s) known.";
                });
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                this, [this](QBluetoothDeviceDiscoveryAgent::Error) {
                    qWarning(logCritical()) << "SpectrometerService: BLE scan error:"
                                            << m_discoveryAgent->errorString();
                    emit errorOccurred(m_discoveryAgent->errorString());
                });
    }
    if (m_discoveryAgent->isActive())
        return;
    qDebug(logInfo()) << "SpectrometerService: scanning for BLE bridges (5 s).";
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void SpectrometerService::onBleDeviceDiscovered(const QBluetoothDeviceInfo& info)
{
    if (!info.serviceUuids().contains(QBluetoothUuid(BridgeContract::serviceUuid())))
        return;

    // macOS hides the MAC address; the platform device UUID identifies then.
    const QString key = info.address().isNull() ? info.deviceUuid().toString()
                                                : info.address().toString();
    if (m_bleDeviceKeys.contains(key))
        return;
    m_bleDeviceKeys.append(key);

    const QString label = (info.name().isEmpty()
                               ? QStringLiteral("Spectral bridge")
                               : info.name())
                          + QStringLiteral(" (BLE)");
    attachDevice(new CodecDevice(label, std::make_unique<BleTransport>(info),
                                 std::make_unique<As7265xCodec>(), this));
    qDebug(logInfo()) << "SpectrometerService: BLE bridge found:" << label;
    emit availableDevicesChanged();
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

    recomputeDerived();
    emit spectrumUpdated();
}

Spectrum SpectrometerService::latestSpectrumSnapshot() const
{
    QMutexLocker locker(&m_spectrumMutex);
    return m_liveSpectrum;
}
