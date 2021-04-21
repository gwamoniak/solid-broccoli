#include "LoggerDAO.h"

#include <QSqlQuery>
#include <QSqlError>

#include "logger.h"
#include "DatabaseManager.h"

LoggerDAO::LoggerDAO(QSqlDatabase &_database):
         m_sqlDB(_database)
{

}

void LoggerDAO::init() const
{

    if (!m_sqlDB.tables().contains(TABLE)) {
        QSqlQuery query(m_sqlDB);
        query.exec(QString("CREATE TABLE " + TABLE + " ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          + TABLE_TIME    + " VARCHAR(255)    NOT NULL,"
                          + TABLE_TYPE    + " VARCHAR(255)    NOT NULL,"
                          + TABLE_MESSAGE + " VARCHAR(255)    NOT NULL"
                           " )"
                                                                   ));
        DatabaseManager::debugQuery(query);
    }
}

void LoggerDAO::insertIntoTable(const QVariantList &data) const
{

    QSqlQuery query(m_sqlDB);
    query.prepare(QString("INSERT INTO " + TABLE + " ( " + TABLE_TIME +", "
                  + TABLE_TYPE + ", "
                  + TABLE_MESSAGE + " ) "
                          "VALUES (:Time, :Type, :Message)"));
    query.bindValue(":Time",       data[0].toString());
    query.bindValue(":Type",       data[1].toString());
    query.bindValue(":Message",    data[2].toString());

    if(!query.exec()){
        qDebug(logCritical()) << "error insert into " << TABLE;
        qDebug(logCritical()) << query.lastError().text();

    }

}


