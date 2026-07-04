#ifndef DETECTIONMODEL_H
#define DETECTIONMODEL_H

#include <QAbstractListModel>
#include <QMutex>
#include <QVariantMap>
#include <QVector>

#include "Detection.h"

// QML bridge for the live object detections: the chip row on the camera
// page binds here. Fed on the GUI thread by updateFromResult (delivered
// queued from whichever thread the detector published on); also holds a
// mutex-guarded latest snapshot for cross-thread readers (the Milestone 13
// report generator), mirroring latestSpectrumSnapshot().
class DetectionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        LabelRole = Qt::UserRole + 1,
        ConfidenceRole,       // 0..1
        ConfidenceTextRole,   // pre-formatted "93%"
    };

    explicit DetectionModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVector<Detection> latestDetections() const;  // thread-safe copy

public slots:
    void updateFromResult(const QVariantMap& result);
    void clear();

signals:
    void countChanged();

private:
    void setDetections(const QVector<Detection>& detections);

    QVector<Detection> m_detections;
    mutable QMutex m_snapshotMutex;
    QVector<Detection> m_snapshot;
};

#endif // DETECTIONMODEL_H
