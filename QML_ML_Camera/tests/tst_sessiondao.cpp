#include <QSqlDatabase>
#include <QtTest>

#include "DatabaseManager.h"
#include "SessionSpectrumModel.h"
#include "SpectrumSynthesizer.h"

// Session/spectrum persistence against an in-memory database: blob round
// trips must be exact (raw float64), deletes must cascade, and the models
// must reflect the DAO state.
class TstSessionDao : public QObject
{
    Q_OBJECT

private:
    static Spectrum makeSpectrum(Spectrum::Kind kind)
    {
        Spectrum s;
        s.wavelengthsNm = SpectrumSynthesizer::as7265xGrid();
        s.counts = SpectrumSynthesizer::mercuryLamp(s.wavelengthsNm, 10.0);
        s.counts[0] = 0.123456789012345;  // exactness probe
        s.timestampMs = 1700000000000;
        s.params = {250, 4};
        s.kind = kind;
        return s;
    }

private slots:
    void tablesCreated()
    {
        DatabaseManager db(":memory:");
        QVERIFY(db.isOpen());
        QVERIFY(db.database().tables().contains("sessions"));
        QVERIFY(db.database().tables().contains("spectra"));
    }

    void blobRoundTripIsExact()
    {
        const QVector<double> values{0.0, -1.5, 3.14159265358979, 1e-300, 6.5e4};
        const QByteArray blob = SpectrumDAO::encodeDoubles(values);
        QCOMPARE(blob.size(), values.size() * int(sizeof(double)));
        QCOMPARE(SpectrumDAO::decodeDoubles(blob), values);
    }

    void addAndReadBackSpectrum()
    {
        DatabaseManager db(":memory:");
        const int sessionId = db.sessionDao().addSession("Test session");
        QVERIFY(sessionId > 0);

        const Spectrum original = makeSpectrum(Spectrum::Kind::Dark);
        const int spectrumId = db.spectrumDao().addSpectrum(sessionId, original,
                                                            "dark frame", "tag1,tag2");
        QVERIFY(spectrumId > 0);

        const auto entries = db.spectrumDao().spectra(sessionId);
        QCOMPARE(entries.size(), 1);
        const SpectrumEntry& entry = entries.first();
        QCOMPARE(entry.name, QStringLiteral("dark frame"));
        QCOMPARE(entry.tags, QStringLiteral("tag1,tag2"));
        QCOMPARE(entry.spectrum.kind, Spectrum::Kind::Dark);
        QCOMPARE(entry.spectrum.params.integrationTimeMs, 250);
        QCOMPARE(entry.spectrum.params.averaging, 4);
        QCOMPARE(entry.spectrum.wavelengthsNm, original.wavelengthsNm);  // bit-exact
        QCOMPARE(entry.spectrum.counts, original.counts);

        const SpectrumEntry byId = db.spectrumDao().spectrumById(spectrumId);
        QVERIFY(byId.isValid());
        QCOMPARE(byId.spectrum.counts, original.counts);
        QVERIFY(!db.spectrumDao().spectrumById(99999).isValid());
    }

    void removeSessionCascades()
    {
        DatabaseManager db(":memory:");
        const int sessionId = db.sessionDao().addSession("Doomed");
        db.spectrumDao().addSpectrum(sessionId, makeSpectrum(Spectrum::Kind::Sample), "a", "");
        db.spectrumDao().addSpectrum(sessionId, makeSpectrum(Spectrum::Kind::Sample), "b", "");
        QCOMPARE(db.sessionDao().sessions().first().captureCount, 2);

        db.sessionDao().removeSession(sessionId);
        QVERIFY(db.sessionDao().sessions().isEmpty());
        QVERIFY(db.spectrumDao().spectra(sessionId).isEmpty());
    }

    void videoLinksRoundTripAndCascade()
    {
        DatabaseManager db(":memory:");
        const int sessionId = db.sessionDao().addSession("With video");
        db.sessionDao().addVideo(sessionId, "/tmp/recording_001.mp4", 5250);

        const auto videos = db.sessionDao().videos(sessionId);
        QCOMPARE(videos.size(), 1);
        QCOMPARE(videos.first().filepath, QStringLiteral("/tmp/recording_001.mp4"));
        QCOMPARE(videos.first().durationMs, qint64(5250));

        db.sessionDao().removeSession(sessionId);
        QVERIFY(db.sessionDao().videos(sessionId).isEmpty());
    }

    void renameAndRemoveSpectrum()
    {
        DatabaseManager db(":memory:");
        const int sessionId = db.sessionDao().addSession("S");
        const int id = db.spectrumDao().addSpectrum(sessionId,
                                                    makeSpectrum(Spectrum::Kind::Sample),
                                                    "old", "");
        db.spectrumDao().rename(id, "new");
        QCOMPARE(db.spectrumDao().spectrumById(id).name, QStringLiteral("new"));

        db.spectrumDao().removeSpectrum(id);
        QVERIFY(!db.spectrumDao().spectrumById(id).isValid());
    }

    void scopedModelFollowsSession()
    {
        DatabaseManager db(":memory:");
        SessionSpectrumModel model(db);
        const int sessionA = db.sessionDao().addSession("A");
        const int sessionB = db.sessionDao().addSession("B");

        model.setSessionId(sessionA);
        QCOMPARE(model.rowCount(), 0);

        model.addSpectrum(sessionA, makeSpectrum(Spectrum::Kind::Sample), "inA", "");
        QCOMPARE(model.rowCount(), 1);  // visible session refreshed in place

        model.addSpectrum(sessionB, makeSpectrum(Spectrum::Kind::Sample), "inB", "");
        QCOMPARE(model.rowCount(), 1);  // other session untouched

        model.setSessionId(sessionB);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0), SessionSpectrumModel::NameRole).toString(),
                 QStringLiteral("inB"));

        QVERIFY(model.removeRows(0, 1));
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(TstSessionDao)
#include "tst_sessiondao.moc"
