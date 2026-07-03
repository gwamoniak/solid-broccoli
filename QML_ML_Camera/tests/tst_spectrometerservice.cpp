#include <QSignalSpy>
#include <QtTest>

#include <cmath>

#include "SpectroAnalysis.h"
#include "SpectrometerService.h"

// Drives the real service against its built-in simulated devices (device 0 =
// mercury lamp, device 1 = Beer-Lambert dye sample). Frames arrive on QTimer
// ticks, so tests wait on the spectrumUpdated signal. The dye checks reuse
// the Milestone 3 ground-truth approach at the service level: the simulator
// injected a 0.8-absorbance band at 664 nm (FWHM 45 nm).
class TstSpectrometerService : public QObject
{
    Q_OBJECT

private:
    // Runs the calibration workflow on the dye device: start, capture dark
    // (lamp off for a frame), capture reference (blank for a frame).
    static void calibrate(SpectrometerService& service, QSignalSpy& frames)
    {
        service.setCurrentDeviceIndex(1);
        service.setIntegrationTimeMs(5);  // 33 ms tick floor -> fast frames
        service.startAcquisition();
        QVERIFY(frames.wait(1000));

        service.captureDark();
        QTRY_VERIFY_WITH_TIMEOUT(service.hasDark(), 1000);
        service.captureReference();
        QTRY_VERIFY_WITH_TIMEOUT(service.hasReference(), 1000);
    }

private slots:
    void modeRefusesWithoutCalibration()
    {
        SpectrometerService service;
        QSignalSpy errorSpy(&service, &SpectrometerService::errorOccurred);

        service.setMode(2);
        QCOMPARE(service.mode(), 0);
        QCOMPARE(errorSpy.count(), 1);
    }

    void connectAndAcquireEmitsFrames()
    {
        SpectrometerService service;
        QSignalSpy frames(&service, &SpectrometerService::spectrumUpdated);

        QVERIFY(!service.isConnected());
        service.setIntegrationTimeMs(5);
        service.startAcquisition();  // auto-connects
        QVERIFY(service.isConnected());
        QVERIFY(service.isAcquiring());
        QVERIFY(frames.wait(1000));
        QVERIFY(service.latestSpectrumSnapshot().isValid());
        QVERIFY(service.peakWavelengthNm() > 0.0);  // mercury 546.1 dominant

        service.stopAcquisition();
        QVERIFY(!service.isAcquiring());
    }

    // After Dark + Ref, absorbance mode must hand the view exactly what
    // SpectroAnalysis::absorbance computes from the same three spectra.
    void absorbanceModeMatchesAnalysis()
    {
        SpectrometerService service;
        QSignalSpy frames(&service, &SpectrometerService::spectrumUpdated);
        calibrate(service, frames);

        service.setMode(2);
        QCOMPARE(service.mode(), 2);
        frames.clear();
        QVERIFY(frames.wait(1000));

        const Spectrum live = service.latestSpectrumSnapshot();
        const QVector<double> expected = SpectroAnalysis::absorbance(
            live, service.darkSpectrum(), service.referenceSpectrum());
        QCOMPARE(service.displaySpectrum().counts, expected);

        // The recovered band peaks near the injected 664 nm / A = 0.8
        // (tolerances cover drift between calibration and sample frames).
        const auto peaks = SpectroAnalysis::findPeaks(service.displaySpectrum(), 0.2, 10.0);
        QVERIFY(!peaks.isEmpty());
        QVERIFY2(std::abs(peaks.first().wavelengthNm - 664.0) <= 3.0,
                 qPrintable(QString::number(peaks.first().wavelengthNm)));
        QVERIFY2(std::abs(peaks.first().value - 0.8) <= 0.1,
                 qPrintable(QString::number(peaks.first().value)));

        service.stopAcquisition();
    }

    // Integration over the absorbance band: the injected Gaussian has area
    // peakA * sigma * sqrt(2*pi) = 0.8 * (45/2.3548) * 2.5066 ~= 38.3.
    // Amplitude drift (±3%) between calibration and sample frames offsets
    // the whole absorbance curve, so the window is generous.
    void integralRecoversBandArea()
    {
        SpectrometerService service;
        QSignalSpy frames(&service, &SpectrometerService::spectrumUpdated);
        calibrate(service, frames);
        service.setMode(2);

        service.setIntegrationRegion(600.0, 730.0);
        QVERIFY(service.hasIntegrationRegion());
        frames.clear();
        QVERIFY(frames.wait(1000));

        const double integral = service.integralValue();
        QVERIFY(std::isfinite(integral));
        QVERIFY2(std::abs(integral - 38.3) <= 8.0, qPrintable(QString::number(integral)));

        // Stable across frames within the drift envelope.
        frames.clear();
        QVERIFY(frames.wait(1000));
        QVERIFY2(std::abs(service.integralValue() - integral) <= 4.0,
                 qPrintable(QString::number(service.integralValue())));

        service.stopAcquisition();
    }

    void peakModelListsMercuryLines()
    {
        SpectrometerService service;
        QSignalSpy frames(&service, &SpectrometerService::spectrumUpdated);
        service.setIntegrationTimeMs(5);
        service.startAcquisition();
        QVERIFY(frames.wait(1000));

        auto* model = qobject_cast<QAbstractListModel*>(service.peakModel());
        QVERIFY(model);
        QTRY_VERIFY_WITH_TIMEOUT(model->rowCount() >= 3, 1000);

        // Top row is the dominant 546.1 nm line (rows sorted by prominence).
        const QModelIndex top = model->index(0, 0);
        const double topNm = model->data(top, Qt::UserRole + 1).toDouble();
        QVERIFY2(std::abs(topNm - 546.1) <= 1.0, qPrintable(QString::number(topNm)));

        service.stopAcquisition();
    }

    void smoothingKeepsPeakPosition()
    {
        SpectrometerService service;
        QSignalSpy frames(&service, &SpectrometerService::spectrumUpdated);
        service.setIntegrationTimeMs(5);
        service.startAcquisition();
        QVERIFY(frames.wait(1000));
        const double before = service.peakWavelengthNm();

        service.setSmoothingWindow(11);
        QCOMPARE(service.smoothingWindow(), 11);
        frames.clear();
        QVERIFY(frames.wait(1000));
        QVERIFY2(std::abs(service.peakWavelengthNm() - before) <= 1.0,
                 qPrintable(QString::number(service.peakWavelengthNm())));

        // Even values normalize to odd; zero disables.
        service.setSmoothingWindow(8);
        QCOMPARE(service.smoothingWindow(), 9);
        service.setSmoothingWindow(0);
        QCOMPARE(service.smoothingWindow(), 0);

        service.stopAcquisition();
    }
};

QTEST_GUILESS_MAIN(TstSpectrometerService)
#include "tst_spectrometerservice.moc"
