#include "SpectrumDAO.h"

#include <cstring>

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

SpectrumDAO::SpectrumDAO(QSqlDatabase& database)
    : m_sqlDB(database)
{
}

void SpectrumDAO::init() const
{
    if (!m_sqlDB.tables().contains("spectra")) {
        QSqlQuery query(m_sqlDB);
        query.exec("CREATE TABLE spectra (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "session_id INTEGER, kind TEXT, name TEXT, created_utc TEXT, "
                   "integration_ms INTEGER, averaging INTEGER, "
                   "wavelengths BLOB, counts BLOB, tags TEXT)");
        DatabaseManager::debugQuery(query);
    }
}

QByteArray SpectrumDAO::encodeDoubles(const QVector<double>& values)
{
    return QByteArray(reinterpret_cast<const char*>(values.constData()),
                      values.size() * int(sizeof(double)));
}

QVector<double> SpectrumDAO::decodeDoubles(const QByteArray& blob)
{
    QVector<double> values(blob.size() / int(sizeof(double)));
    std::memcpy(values.data(), blob.constData(), values.size() * sizeof(double));
    return values;
}

QString SpectrumDAO::kindToString(Spectrum::Kind kind)
{
    switch (kind) {
    case Spectrum::Kind::Dark:      return QStringLiteral("dark");
    case Spectrum::Kind::Reference: return QStringLiteral("reference");
    case Spectrum::Kind::Sample:    break;
    }
    return QStringLiteral("sample");
}

Spectrum::Kind SpectrumDAO::kindFromString(const QString& kind)
{
    if (kind == QStringLiteral("dark"))
        return Spectrum::Kind::Dark;
    if (kind == QStringLiteral("reference"))
        return Spectrum::Kind::Reference;
    return Spectrum::Kind::Sample;
}

int SpectrumDAO::addSpectrum(int sessionId, const Spectrum& spectrum,
                             const QString& name, const QString& tags) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("INSERT INTO spectra (session_id, kind, name, created_utc, "
                  "integration_ms, averaging, wavelengths, counts, tags) VALUES "
                  "(:session, :kind, :name, :created, :integration, :averaging, "
                  ":wavelengths, :counts, :tags)");
    query.bindValue(":session", sessionId);
    query.bindValue(":kind", kindToString(spectrum.kind));
    query.bindValue(":name", name);
    const QDateTime captured = spectrum.timestampMs > 0
        ? QDateTime::fromMSecsSinceEpoch(spectrum.timestampMs).toUTC()
        : QDateTime::currentDateTimeUtc();
    query.bindValue(":created", captured.toString(Qt::ISODate));
    query.bindValue(":integration", spectrum.params.integrationTimeMs);
    query.bindValue(":averaging", spectrum.params.averaging);
    query.bindValue(":wavelengths", encodeDoubles(spectrum.wavelengthsNm));
    query.bindValue(":counts", encodeDoubles(spectrum.counts));
    query.bindValue(":tags", tags);
    query.exec();
    DatabaseManager::debugQuery(query);
    return query.lastInsertId().toInt();
}

SpectrumEntry SpectrumDAO::entryFromQuery(const QSqlQuery& query) const
{
    SpectrumEntry entry;
    entry.id = query.value("id").toInt();
    entry.sessionId = query.value("session_id").toInt();
    entry.name = query.value("name").toString();
    entry.tags = query.value("tags").toString();
    entry.createdUtc = query.value("created_utc").toString();
    entry.spectrum.kind = kindFromString(query.value("kind").toString());
    entry.spectrum.params.integrationTimeMs = query.value("integration_ms").toInt();
    entry.spectrum.params.averaging = query.value("averaging").toInt();
    entry.spectrum.wavelengthsNm = decodeDoubles(query.value("wavelengths").toByteArray());
    entry.spectrum.counts = decodeDoubles(query.value("counts").toByteArray());
    entry.spectrum.timestampMs =
        QDateTime::fromString(entry.createdUtc, Qt::ISODate).toMSecsSinceEpoch();
    return entry;
}

QVector<SpectrumEntry> SpectrumDAO::spectra(int sessionId) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM spectra WHERE session_id = (:session) ORDER BY id");
    query.bindValue(":session", sessionId);
    query.exec();
    DatabaseManager::debugQuery(query);

    QVector<SpectrumEntry> list;
    while (query.next())
        list.append(entryFromQuery(query));
    return list;
}

SpectrumEntry SpectrumDAO::spectrumById(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("SELECT * FROM spectra WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
    if (query.next())
        return entryFromQuery(query);
    return {};
}

void SpectrumDAO::removeSpectrum(int id) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("DELETE FROM spectra WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}

void SpectrumDAO::rename(int id, const QString& name) const
{
    QSqlQuery query(m_sqlDB);
    query.prepare("UPDATE spectra SET name = (:name) WHERE id = (:id)");
    query.bindValue(":name", name);
    query.bindValue(":id", id);
    query.exec();
    DatabaseManager::debugQuery(query);
}
