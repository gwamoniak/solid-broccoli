#include "PeakMatcher.h"

#include <algorithm>
#include <cmath>

#include <QMap>

namespace PeakMatcher {

QVector<MatchCandidate> match(const QVector<SpectroAnalysis::Peak>& peaks,
                              double toleranceNm)
{
    QMap<QString, MatchCandidate> bySpecies;  // QMap: stable alphabetical order

    for (const ReferenceLine& line : SpectralLineLibrary::all()) {
        // Closest detected peak to this reference line, if within tolerance.
        double bestDelta = toleranceNm;
        const SpectroAnalysis::Peak* bestPeak = nullptr;
        for (const SpectroAnalysis::Peak& peak : peaks) {
            const double delta = std::abs(peak.wavelengthNm - line.wavelengthNm);
            if (delta <= bestDelta) {
                bestDelta = delta;
                bestPeak = &peak;
            }
        }
        if (!bestPeak)
            continue;

        MatchCandidate& candidate = bySpecies[line.species];
        candidate.species = line.species;
        candidate.score += 1.0 + (1.0 - bestDelta / toleranceNm);
        candidate.matchedPeaks.append({bestPeak->wavelengthNm, line});
    }

    QVector<MatchCandidate> ranked;
    ranked.reserve(bySpecies.size());
    for (const MatchCandidate& candidate : bySpecies)
        ranked.append(candidate);
    std::stable_sort(ranked.begin(), ranked.end(),
                     [](const MatchCandidate& a, const MatchCandidate& b) {
                         return a.score > b.score;  // ties keep name order
                     });
    return ranked;
}

double toleranceForGrid(const QVector<double>& wavelengthsNm)
{
    if (wavelengthsNm.size() < 2)
        return 3.0;
    QVector<double> spacings;
    spacings.reserve(wavelengthsNm.size() - 1);
    for (int i = 1; i < wavelengthsNm.size(); ++i)
        spacings.append(wavelengthsNm[i] - wavelengthsNm[i - 1]);
    std::sort(spacings.begin(), spacings.end());
    const double median = spacings[spacings.size() / 2];
    return std::max(3.0, median / 2.0);
}

} // namespace PeakMatcher
