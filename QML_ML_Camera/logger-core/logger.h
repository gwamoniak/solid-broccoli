#ifndef LOGGERCORE_H
#define LOGGERCORE_H

#include "loggingcategories.h"

#include <QObject>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>
#include <QString>
#include <QDebug>


// might to add mutex and worker/logger thread

// https://www.francescmm.com/logging-with-qt/

static const int LOGSIZE = 2048*100;
static const int LOGFILES = 3;

void LogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString& msg);

const QString sLogFolderName = "logs";

class LOGGERCORESHARED_EXPORT Logger
{

public:
    bool InitLogger();

    void InitLogFile();
    void DeleteOldLogs();

};

#endif // LOGGERCORE_H
