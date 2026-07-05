#ifndef FAULTPOLICY_H
#define FAULTPOLICY_H

#include <QtGlobal>

#include "spectro-sim_global.h"

// Deterministic fault injection for simulated transports. All randomness is
// seeded so a failing test reproduces exactly.
struct SPECTROSIM_EXPORT FaultPolicy
{
    int dropAfterFrames = -1;           // emit closed() after N frames (-1 = never)
    double corruptByteProbability = 0.0;  // per-byte chance of a bit flip
    int stallMs = 0;                    // silence gap injected between deliveries
    quint32 seed = 1;
};

#endif // FAULTPOLICY_H
