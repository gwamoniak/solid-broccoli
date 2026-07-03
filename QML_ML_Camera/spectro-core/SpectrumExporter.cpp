#include "SpectrumExporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

namespace {

bool openForWrite(QFile& file, QString* errorMessage)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("cannot write %1").arg(file.fileName());
        return false;
    }
    return true;
}

} // namespace

bool SpectrumExporter::exportCsv(const Spectrum& spectrum,
                                 const SpectrumExportMetadata& meta,
                                 const QString& filePath, QString* errorMessage)
{
    if (!spectrum.isValid()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("spectrum is empty");
        return false;
    }
    QFile file(filePath);
    if (!openForWrite(file, errorMessage))
        return false;

    QTextStream out(&file);
    out << "# solid-broccoli spectrum export\n";
    out << "# name: " << meta.name << ", kind: " << meta.kind
        << ", captured: " << meta.createdUtc << "\n";
    out << "# integration_ms: " << meta.integrationMs
        << ", averaging: " << meta.averaging << ", tags: " << meta.tags << "\n";
    out << "wavelength_nm,counts\n";
    out.setRealNumberNotation(QTextStream::SmartNotation);
    out.setRealNumberPrecision(10);
    for (int i = 0; i < spectrum.counts.size(); ++i)
        out << spectrum.wavelengthsNm[i] << "," << spectrum.counts[i] << "\n";
    return true;
}

bool SpectrumExporter::exportJson(const Spectrum& spectrum,
                                  const SpectrumExportMetadata& meta,
                                  const QString& filePath, QString* errorMessage)
{
    if (!spectrum.isValid()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("spectrum is empty");
        return false;
    }
    QFile file(filePath);
    if (!openForWrite(file, errorMessage))
        return false;

    QJsonArray wavelengths;
    QJsonArray counts;
    for (int i = 0; i < spectrum.counts.size(); ++i) {
        wavelengths.append(spectrum.wavelengthsNm[i]);
        counts.append(spectrum.counts[i]);
    }
    const QJsonObject object{
        {QStringLiteral("name"), meta.name},
        {QStringLiteral("kind"), meta.kind},
        {QStringLiteral("captured_utc"), meta.createdUtc},
        {QStringLiteral("integration_ms"), meta.integrationMs},
        {QStringLiteral("averaging"), meta.averaging},
        {QStringLiteral("tags"), meta.tags},
        {QStringLiteral("wavelengths_nm"), wavelengths},
        {QStringLiteral("counts"), counts},
    };
    file.write(QJsonDocument(object).toJson(QJsonDocument::Compact));
    return true;
}

bool SpectrumExporter::importCsv(const QString& filePath, Spectrum* spectrum,
                                 QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("cannot read %1").arg(filePath);
        return false;
    }

    Spectrum result;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith("wavelength"))
            continue;
        const QList<QByteArray> parts = line.split(',');
        if (parts.size() != 2)
            continue;
        bool okX = false;
        bool okY = false;
        const double x = parts[0].toDouble(&okX);
        const double y = parts[1].toDouble(&okY);
        if (okX && okY) {
            result.wavelengthsNm.append(x);
            result.counts.append(y);
        }
    }
    if (!result.isValid()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("no data rows in %1").arg(filePath);
        return false;
    }
    if (spectrum)
        *spectrum = result;
    return true;
}
