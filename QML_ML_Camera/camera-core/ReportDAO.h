#ifndef REPORTDAO_H
#define REPORTDAO_H

#include <QString>
#include <QVector>

#include "camera-core_global.h"

class QSqlDatabase;

struct CAMERACORESHARED_EXPORT ReportRecord
{
    int id = -1;
    int sessionId = -1;
    QString createdUtc;
    QString modelName;      // GGUF the report was generated with
    QString promptVersion;  // template identifier, e.g. "report_v1"
    QString contextJson;    // exact model input — every report is auditable
    QString contentMd;      // the generated Markdown
};

// AI-generated session reports (Milestone 13). The context JSON is stored
// with each report so any report can be reproduced and audited. Cascade on
// session delete is explicit, performed by SessionDAO::removeSession.
class CAMERACORESHARED_EXPORT ReportDAO
{
public:
    explicit ReportDAO(QSqlDatabase& database);
    void init() const;

    int addReport(int sessionId, const QString& modelName,
                  const QString& promptVersion, const QString& contextJson,
                  const QString& contentMd) const;
    QVector<ReportRecord> reports(int sessionId) const;
    ReportRecord reportById(int id) const;
    void removeReport(int id) const;

private:
    QSqlDatabase& m_sqlDB;
};

#endif // REPORTDAO_H
