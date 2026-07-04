#include "ReportContextBuilder.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "PeakMatcher.h"
#include "SpectroAnalysis.h"

namespace ReportContextBuilder {

QJsonObject build(const SessionRecord& session,
                  const QVector<SpectrumEntry>& captures,
                  const QVector<MeasurementRecord>& measurements,
                  const QVector<Detection>& detections,
                  const QString& instrumentName)
{
    QJsonObject root;
    root["session"] = QJsonObject{{"name", session.name},
                                  {"createdUtc", session.createdUtc},
                                  {"notes", session.notes}};
    root["instrument"] = instrumentName;

    QJsonArray captureArray;
    for (const SpectrumEntry& entry : captures) {
        QJsonObject capture;
        capture["name"] = entry.name;
        capture["kind"] = SpectrumDAO::kindToString(entry.spectrum.kind);
        capture["integrationMs"] = entry.spectrum.params.integrationTimeMs;
        capture["averaging"] = entry.spectrum.params.averaging;
        capture["points"] = entry.spectrum.counts.size();

        // Deterministic identification: detected peaks with their library
        // candidates, tolerance scaled to the capture's grid.
        const double tolerance =
            PeakMatcher::toleranceForGrid(entry.spectrum.wavelengthsNm);
        const auto& counts = entry.spectrum.counts;
        const auto [minIt, maxIt] = std::minmax_element(counts.cbegin(), counts.cend());
        const double prominence = std::max(0.05 * (*maxIt - *minIt), 1e-6);
        const auto peaks = SpectroAnalysis::findPeaks(entry.spectrum, prominence, 5.0);

        QJsonArray peakArray;
        for (const auto& peak : peaks)
            peakArray.append(QJsonObject{
                {"wavelengthNm", QString::number(peak.wavelengthNm, 'f', 1).toDouble()},
                {"value", QString::number(peak.value, 'f', 1).toDouble()}});
        capture["peaks"] = peakArray;

        QJsonArray candidateArray;
        for (const auto& candidate : PeakMatcher::match(peaks, tolerance)) {
            QJsonArray matchedArray;
            for (const auto& [peakNm, line] : candidate.matchedPeaks)
                matchedArray.append(QJsonObject{
                    {"peakNm", QString::number(peakNm, 'f', 1).toDouble()},
                    {"lineNm", line.wavelengthNm},
                    {"line", line.label}});
            candidateArray.append(QJsonObject{
                {"species", candidate.species},
                {"score", QString::number(candidate.score, 'f', 2).toDouble()},
                {"matches", matchedArray}});
        }
        capture["candidates"] = candidateArray;
        captureArray.append(capture);
    }
    root["captures"] = captureArray;

    QJsonArray measurementArray;
    for (const MeasurementRecord& m : measurements)
        measurementArray.append(QJsonObject{
            {"type", m.type},
            {"value", m.value},
            {"unit", m.unit},
            {"summary", QJsonDocument::fromJson(m.summary.toUtf8()).object()}});
    root["measurements"] = measurementArray;

    QJsonArray detectionArray;
    for (const Detection& d : detections)
        detectionArray.append(QJsonObject{
            {"label", d.label},
            {"confidence", QString::number(double(d.confidence), 'f', 2).toDouble()}});
    root["cameraDetections"] = detectionArray;

    return root;
}

} // namespace ReportContextBuilder
