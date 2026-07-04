#include "DetectionModel.h"

DetectionModel::DetectionModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int DetectionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_detections.size();
}

QVariant DetectionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_detections.size())
        return QVariant();
    const Detection& d = m_detections.at(index.row());
    switch (role) {
    case LabelRole:
        return d.label;
    case ConfidenceRole:
        return double(d.confidence);
    case ConfidenceTextRole:
        return QStringLiteral("%1%").arg(qRound(d.confidence * 100));
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DetectionModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {ConfidenceRole, "confidence"},
        {ConfidenceTextRole, "confidenceText"},
    };
}

void DetectionModel::updateFromResult(const QVariantMap& result)
{
    if (result.value(QStringLiteral("type")).toString()
        != QLatin1String("detections"))
        return;

    QVector<Detection> detections;
    const QVariantList list = result.value(QStringLiteral("detections")).toList();
    detections.reserve(list.size());
    for (const QVariant& entry : list)
        detections.append(Detection::fromVariant(entry.toMap()));
    setDetections(detections);
}

void DetectionModel::clear()
{
    setDetections({});
}

void DetectionModel::setDetections(const QVector<Detection>& detections)
{
    // Full reset per update: detection sets are small (a handful of rows)
    // and arrive a few times a second at most.
    beginResetModel();
    m_detections = detections;
    endResetModel();
    {
        QMutexLocker locker(&m_snapshotMutex);
        m_snapshot = detections;
    }
    emit countChanged();
}

QVector<Detection> DetectionModel::latestDetections() const
{
    QMutexLocker locker(&m_snapshotMutex);
    return m_snapshot;
}
