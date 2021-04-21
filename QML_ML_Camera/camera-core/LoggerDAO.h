#ifndef LOGGERDAO_H
#define LOGGERDAO_H

#include <QObject>
#include <memory>

const QString TABLE =                  "CSVTable";
const QString TABLE_TIME  =            "Time";
const QString TABLE_TYPE   =           "Type";
const QString TABLE_MESSAGE =          "Message";

class QSqlDatabase;


// CRUD (Create/Read/Update/Delete) class
// https://en.wikipedia.org/wiki/Create,_read,_update_and_delete
// data access object - DAO
class LoggerDAO
{
public:
    explicit LoggerDAO(QSqlDatabase& _database);
    void init() const;
    void insertIntoTable(const QVariantList &data) const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // LOGGERDAO_H
