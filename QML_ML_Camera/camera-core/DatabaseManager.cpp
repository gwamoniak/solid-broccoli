#include "DatabaseManager.h"
#include "logger.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QStandardPaths>

void DatabaseManager::debugQuery(const QSqlQuery& _query)
{
    if (_query.lastError().type() == QSqlError::ErrorType::NoError) {
        qDebug(logInfo) << "Query OK:"  << _query.lastQuery();
    } else {
       qWarning(logWarning()) << "Query KO:" << _query.lastError().text();
       qWarning(logWarning()) << "Query text:" << _query.lastQuery();
    }
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager singleton; // call it once

    return singleton;
}

DatabaseManager::DatabaseManager(const QString& _path) :
    m_sqlDataBase(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"))),
    m_albumDao(*m_sqlDataBase),  
    m_pictureDao(*m_sqlDataBase),   
    m_loggerDao(*m_sqlDataBase)
{
    m_sqlDataBase->setDatabaseName(_path);

    bool openStatus = m_sqlDataBase->open();
    qDebug(logInfo) << "Database connection: " << (openStatus ? "OK" : "Error");

    m_albumDao.init();   
    m_pictureDao.init();   
    m_loggerDao.init();
}

DatabaseManager::~DatabaseManager()
{
    m_sqlDataBase->close();
}


