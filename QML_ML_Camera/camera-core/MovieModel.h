#ifndef MOVIEMODEL_H
#define MOVIEMODEL_H

#include <memory>
#include <vector>

#include <QAbstractListModel>

#include "camera-core_global.h"
#include "Movie.h"

class DatabaseManager;

class CAMERACORESHARED_EXPORT MovieModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum DBRoles {
        IdRole = Qt::UserRole + 1,
        UrlRole,
        FilePathRole,
        NameRole,
        DurationRole,   // formatted "mm:ss"
        CreatedAtRole,
    };

    explicit MovieModel(DatabaseManager& db, QObject* parent = nullptr);

    Q_INVOKABLE void rename(int row, const QString& _name);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& _index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    // Wired in main.cpp to CameraService::recordingSaved.
    void addRecording(const QString& filePath, qint64 durationMs);

private:
    bool isIndexValid(const QModelIndex& _index) const;

private:
    DatabaseManager& m_sqlDB;
    std::unique_ptr<std::vector<std::unique_ptr<Movie>>> m_vMovies;
};

#endif // MOVIEMODEL_H
