#include <QtTest>
#include <QSignalSpy>

#include "DatabaseManager.h"
#include "AlbumModel.h"

// Drives AlbumModel over an in-memory database: insertion, rename, and removal
// each update the row count, emit the expected model signals, and round-trip
// through the DAO.
class TestAlbumModel : public QObject
{
    Q_OBJECT
private slots:
    void startsEmpty();
    void addAlbumInsertsRowAndPersistsId();
    void renameChangesNameAndEmits();
    void removeRowsDeletes();
    void removeRowsRejectsOutOfRange();
};

void TestAlbumModel::startsEmpty()
{
    DatabaseManager db(":memory:");
    AlbumModel model(db);
    QCOMPARE(model.rowCount(), 0);
}

void TestAlbumModel::addAlbumInsertsRowAndPersistsId()
{
    DatabaseManager db(":memory:");
    AlbumModel model(db);
    QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);

    model.addAlbumFromName("Holiday");

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(inserted.count(), 1);
    QCOMPARE(model.data(model.index(0), AlbumModel::NameRole).toString(),
             QStringLiteral("Holiday"));
    QVERIFY(model.data(model.index(0), AlbumModel::IdRole).toInt() > 0);
}

void TestAlbumModel::renameChangesNameAndEmits()
{
    DatabaseManager db(":memory:");
    AlbumModel model(db);
    model.addAlbumFromName("Holiday");
    QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

    model.rename(0, "Trip");

    QCOMPARE(changed.count(), 1);
    QCOMPARE(model.data(model.index(0), AlbumModel::NameRole).toString(),
             QStringLiteral("Trip"));

    // A fresh model over the same database sees the persisted rename.
    AlbumModel reloaded(db);
    QCOMPARE(reloaded.data(reloaded.index(0), AlbumModel::NameRole).toString(),
             QStringLiteral("Trip"));
}

void TestAlbumModel::removeRowsDeletes()
{
    DatabaseManager db(":memory:");
    AlbumModel model(db);
    model.addAlbumFromName("Holiday");
    QSignalSpy removed(&model, &QAbstractItemModel::rowsRemoved);

    QVERIFY(model.removeRows(0, 1));

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(removed.count(), 1);
    AlbumModel reloaded(db);
    QCOMPARE(reloaded.rowCount(), 0);
}

void TestAlbumModel::removeRowsRejectsOutOfRange()
{
    DatabaseManager db(":memory:");
    AlbumModel model(db);
    QVERIFY(!model.removeRows(0, 1)); // empty model
    model.addAlbumFromName("Holiday");
    QVERIFY(!model.removeRows(1, 1)); // past the end
    QVERIFY(!model.removeRows(0, 5)); // count overruns
    QCOMPARE(model.rowCount(), 1);
}

QTEST_GUILESS_MAIN(TestAlbumModel)
#include "tst_albummodel.moc"
