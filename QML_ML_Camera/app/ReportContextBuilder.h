#ifndef REPORTCONTEXTBUILDER_H
#define REPORTCONTEXTBUILDER_H

#include <QJsonObject>
#include <QVector>

#include "Detection.h"
#include "MeasurementDAO.h"
#include "SessionDAO.h"
#include "SpectrumDAO.h"

// Builds the single JSON document a report is generated from: every factual
// claim available to the language model comes from here — peak tables with
// deterministic PeakMatcher candidates, integration settings, radiation
// stats, camera detections. Pure function of its inputs (golden-file
// tested); the caller assembles the inputs from the DAOs and live services.
// The JSON is stored verbatim with each report, making reports auditable.
namespace ReportContextBuilder {

QJsonObject build(const SessionRecord& session,
                  const QVector<SpectrumEntry>& captures,
                  const QVector<MeasurementRecord>& measurements,
                  const QVector<Detection>& detections,
                  const QString& instrumentName);

} // namespace ReportContextBuilder

#endif // REPORTCONTEXTBUILDER_H
