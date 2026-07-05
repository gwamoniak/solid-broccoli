#ifndef SESSIONMODEL_H
#define SESSIONMODEL_H

#include <QAbstractListModel>

#include "SessionDAO.h"
#include "camera-core_global.h"

class DatabaseManager;

// QML bridge for the sessions list (Sessions tab). Mirrors AlbumModel.
class CAMERACORESHARED_EXPORT SessionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DBRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CreatedRole,
        CountRole,
    };

    explicit SessionModel(DatabaseManager& db, QObject* parent = nullptr);

    Q_INVOKABLE int addSessionFromName(const QString& name);
    Q_INVOKABLE void rename(int row, const QString& name);
    Q_INVOKABLE bool removeRows(int row, int count,
                                const QModelIndex& parent = QModelIndex()) override;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE int sessionIdAt(int row) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    DatabaseManager& m_sqlDB;
    QVector<SessionRecord> m_sessions;
};

#endif // SESSIONMODEL_H
