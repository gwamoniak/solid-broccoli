#include "ReportDAO.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

namespace {
ReportRecord fromQuery(const QSqlQuery& query)
{
    ReportRecord record;
    record.id = query.value("id").toInt();
    record.sessionId = query.value("session_id").toInt();
    record.createdUtc = query.value("created_utc").toString();
    record.modelName = query.value("model_name").toString();
    record.promptVersion = query.value("prompt_version").toString();
    record.contextJson = query.value("context_json").toString();
    record.contentMd = query.value("content_md").toString();
    return record;
}
}

ReportDAO::ReportDAO(QSqlDatabase& database)
    : m_sqlDB(database)
{
}

void ReportDAO::init() const
{
    if (!m_sqlDB.tables().contains("reports")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE reports (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "session_id INTEGER, created_utc TEXT, model_name TEXT, "
                   "prompt_version TEXT, context_json TEXT, content_md TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

int ReportDAO::addReport(int sessionId, const QString& modelName,
                         const QString& promptVersion, const QString& contextJson,
                         const QString& contentMd) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO reports (session_id, created_utc, model_name, "
                  "prompt_version, context_json, content_md) VALUES (:session, "
                  ":created, :model, :prompt, :context, :content)");
    query.bindValue(":session", sessionId);
    query.bindValue(":created", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":model", modelName);
    query.bindValue(":prompt", promptVersion);
    query.bindValue(":context", contextJson);
    query.bindValue(":content", contentMd);
    query.exec();
    DatabaseManager::debugQuery(query);
    return query.lastInsertId().toInt();
}

QVector<ReportRecord> ReportDAO::reports(int sessionId) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM reports WHERE session_id = (:session) ORDER BY id");
    query.bindValue(":session", sessionId);
    query.exec();
    DatabaseManager::debugQuery(query);

    QVector<ReportRecord> list;
    while (query.next())
        list.append(fromQuery(query));
    return list;
}

ReportRecord ReportDAO::reportById(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM reports WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
    return query.next() ? fromQuery(query) : ReportRecord{};
}

void ReportDAO::removeReport(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM reports WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}
