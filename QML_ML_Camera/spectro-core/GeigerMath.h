#ifndef GEIGERMATH_H
#define GEIGERMATH_H

#include <QPair>
#include <QVector>

#include "spectro-core_global.h"

// Derived radiation quantities, kept pure so they test against hand-computed
// sequences. The codec/device layer reports raw counts-per-second; everything
// the UI shows (rolling CPM, dose rate) is computed here, app-side.
namespace GeigerMath {

// Rolling window over (timestampMs, cps) readings: CPM is the mean CPS over
// the retained window scaled to a minute, so it is defined (if noisy) from
// the very first reading instead of needing a full minute of history.
class SPECTROCORE_EXPORT RollingWindow
{
public:
    explicit RollingWindow(qint64 windowMs = 60000);

    // Readings must arrive in timestamp order; entries older than
    // (newest - windowMs) are pruned.
    void add(qint64 timestampMs, double cps);
    void clear();

    double meanCps() const;
    double cpm() const { return meanCps() * 60.0; }
    double maxCps() const;
    int count() const { return m_entries.size(); }
    const QVector<QPair<qint64, double>>& entries() const { return m_entries; }

private:
    qint64 m_windowMs;
    QVector<QPair<qint64, double>> m_entries;
};

// Dose rate from count rate: µSv/h = CPM × tube conversion factor. The
// factor is a property of the tube (default 0.0057 for the common SBM-20
// approximation) and lives in settings, not here.
SPECTROCORE_EXPORT double doseMicroSvPerHour(double cpm, double tubeFactor);

} // namespace GeigerMath

#endif // GEIGERMATH_H
