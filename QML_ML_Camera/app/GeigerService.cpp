#include "GeigerService.h"

#include <memory>

#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QUrl>

#include "CodecDevice.h"
#include "GeigerCodec.h"
#include "MeasurementDAO.h"
#include "SensorDevice.h"
#include "SimulatedBridgeTransport.h"
#include "SimulatedGeiger.h"
#include "StorageLocations.h"
#include "loggingcategories.h"

GeigerService::GeigerService(QObject* parent)
    : QObject(parent)
{
    attachDevice(SimulatedGeiger::background(this));
    attachDevice(SimulatedGeiger::checkSource(this));
    // The full byte path with no radio: geiger frames through the bridge
    // impersonation and the GeigerCodec — Milestone 9's proof, second modality.
    attachDevice(new CodecDevice(
        QStringLiteral("Simulated bridge (loopback, geiger)"),
        std::unique_ptr<SensorTransport>(SimulatedBridgeTransport::geigerSource(20.0)),
        std::make_unique<GeigerCodec>(), this));
    qDebug(logInfo()) << "GeigerService:" << m_devices.size() << "device(s) available.";
}

QStringList GeigerService::availableDevices() const
{
    QStringList names;
    names.reserve(m_devices.size());
    for (const SensorDevice* device : m_devices)
        names.append(device->name());
    return names;
}

SensorDevice* GeigerService::currentDevice() const
{
    if (m_currentDeviceIndex < 0 || m_currentDeviceIndex >= m_devices.size())
        return nullptr;
    return m_devices[m_currentDeviceIndex];
}

bool GeigerService::isConnected() const
{
    const SensorDevice* device = currentDevice();
    return device && device->isConnected();
}

void GeigerService::setCurrentDeviceIndex(int index)
{
    if (index == m_currentDeviceIndex || index < 0 || index >= m_devices.size())
        return;
    if (isConnected())
        disconnectDevice();
    m_currentDeviceIndex = index;
    resetWindow();
    qDebug(logInfo()) << "GeigerService: device selected:" << currentDevice()->name();
    emit currentDeviceIndexChanged();
    emit connectedChanged();
}

void GeigerService::selectNextDevice()
{
    if (m_devices.size() > 1)
        setCurrentDeviceIndex((m_currentDeviceIndex + 1) % m_devices.size());
}

