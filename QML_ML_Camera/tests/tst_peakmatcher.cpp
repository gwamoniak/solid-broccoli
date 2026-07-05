#include <QtTest>

#include "PeakMatcher.h"
#include "SpectrumSynthesizer.h"

// Identification ground truth: the simulator's mercury lamp must be named
// mercury. Pure functions, no model, no I/O.
class TstPeakMatcher : public QObject
{
    Q_OBJECT

private:
    static QVector<SpectroAnalysis::Peak> peaksAt(std::initializer_list<double> nm)
    {
        QVector<SpectroAnalysis::Peak> peaks;
        for (double wavelength : nm)
            peaks.append({0, wavelength, 100.0, 50.0});
        return peaks;
    }

private slots:
    void mercuryFiveLineSetRanksMercuryFirst()
    {
        const auto candidates =
            PeakMatcher::match(peaksAt({404.7, 435.8, 546.1, 577.0, 579.1}));
        QVERIFY(!candidates.isEmpty());
        QCOMPARE(candidates.first().species, QStringLiteral("Hg"));
        QCOMPARE(candidates.first().matchedPeaks.size(), 5);
        // Exact hits: each line contributes the full 2.0.
        QCOMPARE(candidates.first().score, 10.0);
    }

    void singleGreenLineFindsMercury()
    {
        const auto candidates = PeakMatcher::match(peaksAt({546.1}));
        bool foundHg = false;
        for (const auto& candidate : candidates) {
            if (candidate.species == QLatin1String("Hg")) {
                foundHg = true;
                QCOMPARE(candidate.matchedPeaks.size(), 1);
                QCOMPARE(candidate.matchedPeaks.first().second.label,
                         QStringLiteral("green line"));
            }
        }
        QVERIFY(foundHg);
    }

    void multiLineCoincidenceOutranksSingleAccidentalHit()
    {
        // Three Hg lines plus one peak sitting exactly on the Ne red line:
        // mercury must still rank first.
        const auto candidates =
            PeakMatcher::match(peaksAt({404.7, 435.8, 546.1, 640.2}));
        QCOMPARE(candidates.first().species, QStringLiteral("Hg"));
        QVERIFY(candidates.first().score > 3.0);
    }

    void toleranceEdgesAreRespected()
    {
        // 2.9 nm off: matched; 3.1 nm off: not.
        QVERIFY(!PeakMatcher::match(peaksAt({549.0})).isEmpty());   // 546.1 + 2.9
        const auto missed = PeakMatcher::match(peaksAt({549.2}));   // 546.1 + 3.1
        for (const auto& candidate : missed)
            QVERIFY(candidate.species != QLatin1String("Hg"));
    }

    void noPeaksMeansNoCandidates()
    {
        QVERIFY(PeakMatcher::match({}).isEmpty());
        // A no-match never produces a forced guess.
        QVERIFY(PeakMatcher::match(peaksAt({999.0})).isEmpty());
    }

    void toleranceScalesWithGridCoarseness()
    {
        // 2048-point grid: fine spacing, floor of 3 nm holds.
        QCOMPARE(PeakMatcher::toleranceForGrid(SpectrumSynthesizer::standardGrid()), 3.0);
        // 18-channel AS7265x: median spacing 25 nm -> 12.5 nm window.
        QCOMPARE(PeakMatcher::toleranceForGrid(SpectrumSynthesizer::as7265xGrid()), 12.5);
    }
};

QTEST_GUILESS_MAIN(TstPeakMatcher)
#include "tst_peakmatcher.moc"
