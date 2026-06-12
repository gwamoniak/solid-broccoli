#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QAbstractListModel>
#include <QList>

class FrameProcessor;
class QPluginLoader;

// Discovers frame-processor plugins at runtime and exposes them to QML as a
// list of (name, description, enabled) rows with a toggle. Enabled state is
// persisted in QSettings keyed by plugin name, so a processor the user turned
// on stays on across restarts. Whenever the enabled set changes it emits
// enabledProcessorsChanged so CameraService can rebuild its preview pipeline.
class PluginManager : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        EnabledRole,
    };

    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    // Scans <application dir>/plugins and the app-data plugins folder. Safe to
    // call once at startup; broken or non-conforming libraries are skipped.
    // extraDirs are scanned in addition to the standard locations; tests use it
    // to point at the build-tree plugin output without installing anything.
    void loadPlugins(const QStringList& extraDirs = QStringList());

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setEnabled(int row, bool enabled);

    // Enabled processors in row order; passed to CameraService for the worker.
    QList<FrameProcessor*> enabledProcessors() const;

signals:
    void enabledProcessorsChanged(const QList<FrameProcessor*>& processors);

private:
    struct PluginEntry {
        QPluginLoader* loader = nullptr;
        FrameProcessor* processor = nullptr;
        QString name;
        QString description;
        bool enabled = false;
    };

    void tryLoad(const QString& filePath);

    QList<PluginEntry> m_plugins;
};

#endif // PLUGINMANAGER_H
