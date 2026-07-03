#ifndef SPECTROANALYSIS_H
#define SPECTROANALYSIS_H

#include <QVector>

#include "Spectrum.h"
#include "spectro-core_global.h"

// Pure spectral math. No I/O, no state, no Qt GUI types — every function is
// deterministic and unit-tested against known ground truth. All functions
// tolerate bad input by returning a neutral value (input unchanged, empty
// vector, or 0.0) rather than asserting in release builds.
namespace SpectroAnalysis {

struct SPECTROCORE_EXPORT Peak
{
    int index = -1;
    double wavelengthNm = 0.0;
    double value = 0.0;
    double prominence = 0.0;
};

// Savitzky-Golay smoothing: least-squares polynomial convolution. Supported:
// odd window 5–25, polyOrder 2–3, window <= y.size(). Reproduces polynomials
// up to polyOrder exactly. The first/last window/2 samples are returned
// unchanged (edge copy). Invalid arguments return y unchanged.
SPECTROCORE_EXPORT QVector<double> savitzkyGolay(const QVector<double>& y,
                                                 int window, int polyOrder);

// Local maxima with prominence >= minProminence, at least minDistanceNm apart
// (greedy, highest prominence wins), sorted by prominence descending.
// Prominence = peak height minus the higher of the two minima separating the
// peak from higher ground (or the spectrum edge).
SPECTROCORE_EXPORT QVector<Peak> findPeaks(const Spectrum& s,
                                           double minProminence,
                                           double minDistanceNm);

// Trapezoidal integral of counts over [fromNm, toNm], with linear
// interpolation at the region boundaries. The region is clamped to the
// spectrum range; an empty overlap integrates to 0.
SPECTROCORE_EXPORT double integrate(const Spectrum& s, double fromNm, double toNm);

// Per-element T = (sample - dark) / (reference - dark) with the denominator
// floored at 1e-9 so a bad reference cannot divide by zero. Returns an empty
// vector on length mismatch.
SPECTROCORE_EXPORT QVector<double> transmittance(const Spectrum& sample,
                                                 const Spectrum& dark,
                                                 const Spectrum& reference);

// A = -log10(T), with T clamped to [1e-6, 10] before the log so the result is
// always finite. Returns an empty vector on length mismatch.
SPECTROCORE_EXPORT QVector<double> absorbance(const Spectrum& sample,
                                              const Spectrum& dark,
                                              const Spectrum& reference);

} // namespace SpectroAnalysis

#endif // SPECTROANALYSIS_H
