#ifndef PEAKMATCHER_H
#define PEAKMATCHER_H

#include <QPair>
#include <QString>
#include <QVector>

#include "SpectralLineLibrary.h"
#include "SpectroAnalysis.h"
#include "spectro-core_global.h"

// Deterministic peak identification against SpectralLineLibrary: pure
// functions, ground-truth tested (the simulator injects a mercury lamp, this
// must rank mercury first). The Milestone 13 language model narrates these
// results; it never produces its own.
namespace PeakMatcher {

struct SPECTROCORE_EXPORT MatchCandidate
{
    QString species;
    double score = 0.0;
    // (detected peak wavelength, matched reference line)
    QVector<QPair<double, ReferenceLine>> matchedPeaks;
};

// Ranks candidate species: each reference line of a species matched by a
// detected peak within toleranceNm contributes 1 + (1 - |dLambda|/tolerance)
// to the species score, so multi-line coincidences (three Hg lines) always
// outrank a single accidental hit. Candidates sorted by descending score,
// ties by species name; species with no match are omitted.
SPECTROCORE_EXPORT QVector<MatchCandidate> match(
    const QVector<SpectroAnalysis::Peak>& peaks, double toleranceNm = 3.0);

// Tolerance suited to a wavelength grid: half the median channel spacing,
// but never below the 3 nm default — coarse filter-bank grids (AS7265x,
// 25-65 nm spacing) need a proportionally wider window.
SPECTROCORE_EXPORT double toleranceForGrid(const QVector<double>& wavelengthsNm);

} // namespace PeakMatcher

#endif // PEAKMATCHER_H
