#include <QtTest>
#include <QSignalSpy>
#include <QUrl>

#include "DatabaseManager.h"
#include "AlbumModel.h"
#include "PictureModel.h"

// Drives PictureModel over an in-memory database. Pictures are scoped to the
// model's currently-loaded album; removing that album cascades to its pictures
// via the AlbumModel::rowsRemoved -> PictureModel::deletePicturesForAlbum wiring.
class TestPictureModel : public QObject
{
    Q_OBJECT
private slots:
    void addPictureInsertsRow();
    void dataExposesUrlAndName();
    void removeRowsDeletes();
    void picturesCascadeDeleteWithAlbum();

private:
    static int makeAlbum(AlbumModel& albums, const QString& name);
};

int TestPictureModel::makeAlbum(AlbumModel& albums, const QString& name)
{
    albums.addAlbumFromName(name);
    const int row = albums.rowCount() - 1;
    return albums.data(albums.index(row), AlbumModel::IdRole).toInt();
}

void TestPictureModel::addPictureInsertsRow()
{
    DatabaseManager db(":memory:");
    AlbumModel albums(db);
    PictureModel pictures(db, albums);
    pictures.setAlbumId(makeAlbum(albums, "Holiday"));

    QSignalSpy inserted(&pictures, &QAbstractItemModel::rowsInserted);
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/photo.jpg"));

    QCOMPARE(pictures.rowCount(), 1);
    QCOMPARE(inserted.count(), 1);
}

void TestPictureModel::dataExposesUrlAndName()
{
    DatabaseManager db(":memory:");
    AlbumModel albums(db);
    PictureModel pictures(db, albums);
    pictures.setAlbumId(makeAlbum(albums, "Holiday"));
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/photo.jpg"));

    const QModelIndex idx = pictures.index(0);
    QCOMPARE(pictures.data(idx, PictureModel::FilePathRole).toString(),
             QStringLiteral("/tmp/photo.jpg"));
    QCOMPARE(pictures.data(idx, Qt::DisplayRole).toString(),
             QStringLiteral("photo.jpg"));
}

void TestPictureModel::removeRowsDeletes()
{
    DatabaseManager db(":memory:");
    AlbumModel albums(db);
    PictureModel pictures(db, albums);
    const int albumId = makeAlbum(albums, "Holiday");
    pictures.setAlbumId(albumId);
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/a.jpg"));
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/b.jpg"));

    QSignalSpy removed(&pictures, &QAbstractItemModel::rowsRemoved);
    QVERIFY(pictures.removeRows(0, 1));

    QCOMPARE(pictures.rowCount(), 1);
    QCOMPARE(removed.count(), 1);
    QCOMPARE(db.pictureDao().picturesForAlbum(albumId)->size(), size_t(1));
}

void TestPictureModel::picturesCascadeDeleteWithAlbum()
{
    DatabaseManager db(":memory:");
    AlbumModel albums(db);
    PictureModel pictures(db, albums);
    const int albumId = makeAlbum(albums, "Holiday");
    pictures.setAlbumId(albumId);
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/a.jpg"));
    pictures.addPictureFromUrl(QUrl::fromLocalFile("/tmp/b.jpg"));
    QCOMPARE(db.pictureDao().picturesForAlbum(albumId)->size(), size_t(2));

    // Removing the album row triggers the cascade for the loaded album.
    QVERIFY(albums.removeRows(0, 1));

    QCOMPARE(pictures.rowCount(), 0);
    QCOMPARE(db.pictureDao().picturesForAlbum(albumId)->size(), size_t(0));
}

QTEST_GUILESS_MAIN(TestPictureModel)
#include "tst_picturemodel.moc"
