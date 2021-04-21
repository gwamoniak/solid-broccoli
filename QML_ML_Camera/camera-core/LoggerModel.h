#ifndef LOGGERMODEL_H
#define LOGGERMODEL_H


#include <QSqlQueryModel>
#include "camera-core_global.h"

class DatabaseManager;

class CAMERACORESHARED_EXPORT LoggerModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    LoggerModel(QObject* parent = nullptr);

    enum Roles {
        IdRole = Qt::UserRole + 1,      // id
        TimeRole,                      // time
        TypeRole,                      // type of message
        MessageRole                    // message
    };


    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void clearTable();
    void updateModel();

    bool insertIntoTable(const QVariantList &data);
private:
    bool isIndexValid(const QModelIndex& _index) const;


private:
    DatabaseManager& m_sqlDB;

public slots:
    void readCSV(QString const &_fileName);

};

#endif // LOGGERMODEL_H
