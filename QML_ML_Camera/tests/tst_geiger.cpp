#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QtTest>

#include "As7265xCodec.h"
#include "CodecDevice.h"
#include "DatabaseManager.h"
#include "GeigerCodec.h"
#include "GeigerMath.h"
#include "GeigerService.h"
#include "ScriptedTransport.h"
#include "SimulatedBridgeTransport.h"
#include "SimulatedGeiger.h"

// Milestone 10 ground truth: radioactive decay is a Poisson process, so the
// simulator's recovered mean must match the scene rate and the count
// variance must equal the mean — the defining Poisson property. Derived
// quantities (rolling CPM, dose) check against hand-computed sequences, and
// the codec path proves the bridge contract carries the second modality.
class TstGeiger : public QObject
{
    Q_OBJECT

private:
    static QByteArray makeGeigerFrame(quint32 timestampMs, float cps)
    {
        QByteArray frame;
        QDataStream out(&frame, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out << quint32(4 + 8) << quint16(0x5BEC) << quint8(0x01) << quint8(0x02)
            << timestampMs << cps;
        return frame;
    }

    static QByteArray makeSpectrumFrame(quint32 timestampMs)
    {
        QByteArray frame;
        QDataStream out(&frame, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out << quint32(4 + 1 + 4 + 18 * 4) << quint16(0x5BEC) << quint8(0x01)
            << quint8(0x01) << quint8(18) << timestampMs;
        for (int i = 0; i < 18; ++i)
            out << float(100 + i);
        return frame;
    }

private slots:
    void poissonStatisticsMatchTheScene()
    {
        GeigerScene scene;
        scene.meanCps = 50.0;
        scene.driftAmplitude = 0.0;  // statistics only, no source movement
        scene.seed = 7;
        SimulatedGeiger tube(QStringLiteral("test tube"), scene);

        QVector<double> counts;  // events per 500 ms tick
        connect(&tube, &SensorDevice::readingReady, this,
                [&counts](const GeigerReading& r) {
                    counts.append(r.countsPerSecond * SimulatedGeiger::tickMs / 1000.0);
                });
        QVERIFY(tube.connectDevice(nullptr));
        tube.emitTicks(2000);
        QCOMPARE(counts.size(), 2000);

        double sum = 0.0;
        for (double c : counts)
            sum += c;
        const double mean = sum / counts.size();
        double variance = 0.0;
        for (double c : counts)
            variance += (c - mean) * (c - mean);
        variance /= counts.size() - 1;

        // Expected 25 counts per tick; sigma of the mean ≈ 0.11.
        QVERIFY(qAbs(mean - 25.0) < 0.5);
        // Fano factor of a Poisson process is 1; estimator noise ≈ ±4.5%.
        QVERIFY(variance / mean > 0.85);
        QVERIFY(variance / mean < 1.15);
    }

    void rollingCpmMatchesHandComputedSequence()
    {
        GeigerMath::RollingWindow window(60000);
        window.add(0, 2.0);
        window.add(30000, 4.0);
        QCOMPARE(window.meanCps(), 3.0);   // (2+4)/2
        QCOMPARE(window.cpm(), 180.0);

        // t=61000 prunes t=0 (older than 61000-60000); (4+6)/2 = 5 CPS.
        window.add(61000, 6.0);
        QCOMPARE(window.count(), 2);
        QCOMPARE(window.cpm(), 300.0);
        QCOMPARE(window.maxCps(), 6.0);

        window.clear();
        QCOMPARE(window.cpm(), 0.0);
    }

    void doseConversionAndThreshold()
    {
        QCOMPARE(GeigerMath::doseMicroSvPerHour(100.0, 0.0057), 0.57);

        GeigerService service;
        QCOMPARE(service.tubeFactor(), 0.0057);   // SBM-20 default
        QCOMPARE(service.alertThreshold(), 0.5);  // µSv/h default
    }

    void codecParsesGeigerFramesAndIgnoresSpectra()
    {
        GeigerCodec codec;
        QVector<GeigerReading> readings;
        codec.setSink([&readings](SensorReading reading) {
            if (std::holds_alternative<GeigerReading>(reading))
                readings.append(std::get<GeigerReading>(reading));
        });

        // Interleaved stream: spectrum, geiger, spectrum, geiger.
        codec.feed(makeSpectrumFrame(1) + makeGeigerFrame(2, 12.5f)
                   + makeSpectrumFrame(3) + makeGeigerFrame(4, 50.0f));
        QCOMPARE(readings.size(), 2);
        QCOMPARE(readings[0].timestampMs, qint64(2));
        QCOMPARE(readings[0].countsPerSecond, 12.5);
        QCOMPARE(readings[1].countsPerSecond, 50.0);
    }

    void codecResynchronizesAfterCorruption()
    {
        GeigerCodec codec;
        int delivered = 0;
        codec.setSink([&delivered](SensorReading) { ++delivered; });

        QByteArray bad = makeGeigerFrame(1, 10.0f);
        bad[4] = char(0xFF);  // wreck the magic
        codec.feed(bad + makeGeigerFrame(2, 20.0f));
        QCOMPARE(delivered, 1);
    }

    void interleavedStreamDispatchesToTheCorrectTypedSignal()
    {
        // The same interleaved byte stream through each codec's CodecDevice:
        // every reading lands on the matching typed signal, never the other.
        const QByteArray stream = makeSpectrumFrame(1) + makeGeigerFrame(2, 30.0f)
                                  + makeGeigerFrame(3, 31.0f);

        auto geigerTransport = std::make_unique<ScriptedTransport>();
        ScriptedTransport* geigerScripted = geigerTransport.get();
        CodecDevice geigerDevice(QStringLiteral("geiger"), std::move(geigerTransport),
                                 std::make_unique<GeigerCodec>());
        QSignalSpy geigerReadings(&geigerDevice, &SensorDevice::readingReady);
        QSignalSpy geigerSpectra(&geigerDevice, &SensorDevice::spectrumReady);
        QVERIFY(geigerDevice.connectDevice(nullptr));
        geigerScripted->enqueueIncoming(stream);
        QCOMPARE(geigerReadings.count(), 2);
        QCOMPARE(geigerSpectra.count(), 0);

        auto spectroTransport = std::make_unique<ScriptedTransport>();
        ScriptedTransport* spectroScripted = spectroTransport.get();
        CodecDevice spectroDevice(QStringLiteral("spectro"), std::move(spectroTransport),
                                  std::make_unique<As7265xCodec>());
        QSignalSpy spectroReadings(&spectroDevice, &SensorDevice::readingReady);
        QSignalSpy spectroSpectra(&spectroDevice, &SensorDevice::spectrumReady);
        QVERIFY(spectroDevice.connectDevice(nullptr));
        spectroScripted->enqueueIncoming(stream);
        QCOMPARE(spectroSpectra.count(), 1);
        QCOMPARE(spectroReadings.count(), 0);
    }

    void bridgeGeigerModeStreamsThroughTheFullBytePath()
    {
        auto transport = std::unique_ptr<SimulatedBridgeTransport>(
            SimulatedBridgeTransport::geigerSource(20.0));
        SimulatedBridgeTransport* bridge = transport.get();
        CodecDevice device(QStringLiteral("loopback geiger"), std::move(transport),
                           std::make_unique<GeigerCodec>());

        QSignalSpy readings(&device, &SensorDevice::readingReady);
        QSignalSpy spectra(&device, &SensorDevice::spectrumReady);
        QVERIFY(device.connectDevice(nullptr));
        device.start();
        QVERIFY(bridge->isStarted());
        bridge->emitFrames(10);

        QCOMPARE(readings.count(), 10);
        QCOMPARE(spectra.count(), 0);  // typed dispatch: geiger only
        const auto reading = readings.last().first().value<GeigerReading>();
        QVERIFY(reading.countsPerSecond >= 0.0);
        device.stop();
        QVERIFY(!bridge->isStarted());
    }

    void measurementDaoRoundTripAndCascade()
    {
        DatabaseManager db(":memory:");
        QVERIFY(db.database().tables().contains("measurements"));

        const int sessionId = db.m_sessionDao.addSession("Radiation survey");
        const int id = db.m_measurementDao.addMeasurement(
            sessionId, "geiger_dose", 0.32, "uSv/h", R"({"avgCpm":56.1})");
        QVERIFY(id > 0);

        const auto records = db.m_measurementDao.measurements(sessionId);
        QCOMPARE(records.size(), 1);
        QCOMPARE(records.first().type, QStringLiteral("geiger_dose"));
        QCOMPARE(records.first().value, 0.32);
        QCOMPARE(records.first().unit, QStringLiteral("uSv/h"));
        QCOMPARE(records.first().summary, QStringLiteral(R"({"avgCpm":56.1})"));

        db.m_sessionDao.removeSession(sessionId);
        QVERIFY(db.m_measurementDao.measurements(sessionId).isEmpty());
    }

    void serviceCountsAndSavesMeasurement()
    {
        DatabaseManager db(":memory:");
        GeigerService service;
        QCOMPARE(service.availableDevices().size(), 3);

        int sessionId = -1;
        service.setMeasurementStore(&db.m_measurementDao, [&db, &sessionId]() {
            if (sessionId < 0)
                sessionId = db.m_sessionDao.addSession("Geiger session");
            return sessionId;
        });

        // Check-source device: high rate, so readings are visibly non-zero.
        service.setCurrentDeviceIndex(1);
        QSignalSpy updates(&service, &GeigerService::readingUpdated);
        service.startAcquisition();  // auto-connects
        QVERIFY(service.isConnected());
        QVERIFY(service.isAcquiring());
        QTRY_VERIFY_WITH_TIMEOUT(updates.count() >= 2, 3000);
        QVERIFY(service.countsPerMinute() > 0.0);
        QVERIFY(service.doseMicroSvPerHour() > 0.0);

        QSignalSpy saved(&service, &GeigerService::measurementSaved);
        service.saveMeasurement(QStringLiteral("bench check"), QString());
        QCOMPARE(saved.count(), 1);

        const auto records = db.m_measurementDao.measurements(sessionId);
        QCOMPARE(records.size(), 1);
        const QJsonObject summary =
            QJsonDocument::fromJson(records.first().summary.toUtf8()).object();
        QCOMPARE(summary.value("name").toString(), QStringLiteral("bench check"));
        QVERIFY(summary.value("avgCpm").toDouble() > 0.0);

        service.stopAcquisition();
        QVERIFY(!service.isAcquiring());
    }

};

QTEST_GUILESS_MAIN(TstGeiger)
#include "tst_geiger.moc"
