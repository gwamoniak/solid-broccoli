#ifndef INSTRUMENTMODEL_H
#define INSTRUMENTMODEL_H

#include <random>

#include <QVector>

#include "Spectrum.h"
#include "spectro-sim_global.h"

// Everything a real sensor does to ideal flux, parameterized and seeded.
// Identical profiles (same seed) produce bit-identical measurement
// sequences, so tests are exactly reproducible.
struct SPECTROSIM_EXPORT InstrumentProfile
{
    quint32 seed = 1;
    double fullWellCounts = 65535.0;    // 16-bit saturation ceiling
    double darkCurrentPerMs = 0.5;      // counts per ms of integration
    double readNoiseSigma = 8.0;        // counts, per read
    QVector<int> hotPixels;             // indices forced to full well

    // Scale and spectral response: counts produced per ms of integration at
    // flux 1.0, shaped by a smooth quantum-efficiency bell over wavelength.
    double countsPerMsFullFlux = 400.0;
    double qeFloor = 0.3;
    double qeCenterNm = 680.0;
    double qeSigmaNm = 300.0;
};

class SPECTROSIM_EXPORT InstrumentModel
{
public:
    explicit InstrumentModel(const InstrumentProfile& profile = {});

    // Noise-free expectation: qe * flux * scale * t + darkCurrent * t.
    // Public so tests can compute residuals against exact ground truth.
    QVector<double> idealSignal(const QVector<double>& flux,
                                const QVector<double>& gridNm,
                                const AcquisitionParams& params) const;

    // One measurement: ideal signal + shot noise (sigma = sqrt(signal)) +
    // read noise, hot pixels forced to full well, clipped to [0, fullWell],
    // averaged over params.averaging independent draws. Advances the RNG.
    Spectrum measure(const QVector<double>& flux,
                     const QVector<double>& gridNm,
                     const AcquisitionParams& params);

    const InstrumentProfile& profile() const { return m_profile; }

private:
    double quantumEfficiency(double wavelengthNm) const;

    InstrumentProfile m_profile;
    std::mt19937 m_rng;
};

#endif // INSTRUMENTMODEL_H
