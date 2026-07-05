#include "SpectralLineLibrary.h"

namespace SpectralLineLibrary {

const QVector<ReferenceLine>& all()
{
    using K = ReferenceLine::Kind;
    static const QVector<ReferenceLine> lines = {
        // Mercury lamp (the simulator's canonical source)
        {404.7, "Hg", "violet line", K::Emission},
        {435.8, "Hg", "blue line", K::Emission},
        {546.1, "Hg", "green line", K::Emission},
        {577.0, "Hg", "yellow doublet (1)", K::Emission},
        {579.1, "Hg", "yellow doublet (2)", K::Emission},
        // Neon (the second simulator lamp)
        {585.2, "Ne", "yellow line", K::Emission},
        {640.2, "Ne", "dominant red line", K::Emission},
        {703.2, "Ne", "deep red line", K::Emission},
        // Sodium
        {589.0, "Na", "D2 line", K::Emission},
        {589.6, "Na", "D1 line", K::Emission},
        // Hydrogen Balmer series
        {656.3, "H", "H-alpha", K::Emission},
        {486.1, "H", "H-beta", K::Emission},
        {434.0, "H", "H-gamma", K::Emission},
        {410.2, "H", "H-delta", K::Emission},
        // Argon
        {763.5, "Ar", "strong NIR line", K::Emission},
        {811.5, "Ar", "strong NIR line", K::Emission},
        // Common laser diodes / DPSS
        {405.0, "laser diode", "violet (Blu-ray)", K::Emission},
        {450.0, "laser diode", "blue", K::Emission},
        {520.0, "laser diode", "green", K::Emission},
        {532.0, "DPSS laser", "green (Nd:YAG 2x)", K::Emission},
        {650.0, "laser diode", "red", K::Emission},
        {780.0, "laser diode", "NIR", K::Emission},
        // Absorption references
        {664.0, "methylene blue", "main absorption band", K::Absorption},
        {430.0, "chlorophyll-a", "Soret band", K::Absorption},
        {662.0, "chlorophyll-a", "red band", K::Absorption},
        {453.0, "chlorophyll-b", "Soret band", K::Absorption},
        {642.0, "chlorophyll-b", "red band", K::Absorption},
        {970.0, "water", "NIR overtone band", K::Absorption},
    };
    return lines;
}

} // namespace SpectralLineLibrary
