#include "LoggerModel.h"
#include "DatabaseManager.h"

#include "logger.h"
#include <QSqlQuery>
#include <QFile>

LoggerModel::LoggerModel(QObject *parent):
            QSqlQueryModel(parent),
            m_sqlDB(DatabaseManager::instance())
{
    qDebug(logInfo()) << "LoggerModel has been created!";
    updateModel();
    clearTable();
}

int LoggerModel::rowCount(const QModelIndex &parent) const
{
    QSqlQuery query;
    query.prepare(QString("SELECT COUNT(*) FROM " + TABLE + " ;"));
    query.exec();
    query.first();

    return query.value(0).toInt();
}

QVariant LoggerModel::data(const QModelIndex &_index, int role) const
{
    if (!isIndexValid(_index)) {
        return QVariant();
    }
    int columnId = role - Qt::UserRole - 1;
    // Create the index using a column ID
    QModelIndex modelIndex = this->index(_index.row(), columnId);

    return QSqlQueryModel::data(modelIndex, Qt::DisplayRole);

}

QHash<int, QByteArray> LoggerModel::roleNames() const
{

    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TimeRole] = "time";
    roles[TypeRole] = "type";
    roles[MessageRole] = "message";
    return roles;
}

void LoggerModel::clearTable()
{
    this ->setQuery(QString("DELETE FROM "  + TABLE));
}

void LoggerModel::updateModel()
{
    this->setQuery(QString("SELECT id, " + TABLE_TIME + ", " + TABLE_TYPE + ", " + TABLE_MESSAGE + " FROM " + TABLE));
}

void LoggerModel::readCSV(const QString &_fileName)
{
    QFile file(_fileName);
    //qDebug(logInfo()) << _fileName;
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug(logCritical()) << "File not exists";
    }
    else {

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            auto values = line.split(",");

            QVariantList data;
            data.append(values.at(0));
            data.append(values.at(1));
            data.append(values.at(2));
            m_sqlDB.m_loggerDao.insertIntoTable(data);

        }
        file.close();
        updateModel();

        //qDebug(logInfo()) << "end of ReadCSV";
    }


}


bool LoggerModel::isIndexValid(const QModelIndex &_index) const
{
    if (_index.row() < 0
        || _index.row() >= rowCount()
        || !_index.isValid()) {
        return false;
    }
    return true;
}
