#include "SpectrumSynthesizer.h"

#include <algorithm>
#include <cmath>

namespace SpectrumSynthesizer {

namespace {

constexpr double kFwhmToSigma = 2.354820045;  // FWHM = 2*sqrt(2*ln2) * sigma

QVector<double> emissionLines(const QVector<double>& grid,
                              std::initializer_list<std::pair<double, double>> lines,
                              double fwhmNm)
{
    const double sigma = fwhmNm / kFwhmToSigma;
    QVector<double> flux(grid.size(), 0.0);
    for (int i = 0; i < grid.size(); ++i) {
        for (const auto& [center, intensity] : lines) {
            const double d = (grid[i] - center) / sigma;
            flux[i] += intensity * std::exp(-0.5 * d * d);
        }
    }
    return flux;
}

} // namespace

QVector<double> standardGrid()
{
    constexpr int n = 2048;
    QVector<double> grid(n);
    for (int i = 0; i < n; ++i)
        grid[i] = 340.0 + (1020.0 - 340.0) * i / (n - 1);
    return grid;
}

QVector<double> as7265xGrid()
{
    return {410, 435, 460, 485, 510, 535, 560, 585, 610,
            645, 680, 705, 730, 760, 810, 860, 900, 940};
}

QVector<double> mercuryLamp(const QVector<double>& grid, double fwhmNm)
{
    return emissionLines(grid,
                         {{404.7, 0.36}, {435.8, 0.70}, {546.1, 1.00},
                          {577.0, 0.45}, {579.1, 0.42}},
                         fwhmNm);
}

QVector<double> neonLamp(const QVector<double>& grid, double fwhmNm)
{
    return emissionLines(grid,
                         {{585.2, 0.50}, {614.3, 0.70}, {640.2, 1.00}, {703.2, 0.60}},
                         fwhmNm);
}

QVector<double> blackbody(const QVector<double>& grid, double temperatureK)
{
    // Planck's law in wavelength form; only the shape matters, so constants
    // reduce to the second radiation constant c2 = h*c/k = 1.4388e7 nm*K.
    constexpr double c2 = 1.4388e7;
    QVector<double> flux(grid.size());
    double maxV = 0.0;
    for (int i = 0; i < grid.size(); ++i) {
        const double lambda = grid[i];
        flux[i] = 1.0 / (std::pow(lambda, 5.0)
                         * (std::exp(c2 / (lambda * temperatureK)) - 1.0));
        maxV = std::max(maxV, flux[i]);
    }
    for (double& v : flux)
        v /= maxV;
    return flux;
}

QVector<double> gaussianAbsorber(const QVector<double>& grid, double centerNm,
                                 double fwhmNm, double peakAbsorbance)
{
    const double sigma = fwhmNm / kFwhmToSigma;
    QVector<double> a(grid.size());
    for (int i = 0; i < grid.size(); ++i) {
        const double d = (grid[i] - centerNm) / sigma;
        a[i] = peakAbsorbance * std::exp(-0.5 * d * d);
    }
    return a;
}

QVector<double> applyBeerLambert(const QVector<double>& sourceFlux,
                                 const QVector<double>& absorbance,
                                 double concentration)
{
    Q_ASSERT(sourceFlux.size() == absorbance.size());
    QVector<double> out(sourceFlux.size());
    for (int i = 0; i < sourceFlux.size(); ++i)
        out[i] = sourceFlux[i] * std::pow(10.0, -absorbance[i] * concentration);
    return out;
}

} // namespace SpectrumSynthesizer
