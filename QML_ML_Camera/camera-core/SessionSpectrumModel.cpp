#include "SessionSpectrumModel.h"

#include <QFileInfo>
#include <QVariantMap>

#include "DatabaseManager.h"

SessionSpectrumModel::SessionSpectrumModel(DatabaseManager& db, QObject* parent)
    : QAbstractListModel(parent)
    , m_sqlDB(db)
{
}

void SessionSpectrumModel::setSessionId(int sessionId)
{
    beginResetModel();
    m_sessionId = sessionId;
    m_entries = sessionId >= 0 ? m_sqlDB.m_spectrumDao.spectra(sessionId)
                               : QVector<SpectrumEntry>();
    endResetModel();
}

void SessionSpectrumModel::rename(int row, const QString& name)
{
    if (row < 0 || row >= m_entries.size())
        return;
    m_sqlDB.m_spectrumDao.rename(m_entries[row].id, name);
    m_entries[row].name = name;
    emit dataChanged(index(row), index(row));
}

bool SessionSpectrumModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count <= 0 || row + count > m_entries.size())
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_sqlDB.m_spectrumDao.removeSpectrum(m_entries[row].id);
        m_entries.removeAt(row);
    }
    endRemoveRows();
    return true;
}

int SessionSpectrumModel::addSpectrum(int sessionId, const Spectrum& spectrum,
                                      const QString& name, const QString& tags)
{
    const int id = m_sqlDB.m_spectrumDao.addSpectrum(sessionId, spectrum, name, tags);
    if (sessionId == m_sessionId)
        setSessionId(m_sessionId);  // reload the visible list
    return id;
}

QVariantList SessionSpectrumModel::sessionVideos(int sessionId) const
{
    QVariantList list;
    for (const SessionVideoRecord& video : m_sqlDB.m_sessionDao.videos(sessionId)) {
        list.append(QVariantMap{
            {QStringLiteral("name"), QFileInfo(video.filepath).fileName()},
            {QStringLiteral("path"), video.filepath},
            {QStringLiteral("durationMs"), video.durationMs},
        });
    }
    return list;
}

QVariantList SessionSpectrumModel::sessionMeasurements(int sessionId) const
{
    QVariantList list;
    for (const MeasurementRecord& m : m_sqlDB.m_measurementDao.measurements(sessionId)) {
        list.append(QVariantMap{
            {QStringLiteral("id"), m.id},
            {QStringLiteral("type"), m.type},
            {QStringLiteral("value"), m.value},
            {QStringLiteral("unit"), m.unit},
            {QStringLiteral("createdUtc"), m.createdUtc},
            {QStringLiteral("summary"), m.summary},
        });
    }
    return list;
}

SpectrumEntry SessionSpectrumModel::entryById(int id) const
{
    return m_sqlDB.m_spectrumDao.spectrumById(id);
}

int SessionSpectrumModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant SessionSpectrumModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    const SpectrumEntry& entry = m_entries[index.row()];
    switch (role) {
    case IdRole:      return entry.id;
    case NameRole:    return entry.name;
    case KindRole:    return SpectrumDAO::kindToString(entry.spectrum.kind);
    case CreatedRole: return entry.createdUtc;
    case TagsRole:    return entry.tags;
    }
    return {};
}

QHash<int, QByteArray> SessionSpectrumModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {NameRole, "name"},
        {KindRole, "kind"},
        {CreatedRole, "created"},
        {TagsRole, "tags"},
    };
}
