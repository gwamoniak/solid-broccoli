#include <QtTest>
#include <QSqlDatabase>
#include <QStringList>

#include "DatabaseManager.h"

// Exercises the de-singletoned DatabaseManager against an in-memory SQLite
// database: a good path opens and creates the schema; a path in a non-existent
// directory reports the connection closed instead of crashing.
class TestDatabaseManager : public QObject
{
    Q_OBJECT
private slots:
    void inMemoryOpensAndCreatesTables();
    void multipleInstancesCoexist();
    void badPathReportsClosed();
};

void TestDatabaseManager::inMemoryOpensAndCreatesTables()
{
    DatabaseManager db(":memory:");
    QVERIFY(db.isOpen());

    const QStringList tables = db.database().tables();
    QVERIFY2(tables.contains("albums"), qPrintable(tables.join(',')));
    QVERIFY2(tables.contains("pictures"), qPrintable(tables.join(',')));
    QVERIFY2(tables.contains("movies"), qPrintable(tables.join(',')));
}

void TestDatabaseManager::multipleInstancesCoexist()
{
    // Each manager uses a unique connection name, so two in-memory databases
    // can be open at once without clobbering each other.
    DatabaseManager a(":memory:");
    DatabaseManager b(":memory:");
    QVERIFY(a.isOpen());
    QVERIFY(b.isOpen());
    QVERIFY(a.database().connectionName() != b.database().connectionName());
}

void TestDatabaseManager::badPathReportsClosed()
{
    DatabaseManager db("/this/directory/does/not/exist/gallery.db");
    QVERIFY(!db.isOpen());
}

QTEST_GUILESS_MAIN(TestDatabaseManager)
#include "tst_databasemanager.moc"
