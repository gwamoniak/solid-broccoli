#include "Spectrum.h"

bool Spectrum::isValid() const
{
    if (wavelengthsNm.isEmpty() || wavelengthsNm.size() != counts.size())
        return false;
    for (int i = 1; i < wavelengthsNm.size(); ++i) {
        if (wavelengthsNm[i] <= wavelengthsNm[i - 1])
            return false;
    }
    return true;
}

// Queued signal connections between device threads and the GUI thread need
// the metatypes registered before the first cross-thread emission.
static void registerSpectroMetaTypes()
{
    qRegisterMetaType<Spectrum>("Spectrum");
    qRegisterMetaType<GeigerReading>("GeigerReading");
}
Q_CONSTRUCTOR_FUNCTION(registerSpectroMetaTypes)
