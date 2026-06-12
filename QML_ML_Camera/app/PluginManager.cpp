#include "PluginManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QLibrary>
#include <QPluginLoader>
#include <QSettings>

#include "FrameProcessor.h"
#include "StorageLocations.h"
#include "logger.h"

PluginManager::PluginManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

PluginManager::~PluginManager()
{
    for (const PluginEntry& entry : m_plugins) {
        if (entry.loader) {
            entry.loader->unload();
            delete entry.loader;
        }
    }
}

void PluginManager::loadPlugins()
{
    // Candidate plugin directories, in priority order. On a plain layout the
    // plugins sit next to the executable; inside a macOS .app bundle the
    // executable is at Contents/MacOS, so "../PlugIns" is the standard slot and
    // "../../../plugins" reaches the build tree's app/plugins beside the bundle.
    // Finally the app-data plugins folder lets a user drop in a library.
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList dirs;
    dirs << QDir::cleanPath(appDir + "/plugins");
    dirs << QDir::cleanPath(appDir + "/../PlugIns");
    dirs << QDir::cleanPath(appDir + "/../../../plugins");
    dirs << QDir::cleanPath(StorageLocations::root() + "/plugins");

    QStringList scanned;
    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }
        const QStringList files = dir.entryList(QDir::Files);
        for (const QString& file : files) {
            const QString absolute = dir.absoluteFilePath(file);
            if (!QLibrary::isLibrary(absolute) || scanned.contains(absolute)) {
                continue;
            }
            scanned << absolute;
            tryLoad(absolute);
        }
    }
    qDebug(logInfo()) << "PluginManager: loaded" << m_plugins.size() << "processor(s).";
}

void PluginManager::tryLoad(const QString& filePath)
{
    auto* loader = new QPluginLoader(filePath);

    // Validate the interface IID from metadata before instantiating, so a
    // foreign library in the folder is skipped cheaply and without crashing.
    const QString iid = loader->metaData().value("IID").toString();
    if (iid != QLatin1String(FrameProcessor_iid)) {
        qDebug(logInfo()) << "PluginManager: skipping non-processor library:"
                          << filePath << "(iid:" << iid << ")";
        delete loader;
        return;
    }

    QObject* instance = loader->instance();
    auto* processor = qobject_cast<FrameProcessor*>(instance);
    if (!processor) {
        qWarning(logWarning()) << "PluginManager: not a FrameProcessor:" << filePath
                               << loader->errorString();
        loader->unload();
        delete loader;
        return;
    }

    QString error;
    if (!processor->initialize(&error)) {
        qWarning(logWarning()) << "PluginManager: initialize failed for"
                               << processor->name() << ":" << error;
        loader->unload();
        delete loader;
        return;
    }

    PluginEntry entry;
    entry.loader = loader;
    entry.processor = processor;
    entry.name = processor->name();
    entry.description = processor->description();

    QSettings settings;
    entry.enabled = settings.value("processors/" + entry.name + "/enabled", false).toBool();

    m_plugins.append(entry);
    qDebug(logInfo()) << "PluginManager: registered processor:" << entry.name
                      << "enabled:" << entry.enabled;
}

int PluginManager::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_plugins.size();
}

QVariant PluginManager::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_plugins.size()) {
        return QVariant();
    }
    const PluginEntry& entry = m_plugins.at(index.row());
    switch (role) {
        case NameRole:        return entry.name;
        case DescriptionRole: return entry.description;
        case EnabledRole:     return entry.enabled;
        default:              return QVariant();
    }
}

QHash<int, QByteArray> PluginManager::roleNames() const
{
    return {
        { NameRole,        "name" },
        { DescriptionRole, "description" },
        { EnabledRole,     "enabled" },
    };
}

void PluginManager::setEnabled(int row, bool enabled)
{
    if (row < 0 || row >= m_plugins.size() || m_plugins.at(row).enabled == enabled) {
        return;
    }
    m_plugins[row].enabled = enabled;

    QSettings settings;
    settings.setValue("processors/" + m_plugins.at(row).name + "/enabled", enabled);

    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, { EnabledRole });
    emit enabledProcessorsChanged(enabledProcessors());
    qDebug(logInfo()) << "PluginManager: processor" << m_plugins.at(row).name
                      << (enabled ? "enabled" : "disabled");
}

QList<FrameProcessor*> PluginManager::enabledProcessors() const
{
    QList<FrameProcessor*> result;
    for (const PluginEntry& entry : m_plugins) {
        if (entry.enabled) {
            result.append(entry.processor);
        }
    }
    return result;
}
