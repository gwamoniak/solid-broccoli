#include <QJsonArray>
#include <QJsonDocument>
#include <QtTest>

#include "ReportContextBuilder.h"
#include "SpectrumSynthesizer.h"

// The report context is the language model's entire world: it must contain
// the deterministic identifications and nothing surprising. Pure builder,
// fixed inputs, stable output (QJsonObject orders keys alphabetically).
class TstReportContext : public QObject
{
    Q_OBJECT

private slots:
    void emptySessionProducesExactSkeleton()
    {
        SessionRecord session;
        session.name = "Empty";
        session.createdUtc = "2026-07-04T10:00:00Z";

        const QJsonObject context =
            ReportContextBuilder::build(session, {}, {}, {}, "Test instrument");
        const QString compact =
            QString::fromUtf8(QJsonDocument(context).toJson(QJsonDocument::Compact));
        // Golden skeleton: field order is deterministic.
        QCOMPARE(compact,
                 QStringLiteral(
                     "{\"cameraDetections\":[],\"captures\":[],"
                     "\"instrument\":\"Test instrument\",\"measurements\":[],"
                     "\"session\":{\"createdUtc\":\"2026-07-04T10:00:00Z\","
                     "\"name\":\"Empty\",\"notes\":\"\"}}"));
    }

    void mercuryCaptureCarriesPeaksAndCandidates()
    {
        SessionRecord session;
        session.name = "Hg bench";

        SpectrumEntry entry;
        entry.name = "Sample 1";
        entry.spectrum.wavelengthsNm = SpectrumSynthesizer::standardGrid();
        entry.spectrum.counts =
            SpectrumSynthesizer::mercuryLamp(entry.spectrum.wavelengthsNm, 2.0);
        for (double& c : entry.spectrum.counts)
            c *= 50000.0;
        entry.spectrum.params = {100, 2};
        entry.spectrum.kind = Spectrum::Kind::Sample;

        MeasurementRecord dose;
        dose.type = "geiger_dose";
        dose.value = 0.32;
        dose.unit = "uSv/h";
        dose.summary = R"({"avgCpm":56.0})";

        Detection detection;
        detection.label = "cup";
        detection.confidence = 0.93f;

        const QJsonObject context = ReportContextBuilder::build(
            session, {entry}, {dose}, {detection}, "Simulated UV-Vis (mercury lamp)");

        const QJsonObject capture =
            context["captures"].toArray().first().toObject();
        QCOMPARE(capture["kind"].toString(), QStringLiteral("sample"));
        QCOMPARE(capture["integrationMs"].toInt(), 100);
        QVERIFY(capture["peaks"].toArray().size() >= 4);  // >=4 of 5 Hg lines

        // The deterministic identification names mercury first.
        const QJsonArray candidates = capture["candidates"].toArray();
        QVERIFY(!candidates.isEmpty());
        QCOMPARE(candidates.first().toObject()["species"].toString(),
                 QStringLiteral("Hg"));

        const QJsonObject measurement =
            context["measurements"].toArray().first().toObject();
        QCOMPARE(measurement["unit"].toString(), QStringLiteral("uSv/h"));
        QCOMPARE(measurement["summary"].toObject()["avgCpm"].toDouble(), 56.0);

        QCOMPARE(context["cameraDetections"].toArray().first()
                     .toObject()["label"].toString(),
                 QStringLiteral("cup"));
    }
};

QTEST_GUILESS_MAIN(TstReportContext)
#include "tst_reportcontext.moc"
