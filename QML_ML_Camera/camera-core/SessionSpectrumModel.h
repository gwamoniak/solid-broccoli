#ifndef SESSIONSPECTRUMMODEL_H
#define SESSIONSPECTRUMMODEL_H

#include <QAbstractListModel>

#include "SpectrumDAO.h"
#include "camera-core_global.h"

class DatabaseManager;

// QML bridge for the captures of one session (session detail page), scoped
// by setSessionId like PictureModel is scoped to an album. Also the C++
// access point for loading full spectra (overlays, export).
class CAMERACORESHARED_EXPORT SessionSpectrumModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DBRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        KindRole,
        CreatedRole,
        TagsRole,
    };

    explicit SessionSpectrumModel(DatabaseManager& db, QObject* parent = nullptr);

    Q_INVOKABLE void setSessionId(int sessionId);
    Q_INVOKABLE int sessionId() const { return m_sessionId; }
    Q_INVOKABLE void rename(int row, const QString& name);
    Q_INVOKABLE QVariantList sessionVideos(int sessionId) const;
    Q_INVOKABLE QVariantList sessionMeasurements(int sessionId) const;
    Q_INVOKABLE QVariantList sessionReports(int sessionId) const;
    Q_INVOKABLE bool exportReportMarkdown(int reportId) const;
    Q_INVOKABLE bool removeReport(int reportId) const;
    Q_INVOKABLE bool removeRows(int row, int count,
                                const QModelIndex& parent = QModelIndex()) override;

    // C++ API for the service.
    int addSpectrum(int sessionId, const Spectrum& spectrum,
                    const QString& name, const QString& tags);
    SpectrumEntry entryById(int id) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    DatabaseManager& m_sqlDB;
    int m_sessionId = -1;
    QVector<SpectrumEntry> m_entries;
};

#endif // SESSIONSPECTRUMMODEL_H
