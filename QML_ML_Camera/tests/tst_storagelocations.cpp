#include <QtTest>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include "StorageLocations.h"

// Verifies that every storage path is absolute, lives under one root, and is
// created on demand. setRootForTesting redirects the root into a QTemporaryDir
// so the test never touches the real app-data gallery.
class TestStorageLocations : public QObject
{
    Q_OBJECT
private slots:
    void rootIsRedirectedAndCreated();
    void subdirsAreAbsoluteUnderRootAndExist();
    void databasePathIsUnderRoot();
};

void TestStorageLocations::rootIsRedirectedAndCreated()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString base = tmp.path() + "/gallery-root";
    StorageLocations::setRootForTesting(base);

    const QString root = StorageLocations::root();
    QCOMPARE(QFileInfo(root).absoluteFilePath(), QFileInfo(base).absoluteFilePath());
    QVERIFY(QFileInfo(root).isAbsolute());
    QVERIFY(QDir(root).exists());
}

void TestStorageLocations::subdirsAreAbsoluteUnderRootAndExist()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    StorageLocations::setRootForTesting(tmp.path());
    const QString root = QFileInfo(StorageLocations::root()).absoluteFilePath();

    const QStringList dirs = {
        StorageLocations::picturesDir(),
        StorageLocations::recordingsDir(),
        StorageLocations::thumbnailsDir(),
        StorageLocations::logsDir(),
    };
    for (const QString& dir : dirs) {
        QVERIFY2(QFileInfo(dir).isAbsolute(), qPrintable(dir));
        QVERIFY2(QFileInfo(dir).absoluteFilePath().startsWith(root), qPrintable(dir));
        QVERIFY2(QDir(dir).exists(), qPrintable(dir));
    }
}

void TestStorageLocations::databasePathIsUnderRoot()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    StorageLocations::setRootForTesting(tmp.path());
    const QString root = QFileInfo(StorageLocations::root()).absoluteFilePath();

    const QString db = StorageLocations::databasePath();
    QVERIFY(QFileInfo(db).isAbsolute());
    QVERIFY(QFileInfo(db).absoluteFilePath().startsWith(root));
}

QTEST_GUILESS_MAIN(TestStorageLocations)
#include "tst_storagelocations.moc"
