#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "As7265xCodec.h"
#include "BridgeContract.h"
#include "CodecDevice.h"
#include "ReplayTransport.h"
#include "ScriptedTransport.h"
#include "SimulatedBridgeTransport.h"
#include "SpectrumSynthesizer.h"
#include "TraceRecorder.h"

// The sans-IO payoff: the codec is beaten up with hand-built byte arrays
// (whole, split, concatenated, corrupted frames), then the same codec is
// driven end-to-end through CodecDevice with a scripted transport and with
// the simulated bridge — the byte-for-byte impersonation of the firmware.
class TstAs7265xCodec : public QObject
{
    Q_OBJECT

private:
    // Hand-built frame, written independently of both the codec and the
    // simulated bridge so it can catch either side deviating.
    static QByteArray makeFrame(const QVector<float>& counts, quint32 timestampMs)
    {
        QByteArray frame;
        QDataStream out(&frame, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out << quint32(4 + 1 + 4 + counts.size() * 4)  // header + payload
            << quint16(0x5BEC) << quint8(0x01) << quint8(0x01)
            << quint8(counts.size()) << timestampMs;
        for (float c : counts)
            out << c;
        return frame;
    }

    static QVector<float> rampCounts()
    {
        QVector<float> counts(18);
        for (int i = 0; i < counts.size(); ++i)
            counts[i] = 100.0f + 10.5f * float(i);
        return counts;
    }

    struct Collected {
        QVector<Spectrum> spectra;
    };

    static std::function<void(SensorReading)> collector(Collected* into)
    {
        return [into](SensorReading reading) {
            if (std::holds_alternative<Spectrum>(reading))
                into->spectra.append(std::get<Spectrum>(reading));
        };
    }

private slots:
    void wavelengthTableMatchesSimulatorGrid()
    {
        // Two deliberate copies of the chip's channel table; they must agree.
        QCOMPARE(As7265xCodec::wavelengthTable(), SpectrumSynthesizer::as7265xGrid());
    }

    void wholeFrameParses()
    {
        As7265xCodec codec;
        Collected got;
        codec.setSink(collector(&got));

        const QVector<float> counts = rampCounts();
        codec.feed(makeFrame(counts, 12345));

        QCOMPARE(got.spectra.size(), 1);
        const Spectrum& s = got.spectra.first();
        QCOMPARE(s.timestampMs, qint64(12345));
        QCOMPARE(s.wavelengthsNm, As7265xCodec::wavelengthTable());
        QCOMPARE(s.counts.size(), 18);
        for (int i = 0; i < 18; ++i)
            QCOMPARE(s.counts[i], double(counts[i]));  // f32 -> f64 is exact
        QVERIFY(s.isValid());
    }

    void frameSplitAcrossThreeChunks()
    {
        As7265xCodec codec;
        Collected got;
        codec.setSink(collector(&got));

        const QByteArray frame = makeFrame(rampCounts(), 7);
        const int third = frame.size() / 3;
        codec.feed(frame.left(third));
        QCOMPARE(got.spectra.size(), 0);
        codec.feed(frame.mid(third, third));
        QCOMPARE(got.spectra.size(), 0);
        codec.feed(frame.mid(2 * third));
        QCOMPARE(got.spectra.size(), 1);
    }

    void twoFramesInOneChunk()
    {
        As7265xCodec codec;
        Collected got;
        codec.setSink(collector(&got));

        codec.feed(makeFrame(rampCounts(), 1) + makeFrame(rampCounts(), 2));
        QCOMPARE(got.spectra.size(), 2);
        QCOMPARE(got.spectra[0].timestampMs, qint64(1));
        QCOMPARE(got.spectra[1].timestampMs, qint64(2));
    }

    void corruptMagicIsSkippedAndStreamResynchronizes()
    {
        As7265xCodec codec;
        Collected got;
        codec.setSink(collector(&got));

        QByteArray bad = makeFrame(rampCounts(), 1);
        bad[4] = char(0xFF);  // wreck the magic
        codec.feed(bad + makeFrame(rampCounts(), 2));

        QCOMPARE(got.spectra.size(), 1);
        QCOMPARE(got.spectra.first().timestampMs, qint64(2));
    }

    void geigerFramesAreConsumedSilently()
    {
        // A well-formed type-0x02 frame (Milestone 10's modality) must not
        // derail the spectrum stream and must not emit a spectrum.
        QByteArray geiger;
        QDataStream out(&geiger, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out << quint32(4 + 8) << quint16(0x5BEC) << quint8(0x01) << quint8(0x02)
            << quint32(99) << 0.3f;

        As7265xCodec codec;
        Collected got;
        codec.setSink(collector(&got));
        codec.feed(geiger + makeFrame(rampCounts(), 5));

        QCOMPARE(got.spectra.size(), 1);
        QCOMPARE(got.spectra.first().timestampMs, qint64(5));
    }

    void encodedCommandsMatchTheContract()
    {
        As7265xCodec codec;
        QCOMPARE(codec.encodeStart(), QByteArray::fromHex("01"));
        QCOMPARE(codec.encodeStop(), QByteArray::fromHex("02"));
        // 0x03 | u16 LE 250 | u8 4
        QCOMPARE(codec.encodeParams({250, 4}), QByteArray::fromHex("03fa0004"));
    }

    void codecDeviceRoundTripWithScriptedTransport()
    {
        auto transport = std::make_unique<ScriptedTransport>();
        ScriptedTransport* scripted = transport.get();
        CodecDevice device(QStringLiteral("scripted"), std::move(transport),
                           std::make_unique<As7265xCodec>());

        QVERIFY(device.connectDevice(nullptr));
        device.setParams({250, 4});
        device.start();
        QCOMPARE(scripted->writtenBytes(), QByteArray::fromHex("03fa000401"));

        QSignalSpy spy(&device, &SensorDevice::spectrumReady);
        scripted->setChunkSize(20);  // BLE-notification-sized chunks
        scripted->enqueueIncoming(makeFrame(rampCounts(), 42));
        QCOMPARE(spy.count(), 1);
        const auto spectrum = spy.first().first().value<Spectrum>();
        QCOMPARE(spectrum.timestampMs, qint64(42));
        // The codec stamps the params the service last encoded.
        QCOMPARE(spectrum.params.integrationTimeMs, 250);
        QCOMPARE(spectrum.params.averaging, 4);
    }

    void simulatedBridgeStreamsSpectraThroughTheFullBytePath()
    {
        auto transport = std::unique_ptr<SimulatedBridgeTransport>(
            SimulatedBridgeTransport::mercuryLamp());
        SimulatedBridgeTransport* bridge = transport.get();
        CodecDevice device(QStringLiteral("loopback"), std::move(transport),
                           std::make_unique<As7265xCodec>());

        QVERIFY(device.connectDevice(nullptr));
        device.setParams({100, 1});
        device.start();
        QVERIFY(bridge->isStarted());
        QCOMPARE(bridge->params().integrationTimeMs, 100);

        QSignalSpy spy(&device, &SensorDevice::spectrumReady);
        bridge->emitFrames(10);
        QCOMPARE(spy.count(), 10);
        const auto spectrum = spy.last().first().value<Spectrum>();
        QVERIFY(spectrum.isValid());
        QCOMPARE(spectrum.counts.size(), 18);
        QCOMPARE(spectrum.wavelengthsNm, SpectrumSynthesizer::as7265xGrid());

        device.stop();
        QVERIFY(!bridge->isStarted());
    }

    void corruptedBridgeStreamIsDeterministicAndResynchronizes()
    {
        // 200 frames at MTU 20 with per-byte corruption 0.001: a few frames
        // die (header hits) or arrive distorted (payload hits), but the
        // stream must keep resynchronizing, and the whole run must be
        // bit-reproducible under the fixed seed.
        auto runOnce = [this]() {
            auto transport = std::unique_ptr<SimulatedBridgeTransport>(
                SimulatedBridgeTransport::mercuryLamp());
            SimulatedBridgeTransport* bridge = transport.get();
            FaultPolicy faults;
            faults.corruptByteProbability = 0.001;
            faults.seed = 7;
            bridge->setFaultPolicy(faults);
            bridge->setMtu(20);

            CodecDevice device(QStringLiteral("noisy"), std::move(transport),
                               std::make_unique<As7265xCodec>());
            QSignalSpy spy(&device, &SensorDevice::spectrumReady);
            device.connectDevice(nullptr);
            device.start();
            bridge->emitFrames(200);

            QVector<Spectrum> spectra;
            for (int i = 0; i < spy.count(); ++i)
                spectra.append(spy.at(i).first().value<Spectrum>());
            return spectra;
        };

        const QVector<Spectrum> first = runOnce();
        const QVector<Spectrum> second = runOnce();

        QVERIFY(first.size() >= 190);   // ~0.2 corrupt bytes per 85-byte frame
        QVERIFY(first.size() <= 200);
        QVERIFY(first.last().isValid());  // still alive at the end = resynced

        QCOMPARE(second.size(), first.size());  // seeded => exactly repeatable
        for (int i = 0; i < first.size(); ++i)
            QCOMPARE(second[i].counts, first[i].counts);
    }

    void faultPolicyDropClosesTheDevice()
    {
        auto transport = std::unique_ptr<SimulatedBridgeTransport>(
            SimulatedBridgeTransport::mercuryLamp());
        SimulatedBridgeTransport* bridge = transport.get();
        FaultPolicy faults;
        faults.dropAfterFrames = 3;
        bridge->setFaultPolicy(faults);

        CodecDevice device(QStringLiteral("dropping"), std::move(transport),
                           std::make_unique<As7265xCodec>());
        QSignalSpy spectra(&device, &SensorDevice::spectrumReady);
        device.connectDevice(nullptr);
        device.start();
        bridge->emitFrames(10);

        QCOMPARE(spectra.count(), 3);       // link died mid-stream
        QVERIFY(!device.isConnected());     // closed() propagated
    }

    void traceRecordedSessionReplaysIdentically()
    {
        QTemporaryDir dir;
        const QString trace = dir.filePath("bridge.trace");

        QVector<Spectrum> recorded;
        {
            auto* bridge = SimulatedBridgeTransport::mercuryLamp();
            CodecDevice device(QStringLiteral("recorded"),
                               std::make_unique<TraceRecorder>(bridge, trace),
                               std::make_unique<As7265xCodec>());
            bridge->setParent(&device);  // recorder does not own the inner
            QSignalSpy spy(&device, &SensorDevice::spectrumReady);
            device.connectDevice(nullptr);
            device.start();
            bridge->emitFrames(5);
            device.disconnectDevice();
            for (int i = 0; i < spy.count(); ++i)
                recorded.append(spy.at(i).first().value<Spectrum>());
        }
        QCOMPARE(recorded.size(), 5);

        CodecDevice replayed(QStringLiteral("replayed"),
                             std::unique_ptr<SensorTransport>(ReplayTransport::instant(trace)),
                             std::make_unique<As7265xCodec>());
        QSignalSpy spy(&replayed, &SensorDevice::spectrumReady);
        QVERIFY(replayed.connectDevice(nullptr));  // instant replay fires here

        QCOMPARE(spy.count(), recorded.size());
        for (int i = 0; i < spy.count(); ++i) {
            const auto spectrum = spy.at(i).first().value<Spectrum>();
            QCOMPARE(spectrum.counts, recorded[i].counts);
            QCOMPARE(spectrum.timestampMs, recorded[i].timestampMs);
        }
    }
};

QTEST_GUILESS_MAIN(TstAs7265xCodec)
#include "tst_as7265xcodec.moc"
