#include "PeakListModel.h"

PeakListModel::PeakListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QString PeakListModel::formatValue(double value)
{
    // Counts are large integers; transmittance/absorbance are small floats.
    return std::abs(value) >= 100.0 ? QString::number(value, 'f', 0)
                                    : QString::number(value, 'f', 3);
}

void PeakListModel::setPeaks(const QVector<SpectroAnalysis::Peak>& peaks)
{
    if (peaks.size() != m_peaks.size()) {
        beginResetModel();
        m_peaks = peaks;
        endResetModel();
        return;
    }
    m_peaks = peaks;
    if (!m_peaks.isEmpty())
        emit dataChanged(index(0), index(m_peaks.size() - 1));
}

int PeakListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_peaks.size();
}

QVariant PeakListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_peaks.size())
        return {};
    const auto& peak = m_peaks[index.row()];
    switch (role) {
    case WavelengthNmRole:   return peak.wavelengthNm;
    case ValueRole:          return peak.value;
    case ProminenceRole:     return peak.prominence;
    case WavelengthTextRole: return QString::number(peak.wavelengthNm, 'f', 1) + QStringLiteral(" nm");
    case ValueTextRole:      return formatValue(peak.value);
    case ProminenceTextRole: return formatValue(peak.prominence);
    }
    return {};
}

QHash<int, QByteArray> PeakListModel::roleNames() const
{
    return {
        {WavelengthNmRole, "wavelengthNm"},
        {ValueRole, "value"},
        {ProminenceRole, "prominence"},
        {WavelengthTextRole, "wavelengthText"},
        {ValueTextRole, "valueText"},
        {ProminenceTextRole, "prominenceText"},
    };
}
