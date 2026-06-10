#include "DatabaseManager.h"
#include "StorageLocations.h"
#include "logger.h"

#include <QAtomicInt>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

namespace {
QString nextConnectionName()
{
    static QAtomicInt counter(0);
    return QStringLiteral("solidbroccoli_%1").arg(counter.fetchAndAddRelaxed(1));
}
}

void DatabaseManager::debugQuery(const QSqlQuery& _query)
{
    if (_query.lastError().type() == QSqlError::ErrorType::NoError) {
        qDebug(logInfo) << "Query OK:"  << _query.lastQuery();
    } else {
       qWarning(logWarning()) << "Query KO:" << _query.lastError().text();
       qWarning(logWarning()) << "Query text:" << _query.lastQuery();
    }
}

QString DatabaseManager::defaultDatabasePath()
{
    return StorageLocations::databasePath();
}

DatabaseManager::DatabaseManager(const QString& _path) :
    m_connectionName(nextConnectionName()),
    m_sqlDataBase(std::make_unique<QSqlDatabase>(
        QSqlDatabase::addDatabase("QSQLITE", m_connectionName))),
    m_albumDao(*m_sqlDataBase),
    m_pictureDao(*m_sqlDataBase),
    m_loggerDao(*m_sqlDataBase),
    m_movieDao(*m_sqlDataBase)
{
    m_sqlDataBase->setDatabaseName(_path);

    bool openStatus = m_sqlDataBase->open();
    qDebug(logInfo) << "Database connection:" << _path
                    << (openStatus ? "OK" : "Error");

    m_albumDao.init();
    m_pictureDao.init();
    m_loggerDao.init();
    m_movieDao.init();
}

DatabaseManager::~DatabaseManager()
{
    m_sqlDataBase->close();
    m_sqlDataBase.reset();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::isOpen() const
{
    return m_sqlDataBase && m_sqlDataBase->isOpen();
}

QSqlDatabase& DatabaseManager::database()
{
    return *m_sqlDataBase;
}
