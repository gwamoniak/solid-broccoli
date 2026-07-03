#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <cmath>

#include "InstrumentModel.h"
#include "ReplayTransport.h"
#include "ScriptedTransport.h"
#include "SimulatedSpectrometer.h"
#include "SpectroAnalysis.h"
#include "SpectrumSynthesizer.h"
#include "TraceRecorder.h"

// The simulator knows its own ground truth, so these tests are physics
// experiments: the analysis pipeline must recover the exact lines, band
// depths, and statistics that the synthesizer injected. Every tolerance is
// justified by the configured noise model.
class TstSpectroSim : public QObject
{
    Q_OBJECT

private:
    static double stddev(const QVector<double>& values)
    {
        double mean = 0.0;
        for (double v : values)
            mean += v;
        mean /= values.size();
        double var = 0.0;
        for (double v : values)
            var += (v - mean) * (v - mean);
        return std::sqrt(var / (values.size() - 1));
    }

    static double mean(const QVector<double>& values)
    {
        double m = 0.0;
        for (double v : values)
            m += v;
        return m / values.size();
    }

private slots:
    // The five mercury lines went in; peak detection must get them back:
    // 546.1 nm dominant within half a grid step (0.33 nm) plus noise wobble,
    // and at least 4 of 5 lines located within 1 nm.
    void peakDetectionRecoversMercuryLines()
    {
        const auto grid = SpectrumSynthesizer::standardGrid();
        const auto flux = SpectrumSynthesizer::mercuryLamp(grid, 2.0);

        InstrumentModel model({});
        const Spectrum s = model.measure(flux, grid, {100, 4});
        QVERIFY(s.isValid());

        const auto peaks = SpectroAnalysis::findPeaks(s, 2000.0, 1.5);
        QVERIFY(peaks.size() >= 4);
        QVERIFY2(std::abs(peaks[0].wavelengthNm - 546.1) <= 0.5,
                 qPrintable(QString("top peak at %1").arg(peaks[0].wavelengthNm)));

        const QVector<double> lines{404.7, 435.8, 546.1, 577.0, 579.1};
        int found = 0;
        for (double line : lines) {
            for (const auto& p : peaks) {
                if (std::abs(p.wavelengthNm - line) <= 1.0) {
                    ++found;
                    break;
                }
            }
        }
        QVERIFY2(found >= 4, qPrintable(QString("found %1 of 5 lines").arg(found)));
    }

    // Beer-Lambert round trip: a 0.8-absorbance band at 664 nm measured as
    // sample / blank reference / lamp-off dark must come back as 0.8 ± 0.05
    // at 664 ± 2 nm, and halving the concentration must halve the recovered
    // absorbance (the linearity that defines the law). Tolerances: shot
    // noise at ~6000 sample counts with averaging 8 perturbs A by well under
    // 0.01; 0.05 leaves margin for the QE-independent residuals.
    void absorbanceRecoversDyeBand()
    {
        const auto grid = SpectrumSynthesizer::standardGrid();
        const auto source = SpectrumSynthesizer::blackbody(grid, 5500.0);
        const auto absorber = SpectrumSynthesizer::gaussianAbsorber(grid, 664.0, 45.0, 0.8);
        const QVector<double> darkFlux(grid.size(), 0.0);
        const AcquisitionParams params{100, 8};

        InstrumentModel model({});
        Spectrum reference = model.measure(source, grid, params);
        Spectrum dark = model.measure(darkFlux, grid, params);

        const auto recover = [&](double concentration) {
            const auto flux = SpectrumSynthesizer::applyBeerLambert(source, absorber,
                                                                    concentration);
            const Spectrum sample = model.measure(flux, grid, params);
            const auto a = SpectroAnalysis::absorbance(sample, dark, reference);

            double maxA = 0.0;
            double maxNm = 0.0;
            for (int i = 0; i < grid.size(); ++i) {
                if (grid[i] < 600.0 || grid[i] > 730.0)
                    continue;
                if (a[i] > maxA) {
                    maxA = a[i];
                    maxNm = grid[i];
                }
            }
            return std::make_pair(maxA, maxNm);
        };

        const auto [fullA, fullNm] = recover(1.0);
        QVERIFY2(std::abs(fullA - 0.8) <= 0.05,
                 qPrintable(QString("recovered A=%1").arg(fullA)));
        QVERIFY2(std::abs(fullNm - 664.0) <= 2.0,
                 qPrintable(QString("band at %1 nm").arg(fullNm)));

        const auto [halfA, halfNm] = recover(0.5);
        QVERIFY(std::abs(halfNm - 664.0) <= 2.0);
        QVERIFY2(std::abs(halfA - 0.4) <= 0.05,
                 qPrintable(QString("half-concentration A=%1").arg(halfA)));
    }

