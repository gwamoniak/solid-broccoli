#ifndef SPECTRUMSYNTHESIZER_H
#define SPECTRUMSYNTHESIZER_H

#include <QVector>

#include "spectro-sim_global.h"

// Ideal physics, no noise: pure functions producing relative flux (peak
// ~1.0) on a wavelength grid. The InstrumentModel turns this into realistic
// counts. Because these functions ARE the ground truth, tests can assert
// that the analysis pipeline recovers exactly what was injected here.
namespace SpectrumSynthesizer {

// 2048 points, 340–1020 nm — the standard UV-Vis grid of the simulators.
SPECTROSIM_EXPORT QVector<double> standardGrid();

// The AS7265x's fixed 18-channel wavelength table (nm).
SPECTROSIM_EXPORT QVector<double> as7265xGrid();

// Gaussian emission lines of a mercury lamp: 404.7, 435.8, 546.1 (dominant),
// 577.0, 579.1 nm with realistic relative intensities.
SPECTROSIM_EXPORT QVector<double> mercuryLamp(const QVector<double>& grid, double fwhmNm);

// A few neon lines in the 585–705 nm range (640.2 dominant).
SPECTROSIM_EXPORT QVector<double> neonLamp(const QVector<double>& grid, double fwhmNm);

// Planck's law, normalized to peak 1.0 over the grid — the broadband source
// for absorbance work.
SPECTROSIM_EXPORT QVector<double> blackbody(const QVector<double>& grid, double temperatureK);

// A Gaussian absorbance curve A(lambda) with the given peak absorbance.
SPECTROSIM_EXPORT QVector<double> gaussianAbsorber(const QVector<double>& grid,
                                                   double centerNm, double fwhmNm,
                                                   double peakAbsorbance);

// Beer-Lambert: transmitted = source * 10^(-A * concentration), per element.
SPECTROSIM_EXPORT QVector<double> applyBeerLambert(const QVector<double>& sourceFlux,
                                                   const QVector<double>& absorbance,
                                                   double concentration);

} // namespace SpectrumSynthesizer

#endif // SPECTRUMSYNTHESIZER_H
