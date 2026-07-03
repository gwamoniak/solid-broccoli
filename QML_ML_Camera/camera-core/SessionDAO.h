#ifndef SESSIONDAO_H
#define SESSIONDAO_H

#include <QString>
#include <QVector>

#include "camera-core_global.h"

class QSqlDatabase;

struct CAMERACORESHARED_EXPORT SessionRecord
{
    int id = -1;
    QString name;
    QString createdUtc;
    QString notes;
    int captureCount = 0;
};

// CRUD for spectroscopy sessions (the "lab notebook" grouping). Removing a
// session cascades its spectra — SQLite foreign keys are not enabled in this
// connection, so the cascade is explicit.
class CAMERACORESHARED_EXPORT SessionDAO
{
public:
    explicit SessionDAO(QSqlDatabase& database);
    void init() const;

    int addSession(const QString& name) const;
    void updateName(int id, const QString& name) const;
    void removeSession(int id) const;
    QVector<SessionRecord> sessions() const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // SESSIONDAO_H
