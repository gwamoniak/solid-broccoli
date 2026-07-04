#ifndef MEASUREMENTDAO_H
#define MEASUREMENTDAO_H

#include <QString>
#include <QVector>

#include "camera-core_global.h"

class QSqlDatabase;

struct CAMERACORESHARED_EXPORT MeasurementRecord
{
    int id = -1;
    int sessionId = -1;
    QString createdUtc;
    QString type;      // "geiger_dose" for now; future modalities add theirs
    double value = 0.0;
    QString unit;
    QString summary;   // JSON with windowed stats (avg/max CPM, samples, name)
};

// Scalar time-series measurements (dose rates now, other single-value
// modalities later) — deliberately not forced into the spectrum-shaped
// `spectra` table (see Decision Log). Cascade on session delete is explicit,
// performed by SessionDAO::removeSession.
class CAMERACORESHARED_EXPORT MeasurementDAO
{
public:
    explicit MeasurementDAO(QSqlDatabase& database);
    void init() const;

    int addMeasurement(int sessionId, const QString& type, double value,
                       const QString& unit, const QString& summaryJson) const;
    QVector<MeasurementRecord> measurements(int sessionId) const;
    void removeMeasurement(int id) const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // MEASUREMENTDAO_H
