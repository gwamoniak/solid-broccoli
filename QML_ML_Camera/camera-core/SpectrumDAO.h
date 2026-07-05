#ifndef SPECTRUMDAO_H
#define SPECTRUMDAO_H

#include <QByteArray>
#include <QString>
#include <QVector>

#include "Spectrum.h"
#include "camera-core_global.h"

class QSqlDatabase;

struct CAMERACORESHARED_EXPORT SpectrumEntry
{
    int id = -1;
    int sessionId = -1;
    QString name;
    QString tags;
    QString createdUtc;
    Spectrum spectrum;

    bool isValid() const { return id >= 0 && spectrum.isValid(); }
};

// CRUD for stored spectra. Wavelength/counts arrays are stored as raw
// native-endian float64 blobs (all supported targets are little-endian);
// encode/decode are exposed so tests can verify the round trip directly.
class CAMERACORESHARED_EXPORT SpectrumDAO
{
public:
    explicit SpectrumDAO(QSqlDatabase& database);
    void init() const;

    int addSpectrum(int sessionId, const Spectrum& spectrum,
                    const QString& name, const QString& tags) const;
    QVector<SpectrumEntry> spectra(int sessionId) const;
    SpectrumEntry spectrumById(int id) const;
    void removeSpectrum(int id) const;
    void rename(int id, const QString& name) const;

    static QByteArray encodeDoubles(const QVector<double>& values);
    static QVector<double> decodeDoubles(const QByteArray& blob);
    static QString kindToString(Spectrum::Kind kind);
    static Spectrum::Kind kindFromString(const QString& kind);

private:
    SpectrumEntry entryFromQuery(const class QSqlQuery& query) const;

    QSqlDatabase& m_sqlDB;
};

#endif // SPECTRUMDAO_H
