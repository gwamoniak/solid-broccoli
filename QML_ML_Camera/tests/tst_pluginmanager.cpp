#include <QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "PluginManager.h"
#include "FrameProcessor.h"

#ifndef GRAYSCALE_PLUGIN_DIR
#define GRAYSCALE_PLUGIN_DIR ""
#endif

// Exercises PluginManager against the real grayscale plugin built in this tree
// plus a deliberately broken library, proving discovery, IID validation, the
// enable toggle (persisted + signalled), and that a junk file is skipped
// without crashing.
class TestPluginManager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void discoversGrayscaleAndSkipsJunk();
    void enableTogglePersistsAndSignals();

private:
    QString writeBrokenLibrary(const QString& dir) const;
};

void TestPluginManager::initTestCase()
{
    // Isolate QSettings so persisted enable-state cannot leak into or out of
    // the real application's settings.
    QCoreApplication::setOrganizationName("SolidBroccoliTest");
    QCoreApplication::setApplicationName("PluginManagerTest");
    QVERIFY2(QString(GRAYSCALE_PLUGIN_DIR).length() > 0,
             "GRAYSCALE_PLUGIN_DIR define missing");
}

void TestPluginManager::init()
{
    QSettings().clear();
}

QString TestPluginManager::writeBrokenLibrary(const QString& dir) const
{
    // A file with a library suffix that is not a valid Qt plugin; QPluginLoader
    // must reject it (empty IID) without crashing.
#if defined(Q_OS_MAC)
    const QString name = "libbroken.dylib";
#elif defined(Q_OS_WIN)
    const QString name = "broken.dll";
#else
    const QString name = "libbroken.so";
#endif
    const QString path = QDir(dir).absoluteFilePath(name);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write("not a real shared library");
        f.close();
    }
    return path;
}

void TestPluginManager::discoversGrayscaleAndSkipsJunk()
{
    QTemporaryDir junkDir;
    QVERIFY(junkDir.isValid());
    writeBrokenLibrary(junkDir.path());

    PluginManager manager;
    manager.loadPlugins({ QString(GRAYSCALE_PLUGIN_DIR), junkDir.path() });

    QCOMPARE(manager.rowCount(), 1);
    QCOMPARE(manager.data(manager.index(0), PluginManager::NameRole).toString(),
             QStringLiteral("Grayscale"));
    QVERIFY(!manager.data(manager.index(0), PluginManager::DescriptionRole)
                 .toString().isEmpty());
    QVERIFY(!manager.data(manager.index(0), PluginManager::EnabledRole).toBool());
    QVERIFY(manager.enabledProcessors().isEmpty());
}

void TestPluginManager::enableTogglePersistsAndSignals()
{
    PluginManager manager;
    manager.loadPlugins({ QString(GRAYSCALE_PLUGIN_DIR) });
    QCOMPARE(manager.rowCount(), 1);

    QSignalSpy enabledSpy(&manager, &PluginManager::enabledProcessorsChanged);
    manager.setEnabled(0, true);

    QCOMPARE(enabledSpy.count(), 1);
    QVERIFY(manager.data(manager.index(0), PluginManager::EnabledRole).toBool());
    QCOMPARE(manager.enabledProcessors().size(), 1);

    FrameProcessor* proc = manager.enabledProcessors().first();
    QVERIFY(proc != nullptr);
    QCOMPARE(proc->name(), QStringLiteral("Grayscale"));

    // A fresh manager restores the persisted enabled state.
    PluginManager reloaded;
    reloaded.loadPlugins({ QString(GRAYSCALE_PLUGIN_DIR) });
    QVERIFY(reloaded.data(reloaded.index(0), PluginManager::EnabledRole).toBool());
}

QTEST_GUILESS_MAIN(TestPluginManager)
#include "tst_pluginmanager.moc"
