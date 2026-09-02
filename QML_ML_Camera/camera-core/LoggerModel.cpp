#include "LoggerModel.h"
#include "DatabaseManager.h"
#include "StorageLocations.h"

#include "logger.h"
#include <QSqlQuery>
#include <QFile>
#include <QDir>
#include <QFileInfoList>

LoggerModel::LoggerModel(DatabaseManager& db, QObject *parent):
            QSqlQueryModel(parent),
            m_sqlDB(db)
{
    qDebug(logInfo()) << "LoggerModel has been created!";
    updateModel();
    clearTable();
}

int LoggerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QSqlQuery query(m_sqlDB.database());
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
    QSqlQuery query(m_sqlDB.database());
    query.exec(QString("DELETE FROM "  + TABLE));
}

void LoggerModel::updateModel()
{
    this->setQuery(QString("SELECT id, " + TABLE_TIME + ", " + TABLE_TYPE + ", " + TABLE_MESSAGE + " FROM " + TABLE),
                   m_sqlDB.database());
}

void LoggerModel::readCSV(const QString &_fileName)
{
    clearTable();

    QFile file(_fileName);
    qDebug(logInfo()) << "Reading log CSV:" << _fileName;
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug(logCritical()) << "Log CSV does not exist:" << _fileName;
    }
    else {

        int insertedRows = 0;
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            auto values = line.split(",");

            if (values.size() < 3) {
                continue;
            }

            QVariantList data;
            data.append(values.at(0));
            data.append(values.at(1));
            data.append(values.at(2));
            m_sqlDB.loggerDao().insertIntoTable(data);
            insertedRows++;

        }
        file.close();
        updateModel();

        qDebug(logInfo()) << "Loaded log rows:" << insertedRows;
    }


}

void LoggerModel::readLatestLog()
{
    QDir dir(StorageLocations::logsDir());
    dir.setNameFilters(QStringList() << "SolidBroccoli_Log_*.csv");
    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Time);

    const QFileInfoList logs = dir.entryInfoList();
    if (logs.isEmpty()) {
        qDebug(logWarning()) << "No log files found in:" << dir.absolutePath();
        updateModel();
        return;
    }

    qDebug(logInfo()) << "Latest log file:" << logs.first().absoluteFilePath();
    readCSV(logs.first().absoluteFilePath());
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