void GeigerService::attachDevice(SensorDevice* device)
{
    m_devices.append(device);

    connect(device, &SensorDevice::readingReady,
            this, [this, device](const GeigerReading& r) { onReading(device, r); });
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

bool GeigerService::connectDevice()
{
    SensorDevice* device = currentDevice();
    if (!device)
        return false;
    if (device->isConnected())
        return true;

    QString error;
    if (!device->connectDevice(&error)) {
        qWarning(logCritical()) << "GeigerService: connect failed:" << error;
        return false;
    }
    qDebug(logInfo()) << "GeigerService: connected to" << device->name();
    return true;
}

void GeigerService::disconnectDevice()
{
    SensorDevice* device = currentDevice();
    if (!device || !device->isConnected())
        return;
    if (m_acquiring)
        stopAcquisition();
    device->disconnectDevice();
    qDebug(logInfo()) << "GeigerService: disconnected from" << device->name();
}

void GeigerService::startAcquisition()
{
    SensorDevice* device = currentDevice();
    if (!device)
        return;
    if (!device->isConnected()) {
        qDebug(logInfo()) << "GeigerService: auto-connecting" << device->name();
        if (!connectDevice())
            return;
    }
    resetWindow();
    device->start();
    setAcquiring(true);
    qDebug(logInfo()) << "GeigerService: counting started on" << device->name();
}

void GeigerService::stopAcquisition()
{
    SensorDevice* device = currentDevice();
    if (!device || !m_acquiring)
        return;
    device->stop();
    setAcquiring(false);
    qDebug(logInfo()) << "GeigerService: counting stopped.";
}

void GeigerService::setAcquiring(bool acquiring)
{
    if (m_acquiring == acquiring)
        return;
    m_acquiring = acquiring;
    {
        QMutexLocker locker(&m_statusMutex);
        m_status.active = acquiring;
    }
    emit acquiringChanged();
}

void GeigerService::resetWindow()
{
    m_window.clear();
    m_doseSeries.clear();
    m_latestCps = 0.0;
}

double GeigerService::doseMicroSvPerHour() const
{
    return GeigerMath::doseMicroSvPerHour(m_window.cpm(), m_tubeFactor);
}

bool GeigerService::aboveThreshold() const
{
    return doseMicroSvPerHour() > m_alertThreshold;
}

void GeigerService::setTubeFactor(double factor)
{
    factor = qBound(0.0001, factor, 0.1);
    if (qFuzzyCompare(factor, m_tubeFactor))
        return;
    m_tubeFactor = factor;
    qDebug(logInfo()) << "GeigerService: tube factor" << factor << "uSv/h per CPM";
    emit tubeFactorChanged();
    emit readingUpdated();  // dose readouts change immediately
}

void GeigerService::setAlertThreshold(double threshold)
{
    threshold = qBound(0.05, threshold, 100.0);
    if (qFuzzyCompare(threshold, m_alertThreshold))
        return;
    m_alertThreshold = threshold;
    qDebug(logInfo()) << "GeigerService: alert threshold" << threshold << "uSv/h";
    emit alertThresholdChanged();
    emit readingUpdated();
}

void GeigerService::onReading(SensorDevice* device, const GeigerReading& reading)
{
    if (device != currentDevice())
        return;

    m_latestCps = reading.countsPerSecond;
    m_window.add(reading.timestampMs, reading.countsPerSecond);

    const double dose = doseMicroSvPerHour();
    m_doseSeries.append(QPointF(double(reading.timestampMs), dose));
    // Keep a little more than the window so the chart's left edge stays full.
    const double cutoff = double(reading.timestampMs) - 66000.0;
    int firstKept = 0;
    while (firstKept < m_doseSeries.size() && m_doseSeries[firstKept].x() < cutoff)
        ++firstKept;
    if (firstKept > 0)
        m_doseSeries.remove(0, firstKept);

    {
        QMutexLocker locker(&m_statusMutex);
        m_status.active = m_acquiring;
        m_status.alert = dose > m_alertThreshold;
        m_status.doseMicroSvPerHour = dose;
        m_status.countsPerMinute = m_window.cpm();
    }

    emit readingUpdated();
}

GeigerService::OverlayStatus GeigerService::overlayStatus() const
{
    QMutexLocker locker(&m_statusMutex);
    return m_status;
}

void GeigerService::setMeasurementStore(const MeasurementDAO* dao,
                                        std::function<int()> activeSessionProvider)
{
    m_measurementDao = dao;
    m_activeSessionProvider = std::move(activeSessionProvider);
}

void GeigerService::saveMeasurement(const QString& name, const QString& tags)
{
    if (!m_measurementDao || !m_activeSessionProvider) {
        emit errorOccurred(tr("No session store available."));
        return;
    }
    if (m_window.count() == 0) {
        emit errorOccurred(tr("Nothing to save yet — start counting first."));
        return;
    }

    const int sessionId = m_activeSessionProvider();
    if (sessionId < 0) {
        emit errorOccurred(tr("No session store available."));
        return;
    }

    const QJsonObject summary{
        {QStringLiteral("name"), name.isEmpty()
                                     ? QStringLiteral("Dose ")
                                           + QDateTime::currentDateTime().toString(
                                               QStringLiteral("HH:mm:ss"))
                                     : name},
        {QStringLiteral("tags"), tags},
        {QStringLiteral("avgCpm"), m_window.cpm()},
        {QStringLiteral("maxCps"), m_window.maxCps()},
        {QStringLiteral("samples"), m_window.count()},
        {QStringLiteral("tubeFactor"), m_tubeFactor},
        {QStringLiteral("windowSeconds"), 60},
    };
    m_measurementDao->addMeasurement(
        sessionId, QStringLiteral("geiger_dose"), doseMicroSvPerHour(),
        QStringLiteral("uSv/h"),
        QString::fromUtf8(QJsonDocument(summary).toJson(QJsonDocument::Compact)));
    qDebug(logInfo()) << "GeigerService: measurement saved to session" << sessionId
                      << "dose" << doseMicroSvPerHour() << "uSv/h";
    emit measurementSaved();
}

bool GeigerService::exportWindowCsv()
{
    if (m_window.count() == 0) {
        emit errorOccurred(tr("Nothing to export yet — start counting first."));
        return false;
    }

    const QString path = StorageLocations::exportsDir() + QStringLiteral("/geiger_")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))
        + QStringLiteral(".csv");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Could not write %1").arg(path));
        return false;
    }

    // Rolling values recomputed cumulatively so each row shows what the
    // instrument displayed at that moment.
    QTextStream out(&file);
    out << "timestamp_ms,cps,cpm,usv_per_h\n";
    GeigerMath::RollingWindow window(60000);
    for (const auto& entry : m_window.entries()) {
        window.add(entry.first, entry.second);
        out << entry.first << ',' << entry.second << ',' << window.cpm() << ','
            << GeigerMath::doseMicroSvPerHour(window.cpm(), m_tubeFactor) << '\n';
    }
    file.close();
    qDebug(logInfo()) << "GeigerService: exported" << path;
    QDesktopServices::openUrl(QUrl::fromLocalFile(StorageLocations::exportsDir()));
    return true;
}
