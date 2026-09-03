#include "SessionModel.h"

#include "DatabaseManager.h"

SessionModel::SessionModel(DatabaseManager& db, QObject* parent)
    : QAbstractListModel(parent)
    , m_sqlDB(db)
{
    m_sessions = m_sqlDB.sessionDao().sessions();
}

int SessionModel::addSessionFromName(const QString& name)
{
    const int id = m_sqlDB.sessionDao().addSession(name);
    refresh();
    return id;
}

void SessionModel::rename(int row, const QString& name)
{
    if (row < 0 || row >= m_sessions.size())
        return;
    m_sqlDB.sessionDao().updateName(m_sessions[row].id, name);
    m_sessions[row].name = name;
    emit dataChanged(index(row), index(row));
}

bool SessionModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count <= 0 || row + count > m_sessions.size())
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_sqlDB.sessionDao().removeSession(m_sessions[row].id);
        m_sessions.removeAt(row);
    }
    endRemoveRows();
    return true;
}

void SessionModel::refresh()
{
    beginResetModel();
    m_sessions = m_sqlDB.sessionDao().sessions();
    endResetModel();
}

int SessionModel::sessionIdAt(int row) const
{
    return (row >= 0 && row < m_sessions.size()) ? m_sessions[row].id : -1;
}

int SessionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_sessions.size();
}

QVariant SessionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sessions.size())
        return {};
    const SessionRecord& record = m_sessions[index.row()];
    switch (role) {
    case IdRole:      return record.id;
    case NameRole:    return record.name;
    case CreatedRole: return record.createdUtc;
    case CountRole:   return record.captureCount;
    }
    return {};
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {NameRole, "name"},
        {CreatedRole, "created"},
        {CountRole, "count"},
    };
}
