#include <QSqlDatabase>
#include <QtTest>

#include "DatabaseManager.h"

// Report persistence: exact round trip (including the audit context JSON)
// and the explicit cascade on session delete.
class TstReportDao : public QObject
{
    Q_OBJECT

private slots:
    void roundTripAndCascade()
    {
        DatabaseManager db(":memory:");
        QVERIFY(db.database().tables().contains("reports"));

        const int sessionId = db.sessionDao().addSession("Reported");
        const int id = db.reportDao().addReport(sessionId, "gemma-3-4b.gguf",
                                                "report_v1", R"({"captures":[]})",
                                                "## Summary\nAll good.");
        QVERIFY(id > 0);

        const auto reports = db.reportDao().reports(sessionId);
        QCOMPARE(reports.size(), 1);
        QCOMPARE(reports.first().modelName, QStringLiteral("gemma-3-4b.gguf"));
        QCOMPARE(reports.first().promptVersion, QStringLiteral("report_v1"));
        QCOMPARE(reports.first().contextJson, QStringLiteral(R"({"captures":[]})"));
        QCOMPARE(reports.first().contentMd, QStringLiteral("## Summary\nAll good."));

        const ReportRecord byId = db.reportDao().reportById(id);
        QCOMPARE(byId.sessionId, sessionId);
        QVERIFY(db.reportDao().reportById(99999).id < 0);

        db.sessionDao().removeSession(sessionId);
        QVERIFY(db.reportDao().reports(sessionId).isEmpty());
    }

    void removeSingleReport()
    {
        DatabaseManager db(":memory:");
        const int sessionId = db.sessionDao().addSession("S");
        const int id = db.reportDao().addReport(sessionId, "m", "v", "{}", "text");
        db.reportDao().removeReport(id);
        QVERIFY(db.reportDao().reports(sessionId).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TstReportDao)
#include "tst_reportdao.moc"
