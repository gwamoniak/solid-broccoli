#ifndef SPECTRALLINELIBRARY_H
#define SPECTRALLINELIBRARY_H

#include <QString>
#include <QVector>

#include "spectro-core_global.h"

// One curated reference feature: an emission line or an absorption band.
struct SPECTROCORE_EXPORT ReferenceLine
{
    enum class Kind { Emission, Absorption };

    double wavelengthNm = 0.0;
    QString species;   // grouping key: "Hg", "Ne", "chlorophyll-a", ...
    QString label;     // human line name: "green line", "D doublet", ...
    Kind kind = Kind::Emission;
};

// The instrument's reference knowledge: a small, curated table of well-known
// emission lines and absorption bands. This is the deterministic half of the
// Milestone 13 reports — every identification a report contains comes from
// matching against this table, never from the language model.
namespace SpectralLineLibrary {

SPECTROCORE_EXPORT const QVector<ReferenceLine>& all();

} // namespace SpectralLineLibrary

#endif // SPECTRALLINELIBRARY_H