    // Two models with the same profile must produce bit-identical
    // measurements — the property that makes every other test reproducible.
    void identicalSeedsGiveIdenticalSpectra()
    {
        const auto grid = SpectrumSynthesizer::standardGrid();
        const auto flux = SpectrumSynthesizer::mercuryLamp(grid, 2.0);

        InstrumentProfile profile;
        profile.seed = 1234;
        InstrumentModel a(profile);
        InstrumentModel b(profile);
        QCOMPARE(a.measure(flux, grid, {100, 2}).counts,
                 b.measure(flux, grid, {100, 2}).counts);
    }

    // Signal scales with exposure; read noise does not. Doubling integration
    // time doubles the mean (within the tiny error of a 512-point mean);
    // at zero flux and zero dark current the residual spread is pure read
    // noise and must not change with integration time.
    void integrationTimeScalesSignalNotReadNoise()
    {
        QVector<double> grid(512), flux(512, 0.5);
        for (int i = 0; i < 512; ++i)
            grid[i] = 400.0 + i;

        InstrumentProfile profile;
        profile.darkCurrentPerMs = 0.0;
        InstrumentModel model(profile);
        const double m100 = mean(model.measure(flux, grid, {100, 1}).counts);
        const double m200 = mean(model.measure(flux, grid, {200, 1}).counts);
        const double ratio = m200 / m100;
        QVERIFY2(ratio > 1.95 && ratio < 2.05, qPrintable(QString::number(ratio)));

        const QVector<double> zeroFlux(512, 0.0);
        InstrumentModel noiseModel(profile);
        const double s100 = stddev(noiseModel.measure(zeroFlux, grid, {100, 1}).counts);
        const double s200 = stddev(noiseModel.measure(zeroFlux, grid, {200, 1}).counts);
        const double noiseRatio = s200 / s100;
        QVERIFY2(noiseRatio > 0.85 && noiseRatio < 1.15,
                 qPrintable(QString::number(noiseRatio)));
    }

    void saturationClipsAtFullWell()
    {
        QVector<double> grid(256), flux(256, 1.0);
        for (int i = 0; i < 256; ++i)
            grid[i] = 500.0 + i;

        InstrumentModel model({});
        // 1.0 flux * 400 counts/ms * 300 ms = 120000 pre-clip at QE peak.
        const Spectrum s = model.measure(flux, grid, {300, 1});
        const double maxV = *std::max_element(s.counts.cbegin(), s.counts.cend());
        QCOMPARE(maxV, model.profile().fullWellCounts);
        for (double v : s.counts)
            QVERIFY(v <= model.profile().fullWellCounts);
    }

    // Averaging N independent draws shrinks noise by sqrt(N): residuals
    // against the exact ideal signal at averaging 4 must be ~2x tighter than
    // at averaging 1. A 2048-point std estimate is good to ~2%, so the
    // [1.7, 2.4] window is generous.
    void averagingReducesNoiseBySqrtN()
    {
        const auto grid = SpectrumSynthesizer::standardGrid();
        const QVector<double> flux(grid.size(), 0.5);
        const AcquisitionParams one{100, 1};
        const AcquisitionParams four{100, 4};

        InstrumentModel model({});
        const QVector<double> ideal = model.idealSignal(flux, grid, one);

        const auto residualStd = [&](const Spectrum& s) {
            QVector<double> r(s.counts.size());
            for (int i = 0; i < s.counts.size(); ++i)
                r[i] = s.counts[i] - ideal[i];
            return stddev(r);
        };

        const double std1 = residualStd(model.measure(flux, grid, one));
        const double std4 = residualStd(model.measure(flux, grid, four));
        const double ratio = std1 / std4;
        QVERIFY2(ratio > 1.7 && ratio < 2.4, qPrintable(QString::number(ratio)));
    }

