#ifndef SIMSCENE_H
#define SIMSCENE_H

#include "spectro-sim_global.h"

// What the simulated spectrometer is "looking at": a light source, an
// optional Beer-Lambert absorber in the light path, and a slow amplitude
// drift so live views visibly breathe. Plain data — the device evaluates it
// through SpectrumSynthesizer and InstrumentModel each tick.
struct SPECTROSIM_EXPORT SimScene
{
    enum class Source { Mercury, Neon, Blackbody };

    Source source = Source::Mercury;
    double lineFwhmNm = 2.0;       // emission line width (lamp sources)
    double blackbodyK = 5500.0;    // source temperature (Blackbody source)
    bool lampOn = true;            // false = dark capture (noise floor only)

    // Absorber (active when peak absorbance > 0); concentration 0 = blank.
    double absorberCenterNm = 0.0;
    double absorberFwhmNm = 0.0;
    double absorberPeakA = 0.0;
    double concentration = 1.0;

    // Sinusoidal amplitude drift: factor 1 + amplitude*sin(2*pi*t/period).
    double driftAmplitude = 0.03;
    double driftPeriodMs = 10000.0;
};

#endif // SIMSCENE_H
