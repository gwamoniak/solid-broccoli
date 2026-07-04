#include "SessionDAO.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

SessionDAO::SessionDAO(QSqlDatabase& database)
    : m_sqlDB(database)
{
}

void SessionDAO::init() const
{
    if (!m_sqlDB.tables().contains("sessions")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE sessions (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "name TEXT, created_utc TEXT, notes TEXT)");
        DatabaseManager::debugQuery(query);
    }
    if (!m_sqlDB.tables().contains("session_videos")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE session_videos (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "session_id INTEGER, filepath TEXT, duration_ms INTEGER)");
        DatabaseManager::debugQuery(query);
    }
}

void SessionDAO::addVideo(int sessionId, const QString& filepath, qint64 durationMs) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO session_videos (session_id, filepath, duration_ms) "
                  "VALUES (:session, :path, :duration)");
    query.bindValue(":session", sessionId);
    query.bindValue(":path", filepath);
    query.bindValue(":duration", durationMs);
    query.exec();
    DatabaseManager::debugQuery(query);
}

QVector<SessionVideoRecord> SessionDAO::videos(int sessionId) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM session_videos WHERE session_id = (:session) ORDER BY id");
    query.bindValue(":session", sessionId);
    query.exec();
    DatabaseManager::debugQuery(query);

    QVector<SessionVideoRecord> list;
    while (query.next()) {
        SessionVideoRecord record;
        record.id = query.value("id").toInt();
        record.filepath = query.value("filepath").toString();
        record.durationMs = query.value("duration_ms").toLongLong();
        list.append(record);
    }
    return list;
}

int SessionDAO::addSession(const QString& name) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO sessions (name, created_utc, notes) "
                  "VALUES (:name, :created, :notes)");
    query.bindValue(":name", name);
    query.bindValue(":created", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":notes", QString());
    query.exec();
    DatabaseManager::debugQuery(query);
    return query.lastInsertId().toInt();
}

void SessionDAO::updateName(int id, const QString& name) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("UPDATE sessions SET name = (:name) WHERE id = (:id)");
    query.bindValue(":name", name);
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}

void SessionDAO::removeSession(int id) const
{
    QSqlQuery cascade(m_sqlDB);
    cascade.prepare("DELETE FROM spectra WHERE session_id = (:id)");
    cascade.bindValue(":id", id);
    cascade.exec();
    DatabaseManager::debugQuery(cascade);

    QSqlQuery videoCascade(m_sqlDB);
    videoCascade.prepare("DELETE FROM session_videos WHERE session_id = (:id)");
    videoCascade.bindValue(":id", id);
    videoCascade.exec();
    DatabaseManager::debugQuery(videoCascade);

    QSqlQuery measurementCascade(m_sqlDB);
    measurementCascade.prepare("DELETE FROM measurements WHERE session_id = (:id)");
    measurementCascade.bindValue(":id", id);
    measurementCascade.exec();
    DatabaseManager::debugQuery(measurementCascade);

    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM sessions WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}

QVector<SessionRecord> SessionDAO::sessions() const
{
    QSqlQuery query(m_sqlDB);
    query.exec("SELECT s.id, s.name, s.created_utc, s.notes, COUNT(p.id) AS cnt "
               "FROM sessions s LEFT JOIN spectra p ON p.session_id = s.id "
               "GROUP BY s.id ORDER BY s.id DESC");
    DatabaseManager::debugQuery(query);

    QVector<SessionRecord> list;
    while (query.next()) {
        SessionRecord record;
        record.id = query.value("id").toInt();
        record.name = query.value("name").toString();
        record.createdUtc = query.value("created_utc").toString();
        record.notes = query.value("notes").toString();
        record.captureCount = query.value("cnt").toInt();
        list.append(record);
    }
    return list;
}