    // Device level: the simulated spectrometer streams valid 2048-point
    // spectra on its timer and stops cleanly.
    void simulatedDeviceStreamsSpectra()
    {
        SimulatedSpectrometer* device = SimulatedSpectrometer::mercuryLamp(this);
        QSignalSpy spy(device, &SensorDevice::spectrumReady);

        QVERIFY(device->connectDevice(nullptr));
        device->setParams({33, 1});
        device->start();
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 3, 1000);
        device->stop();

        const auto s = spy.takeFirst().at(0).value<Spectrum>();
        QVERIFY(s.isValid());
        QCOMPARE(s.counts.size(), 2048);

        const int after = spy.count();
        QTest::qWait(120);
        QCOMPARE(spy.count(), after);
    }

    void scriptedTransportChunksAndRecordsWrites()
    {
        ScriptedTransport transport;
        QVERIFY(transport.open(nullptr));
        transport.setChunkSize(4);
        QSignalSpy spy(&transport, &SensorTransport::bytesReceived);

        transport.enqueueIncoming("0123456789");
        QCOMPARE(spy.count(), 3);
        QByteArray joined;
        QCOMPARE(spy.at(0).at(0).toByteArray().size(), 4);
        QCOMPARE(spy.at(2).at(0).toByteArray().size(), 2);
        for (const auto& emission : spy)
            joined += emission.at(0).toByteArray();
        QCOMPARE(joined, QByteArray("0123456789"));

        transport.writeBytes("CMD1");
        transport.writeBytes("CMD2");
        QCOMPARE(transport.writtenBytes(), QByteArray("CMD1CMD2"));
    }

    // Corruption is deterministic under a fixed seed: two transports with
    // the same policy corrupt identically, and probability 1.0 flips every
    // byte (XOR with a nonzero value can never be a no-op).
    void faultInjectionIsDeterministic()
    {
        FaultPolicy policy;
        policy.corruptByteProbability = 1.0;
        policy.seed = 7;
        const QByteArray original("sixteen byte msg");

        const auto corruptOnce = [&]() {
            ScriptedTransport t;
            t.open(nullptr);
            t.setFaultPolicy(policy);
            QSignalSpy spy(&t, &SensorTransport::bytesReceived);
            t.enqueueIncoming(original);
            return spy.takeFirst().at(0).toByteArray();
        };

        const QByteArray a = corruptOnce();
        const QByteArray b = corruptOnce();
        QCOMPARE(a, b);
        QCOMPARE(a.size(), original.size());
        for (int i = 0; i < a.size(); ++i)
            QVERIFY(a[i] != original[i]);
    }

    // What TraceRecorder writes, ReplayTransport plays back identically —
    // rx events only (tx lines document history, they are not replayed).
    void traceRecordReplayRoundTrip()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("session.trace");

        auto* inner = new ScriptedTransport(this);
        TraceRecorder recorder(inner, path);
        QSignalSpy recordedSpy(&recorder, &SensorTransport::bytesReceived);
        QVERIFY(recorder.open(nullptr));
        recorder.writeBytes("CMD");
        inner->enqueueIncoming("ABC");
        inner->enqueueIncoming("DEF");
        QCOMPARE(recordedSpy.count(), 2);
        recorder.close();

        ReplayTransport* replay = ReplayTransport::instant(path, this);
        QSignalSpy replaySpy(replay, &SensorTransport::bytesReceived);
        QVERIFY(replay->open(nullptr));
        QCOMPARE(replaySpy.count(), 2);
        QCOMPARE(replaySpy.at(0).at(0).toByteArray(), QByteArray("ABC"));
        QCOMPARE(replaySpy.at(1).at(0).toByteArray(), QByteArray("DEF"));
    }
};

QTEST_GUILESS_MAIN(TstSpectroSim)
#include "tst_spectrosim.moc"
