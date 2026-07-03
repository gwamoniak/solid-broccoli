#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <variant>

#include <QMetaType>
#include <QVector>

#include "spectro-core_global.h"

// Acquisition settings shared by every sensor modality. Integration time is
// the sensor exposure per reading; averaging is how many independent readings
// are combined into one emitted value.
struct SPECTROCORE_EXPORT AcquisitionParams
{
    int integrationTimeMs = 100;
    int averaging = 1;
};

// One optical spectrum: intensity counts over an ascending wavelength grid,
// plus the settings it was captured with. Value type — safe to copy across
// threads in queued signal connections.
class SPECTROCORE_EXPORT Spectrum
{
public:
    enum class Kind { Sample, Dark, Reference };

    QVector<double> wavelengthsNm;   // ascending, same length as counts
    QVector<double> counts;
    qint64 timestampMs = 0;          // ms since epoch
    AcquisitionParams params;
    Kind kind = Kind::Sample;

    // Equal non-zero lengths and strictly ascending wavelengths.
    bool isValid() const;
};

// One radiation reading (Milestone 10 hardware; the type exists now so the
// codec sink never changes shape when the modality arrives).
struct SPECTROCORE_EXPORT GeigerReading
{
    qint64 timestampMs = 0;
    double countsPerSecond = 0.0;
};

// What a protocol codec delivers: whichever modality the device speaks.
using SensorReading = std::variant<Spectrum, GeigerReading>;

Q_DECLARE_METATYPE(Spectrum)
Q_DECLARE_METATYPE(GeigerReading)

#endif // SPECTRUM_H
