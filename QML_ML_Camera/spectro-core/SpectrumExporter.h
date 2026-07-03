#ifndef SPECTRUMEXPORTER_H
#define SPECTRUMEXPORTER_H

#include <QString>

#include "Spectrum.h"
#include "spectro-core_global.h"

// File export/import for spectra. CSV carries the metadata as '#' comment
// header lines followed by "wavelength_nm,counts" rows; JSON carries the
// same as one object. importCsv exists so the round trip is testable and so
// exported data can come back as overlays later.
struct SPECTROCORE_EXPORT SpectrumExportMetadata
{
    QString name;
    QString kind;
    QString createdUtc;
    QString tags;
    int integrationMs = 0;
    int averaging = 1;
};

class SPECTROCORE_EXPORT SpectrumExporter
{
public:
    static bool exportCsv(const Spectrum& spectrum, const SpectrumExportMetadata& meta,
                          const QString& filePath, QString* errorMessage = nullptr);
    static bool exportJson(const Spectrum& spectrum, const SpectrumExportMetadata& meta,
                           const QString& filePath, QString* errorMessage = nullptr);
    static bool importCsv(const QString& filePath, Spectrum* spectrum,
                          QString* errorMessage = nullptr);
};

#endif // SPECTRUMEXPORTER_H
