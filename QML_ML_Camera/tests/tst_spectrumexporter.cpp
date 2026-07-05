#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

#include <cmath>

#include "SpectrumExporter.h"
#include "SpectrumSynthesizer.h"

class TstSpectrumExporter : public QObject
{
    Q_OBJECT

private:
    static Spectrum makeSpectrum()
    {
        Spectrum s;
        s.wavelengthsNm = SpectrumSynthesizer::standardGrid();
        s.counts = SpectrumSynthesizer::mercuryLamp(s.wavelengthsNm, 2.0);
        for (double& c : s.counts)
            c = c * 37000.0 + 42.5;
        return s;
    }

    static SpectrumExportMetadata makeMeta()
    {
        return {QStringLiteral("mercury demo"), QStringLiteral("sample"),
                QStringLiteral("2026-07-03T14:02:11Z"), QStringLiteral("demo,mercury"),
                100, 4};
    }

private slots:
    // Export at 10 significant digits, parse back, compare within 1e-6
    // relative — the loop that proves exported data is usable data.
    void csvRoundTrip()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("demo.csv");
        const Spectrum original = makeSpectrum();

        QString error;
        QVERIFY2(SpectrumExporter::exportCsv(original, makeMeta(), path, &error),
                 qPrintable(error));

        Spectrum imported;
        QVERIFY2(SpectrumExporter::importCsv(path, &imported, &error), qPrintable(error));
        QCOMPARE(imported.counts.size(), original.counts.size());
        for (int i = 0; i < original.counts.size(); ++i) {
            QVERIFY(std::abs(imported.wavelengthsNm[i] - original.wavelengthsNm[i])
                    <= 1e-6 * std::abs(original.wavelengthsNm[i]));
            QVERIFY(std::abs(imported.counts[i] - original.counts[i])
                    <= 1e-6 * std::max(std::abs(original.counts[i]), 1.0));
        }
    }

    void csvCarriesMetadataHeader()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("meta.csv");
        QVERIFY(SpectrumExporter::exportCsv(makeSpectrum(), makeMeta(), path));

        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString head = QString::fromUtf8(file.read(400));
        QVERIFY(head.contains("name: mercury demo"));
        QVERIFY(head.contains("integration_ms: 100"));
        QVERIFY(head.contains("tags: demo,mercury"));
        QVERIFY(head.contains("wavelength_nm,counts"));
    }

    void jsonCarriesEverything()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("demo.json");
        const Spectrum original = makeSpectrum();
        QVERIFY(SpectrumExporter::exportJson(original, makeMeta(), path));

        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QJsonObject object = QJsonDocument::fromJson(file.readAll()).object();
        QCOMPARE(object.value("name").toString(), QStringLiteral("mercury demo"));
        QCOMPARE(object.value("integration_ms").toInt(), 100);
        QCOMPARE(object.value("wavelengths_nm").toArray().size(), original.counts.size());
        QVERIFY(std::abs(object.value("counts").toArray().first().toDouble()
                         - original.counts.first()) < 1e-9);
    }

    void badPathsReportErrors()
    {
        QString error;
        QVERIFY(!SpectrumExporter::exportCsv(makeSpectrum(), makeMeta(),
                                             "/nonexistent/dir/x.csv", &error));
        QVERIFY(!error.isEmpty());

        Spectrum out;
        QVERIFY(!SpectrumExporter::importCsv("/nonexistent/dir/x.csv", &out, &error));

        QVERIFY(!SpectrumExporter::exportCsv(Spectrum(), makeMeta(), "/tmp/y.csv", &error));
    }
};

QTEST_GUILESS_MAIN(TstSpectrumExporter)
#include "tst_spectrumexporter.moc"
