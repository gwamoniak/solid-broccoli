#include <QtTest>

#include <cmath>

#include "SpectroAnalysis.h"
#include "Spectrum.h"

// Ground-truth tests for the pure spectral math in spectro-core. Inputs are
// constructed analytically, so every expected value is known exactly.
class TstSpectroAnalysis : public QObject
{
    Q_OBJECT

private:
    static Spectrum makeSpectrum(int n, double fromNm, double toNm,
                                 const std::function<double(double)>& f)
    {
        Spectrum s;
        s.wavelengthsNm.resize(n);
        s.counts.resize(n);
        for (int i = 0; i < n; ++i) {
            const double x = fromNm + (toNm - fromNm) * i / (n - 1);
            s.wavelengthsNm[i] = x;
            s.counts[i] = f(x);
        }
        return s;
    }

    static double gaussian(double x, double center, double sigma, double amplitude)
    {
        const double d = (x - center) / sigma;
        return amplitude * std::exp(-0.5 * d * d);
    }

private slots:
    // A Savitzky-Golay filter of polynomial order p reproduces polynomials of
    // degree <= p exactly; edges are copied, so a pure quadratic must pass
    // through completely unchanged.
    void savitzkyGolayPreservesQuadratic()
    {
        QVector<double> y(101);
        for (int i = 0; i < y.size(); ++i)
            y[i] = 2.0 * i * i - 3.0 * i + 1.0;

        const QVector<double> smoothed = SpectroAnalysis::savitzkyGolay(y, 7, 2);
        QCOMPARE(smoothed.size(), y.size());
        for (int i = 0; i < y.size(); ++i)
            QVERIFY2(std::abs(smoothed[i] - y[i]) < 1e-6,
                     qPrintable(QString("index %1: %2 vs %3").arg(i).arg(smoothed[i]).arg(y[i])));
    }

    void savitzkyGolayRejectsBadArguments()
    {
        const QVector<double> y{1, 2, 3, 4, 5, 6, 7, 8};
        QCOMPARE(SpectroAnalysis::savitzkyGolay(y, 4, 2), y);   // even window
        QCOMPARE(SpectroAnalysis::savitzkyGolay(y, 27, 2), y);  // window too big
        QCOMPARE(SpectroAnalysis::savitzkyGolay(y, 5, 1), y);   // order too low
        QCOMPARE(SpectroAnalysis::savitzkyGolay(y, 9, 2), y);   // window > size
    }

    // Three Gaussians on a flat baseline: findPeaks must locate each center
    // within one sample and rank the tallest first.
    void findPeaksLocatesThreeGaussians()
    {
        const Spectrum s = makeSpectrum(2048, 340.0, 1020.0, [this](double x) {
            return 100.0
                   + gaussian(x, 436.0, 5.0, 1000.0)
                   + gaussian(x, 546.0, 5.0, 2000.0)
                   + gaussian(x, 611.0, 5.0, 1500.0);
        });
        const double sampleStep = (1020.0 - 340.0) / 2047.0;

        const auto peaks = SpectroAnalysis::findPeaks(s, 300.0, 10.0);
        QCOMPARE(peaks.size(), 3);
        QVERIFY(std::abs(peaks[0].wavelengthNm - 546.0) <= sampleStep);
        QVERIFY(std::abs(peaks[1].wavelengthNm - 611.0) <= sampleStep);
        QVERIFY(std::abs(peaks[2].wavelengthNm - 436.0) <= sampleStep);
        QVERIFY(peaks[0].prominence > peaks[1].prominence);
        QVERIFY(peaks[1].prominence > peaks[2].prominence);
    }

    // With boundary interpolation, the integral of a constant over [a, b] is
    // exactly (b - a) * c, including fractional-sample boundaries.
    void integrateConstant()
    {
        const Spectrum s = makeSpectrum(101, 0.0, 100.0, [](double) { return 5.0; });
        QVERIFY(std::abs(SpectroAnalysis::integrate(s, 10.0, 20.0) - 50.0) < 1e-9);
        QVERIFY(std::abs(SpectroAnalysis::integrate(s, 10.5, 11.5) - 5.0) < 1e-9);
        QVERIFY(std::abs(SpectroAnalysis::integrate(s, 20.0, 10.0) - 50.0) < 1e-9);  // swapped
        QCOMPARE(SpectroAnalysis::integrate(s, 200.0, 300.0), 0.0);                  // no overlap
    }

    void absorbanceOfReferenceIsZero()
    {
        const Spectrum sample = makeSpectrum(64, 340, 1020, [](double) { return 1000.0; });
        const Spectrum dark = makeSpectrum(64, 340, 1020, [](double) { return 100.0; });
        const auto a = SpectroAnalysis::absorbance(sample, dark, sample);
        QCOMPARE(a.size(), 64);
        for (double v : a)
            QVERIFY(std::abs(v) < 1e-12);
    }

    // A zero reference must clamp, never divide to NaN/Inf: T floors at the
    // 1e-9 denominator, clamps to 10, and A = -log10(10) = -1.
    void zeroReferenceProducesFiniteAbsorbance()
    {
        const Spectrum sample = makeSpectrum(32, 340, 1020, [](double) { return 500.0; });
        const Spectrum zero = makeSpectrum(32, 340, 1020, [](double) { return 0.0; });
        const auto a = SpectroAnalysis::absorbance(sample, zero, zero);
        QCOMPARE(a.size(), 32);
        for (double v : a) {
            QVERIFY(std::isfinite(v));
            QVERIFY(std::abs(v - (-1.0)) < 1e-9);
        }
    }

    void lengthMismatchReturnsEmpty()
    {
        const Spectrum a = makeSpectrum(32, 340, 1020, [](double) { return 1.0; });
        const Spectrum b = makeSpectrum(16, 340, 1020, [](double) { return 1.0; });
        QVERIFY(SpectroAnalysis::transmittance(a, b, a).isEmpty());
        QVERIFY(SpectroAnalysis::absorbance(a, a, b).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TstSpectroAnalysis)
#include "tst_spectroanalysis.moc"
