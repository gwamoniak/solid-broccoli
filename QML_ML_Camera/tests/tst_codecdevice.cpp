#include <QSignalSpy>
#include <QtTest>

#include "CodecDevice.h"
#include "ProtocolCodec.h"
#include "SensorDevice.h"
#include "SensorTransport.h"
#include "Spectrum.h"

// In-test fakes: a transport that records writes and injects bytes on
// command, and a codec speaking a one-byte "protocol" ('S' -> spectrum,
// 'G' -> geiger reading). Together they verify CodecDevice's wiring: open /
// close lifecycle, command encoding, and variant dispatch to typed signals.
namespace {

class FakeTransport : public SensorTransport
{
    Q_OBJECT
public:
    bool open(QString* errorMessage) override
    {
        if (failOpen) {
            if (errorMessage)
                *errorMessage = QStringLiteral("simulated open failure");
            return false;
        }
        opened = true;
        return true;
    }
    void close() override
    {
        opened = false;
        emit closed();
    }
    void writeBytes(const QByteArray& bytes) override { written.append(bytes); }

    void inject(const QByteArray& bytes) { emit bytesReceived(bytes); }

    bool failOpen = false;
    bool opened = false;
    QByteArray written;
};

class FakeCodec : public ProtocolCodec
{
public:
    void feed(const QByteArray& bytes) override
    {
        for (char b : bytes) {
            if (b == 'S' && m_sink) {
                Spectrum s;
                s.wavelengthsNm = {400.0, 500.0, 600.0};
                s.counts = {1.0, 2.0, 3.0};
                s.timestampMs = 42;
                m_sink(s);
            } else if (b == 'G' && m_sink) {
                m_sink(GeigerReading{7, 42.0});
            }
        }
    }
    void setSink(std::function<void(SensorReading)> sink) override { m_sink = std::move(sink); }
    QByteArray encodeStart() override { return "START"; }
    QByteArray encodeStop() override { return "STOP"; }
    QByteArray encodeParams(const AcquisitionParams& p) override
    {
        return QByteArray("P") + QByteArray::number(p.integrationTimeMs);
    }

private:
    std::function<void(SensorReading)> m_sink;
};

// Builds a device around fresh fakes and hands back the transport pointer
// (owned by the device) for injection and write assertions.
CodecDevice* makeDevice(FakeTransport*& transportOut, QObject* parent)
{
    auto transport = std::make_unique<FakeTransport>();
    transportOut = transport.get();
    return new CodecDevice(QStringLiteral("fake"), std::move(transport),
                           std::make_unique<FakeCodec>(), parent);
}

} // namespace

class TstCodecDevice : public QObject
{
    Q_OBJECT

private slots:
    void connectLifecycle()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QSignalSpy connectedSpy(device, &SensorDevice::connectedChanged);

        QString error;
        QVERIFY(device->connectDevice(&error));
        QVERIFY(device->isConnected());
        QVERIFY(transport->opened);
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(connectedSpy.takeFirst().at(0).toBool(), true);

        device->disconnectDevice();
        QVERIFY(!device->isConnected());
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(connectedSpy.takeFirst().at(0).toBool(), false);
    }

    void openFailureReportsError()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        transport->failOpen = true;
        QSignalSpy errorSpy(device, &SensorDevice::errorOccurred);

        QString error;
        QVERIFY(!device->connectDevice(&error));
        QVERIFY(!device->isConnected());
        QCOMPARE(error, QStringLiteral("simulated open failure"));
        QCOMPARE(errorSpy.count(), 1);
    }

    void commandsAreEncodedAndWritten()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QVERIFY(device->connectDevice(nullptr));

        device->start();
        device->setParams({250, 4});
        device->stop();
        QCOMPARE(transport->written, QByteArray("STARTP250STOP"));
    }

    void commandsRequireConnection()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        device->start();
        device->stop();
        QCOMPARE(transport->written, QByteArray());
    }

    void spectrumVariantDispatchesToSpectrumReady()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QVERIFY(device->connectDevice(nullptr));
        QSignalSpy spectrumSpy(device, &SensorDevice::spectrumReady);
        QSignalSpy readingSpy(device, &SensorDevice::readingReady);

        transport->inject("SS");
        QCOMPARE(spectrumSpy.count(), 2);
        QCOMPARE(readingSpy.count(), 0);

        const auto s = spectrumSpy.takeFirst().at(0).value<Spectrum>();
        QVERIFY(s.isValid());
        QCOMPARE(s.counts.size(), 3);
        QCOMPARE(s.timestampMs, qint64(42));
    }

    void geigerVariantDispatchesToReadingReady()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QVERIFY(device->connectDevice(nullptr));
        QSignalSpy spectrumSpy(device, &SensorDevice::spectrumReady);
        QSignalSpy readingSpy(device, &SensorDevice::readingReady);

        transport->inject("G");
        QCOMPARE(readingSpy.count(), 1);
        QCOMPARE(spectrumSpy.count(), 0);

        const auto r = readingSpy.takeFirst().at(0).value<GeigerReading>();
        QCOMPARE(r.countsPerSecond, 42.0);
        QCOMPARE(r.timestampMs, qint64(7));
    }

    void transportErrorForwardsToErrorOccurred()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QVERIFY(device->connectDevice(nullptr));
        QSignalSpy errorSpy(device, &SensorDevice::errorOccurred);

        emit transport->transportError(QStringLiteral("link lost"));
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).toString(), QStringLiteral("link lost"));
    }

    void transportClosedDropsConnection()
    {
        FakeTransport* transport = nullptr;
        CodecDevice* device = makeDevice(transport, this);
        QVERIFY(device->connectDevice(nullptr));

        transport->close();
        QVERIFY(!device->isConnected());
    }
};

QTEST_GUILESS_MAIN(TstCodecDevice)
#include "tst_codecdevice.moc"
