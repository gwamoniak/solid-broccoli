#include "InstrumentModel.h"

#include <algorithm>
#include <cmath>

#include <QDateTime>

InstrumentModel::InstrumentModel(const InstrumentProfile& profile)
    : m_profile(profile)
    , m_rng(profile.seed)
{
}

double InstrumentModel::quantumEfficiency(double wavelengthNm) const
{
    const double d = (wavelengthNm - m_profile.qeCenterNm) / m_profile.qeSigmaNm;
    return m_profile.qeFloor + (1.0 - m_profile.qeFloor) * std::exp(-0.5 * d * d);
}

QVector<double> InstrumentModel::idealSignal(const QVector<double>& flux,
                                             const QVector<double>& gridNm,
                                             const AcquisitionParams& params) const
{
    Q_ASSERT(flux.size() == gridNm.size());
    const double t = params.integrationTimeMs;
    QVector<double> ideal(flux.size());
    for (int i = 0; i < flux.size(); ++i) {
        ideal[i] = quantumEfficiency(gridNm[i]) * flux[i] * m_profile.countsPerMsFullFlux * t
                   + m_profile.darkCurrentPerMs * t;
    }
    return ideal;
}

Spectrum InstrumentModel::measure(const QVector<double>& flux,
                                  const QVector<double>& gridNm,
                                  const AcquisitionParams& params)
{
    const QVector<double> ideal = idealSignal(flux, gridNm, params);
    const int n = ideal.size();
    const int draws = std::max(params.averaging, 1);

    std::normal_distribution<double> unitGauss(0.0, 1.0);

    QVector<double> accum(n, 0.0);
    for (int d = 0; d < draws; ++d) {
        for (int i = 0; i < n; ++i) {
            const double shotSigma = std::sqrt(std::max(ideal[i], 0.0));
            double v = ideal[i]
                       + shotSigma * unitGauss(m_rng)
                       + m_profile.readNoiseSigma * unitGauss(m_rng);
            v = std::clamp(v, 0.0, m_profile.fullWellCounts);
            accum[i] += v;
        }
    }

    Spectrum s;
    s.wavelengthsNm = gridNm;
    s.counts.resize(n);
    for (int i = 0; i < n; ++i)
        s.counts[i] = accum[i] / draws;
    // Hot pixels are stuck at the ceiling regardless of exposure.
    for (int hot : m_profile.hotPixels) {
        if (hot >= 0 && hot < n)
            s.counts[hot] = m_profile.fullWellCounts;
    }
    s.timestampMs = QDateTime::currentMSecsSinceEpoch();
    s.params = params;
    return s;
}
