#ifndef PEAKLISTMODEL_H
#define PEAKLISTMODEL_H

#include <QAbstractListModel>

#include "SpectroAnalysis.h"

// Detected peaks for the analysis panel and the plot markers. Rows carry
// both raw numbers (marker positioning, centerOn) and display-formatted
// strings (so QML never formats). Updated per frame: same row count emits
// dataChanged so delegates update in place instead of being recreated.
class PeakListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        WavelengthNmRole = Qt::UserRole + 1,
        ValueRole,
        ProminenceRole,
        WavelengthTextRole,
        ValueTextRole,
        ProminenceTextRole,
    };

    explicit PeakListModel(QObject* parent = nullptr);

    void setPeaks(const QVector<SpectroAnalysis::Peak>& peaks);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    static QString formatValue(double value);

    QVector<SpectroAnalysis::Peak> m_peaks;
};

#endif // PEAKLISTMODEL_H
