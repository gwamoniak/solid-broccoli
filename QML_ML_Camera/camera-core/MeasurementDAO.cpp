#include "MeasurementDAO.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

MeasurementDAO::MeasurementDAO(QSqlDatabase& database)
    : m_sqlDB(database)
{
}

void MeasurementDAO::init() const
{
    if (!m_sqlDB.tables().contains("measurements")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE measurements (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "session_id INTEGER, created_utc TEXT, type TEXT, "
                   "value REAL, unit TEXT, summary TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

int MeasurementDAO::addMeasurement(int sessionId, const QString& type, double value,
                                   const QString& unit, const QString& summaryJson) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO measurements (session_id, created_utc, type, value, "
                  "unit, summary) VALUES (:session, :created, :type, :value, :unit, "
                  ":summary)");
    query.bindValue(":session", sessionId);
    query.bindValue(":created", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":type", type);
    query.bindValue(":value", value);
    query.bindValue(":unit", unit);
    query.bindValue(":summary", summaryJson);
    query.exec();
    DatabaseManager::debugQuery(query);
    return query.lastInsertId().toInt();
}

QVector<MeasurementRecord> MeasurementDAO::measurements(int sessionId) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM measurements WHERE session_id = (:session) ORDER BY id");
    query.bindValue(":session", sessionId);
    query.exec();
    DatabaseManager::debugQuery(query);

    QVector<MeasurementRecord> list;
    while (query.next()) {
        MeasurementRecord record;
        record.id = query.value("id").toInt();
        record.sessionId = query.value("session_id").toInt();
        record.createdUtc = query.value("created_utc").toString();
        record.type = query.value("type").toString();
        record.value = query.value("value").toDouble();
        record.unit = query.value("unit").toString();
        record.summary = query.value("summary").toString();
        list.append(record);
    }
    return list;
}

void MeasurementDAO::removeMeasurement(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM measurements WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}
