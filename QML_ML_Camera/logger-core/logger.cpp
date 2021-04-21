#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "logger.h"


static QString sLogFileName;

void LogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    QFile outFileCheck(sLogFileName);
    int size = outFileCheck.size();
    Logger log;
    if (size > LOGSIZE) //check current log size
    {
        log.DeleteOldLogs();
        log.InitLogFile();
    }


    QFile outFile(sLogFileName);
    if(!outFile.exists())
    {
        qDebug(logDebug()) << "File doesn't exist";
    }
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&outFile);
    textStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") << ",";
    switch (type)
    {
    case QtInfoMsg:     textStream << "SolidBroccoli - "  ; break;
#ifdef _DEBUG
    case QtDebugMsg:    textStream << "SolidBroccoli - "  ; break;
#endif
    case QtWarningMsg:  textStream << "SolidBroccoli - "  ; break;
    case QtCriticalMsg: textStream << "SolidBroccoli - "  ; break;
    case QtFatalMsg:    textStream << "SolidBroccoli - "  ; break;;
    }
    textStream << context.category << ": "  << ","
               << msg << endl;
    textStream.flush();    // Clear the buffered data
}


void Logger::InitLogFile()
{
    sLogFileName = QString(sLogFolderName + "/SolidBroccoli_Log_%1.csv")
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm"));

}

//The oldest files will be automatically deleted
// LOGFILES = 3
void Logger::DeleteOldLogs()
{
    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed);
    dir.setPath(sLogFolderName);
    QString SolidBroccoliExp = ""; //match SolidBroccoli_Log

    QFileInfoList list = dir.entryInfoList();
    if (list.size() <= LOGFILES)
    {
        return; //no files to delete
    } else
    {
        for (int i = 0; i < (list.size() - LOGFILES); i++)
        {
            QString path = list.at(i).absoluteFilePath();
            SolidBroccoliExp = path.section(QRegExp("^((?!SolidBroccoli_Log).)*$"), 0, 0,
                                     QString::SectionSkipEmpty); // delete only log files and leave other files intact
            if(SolidBroccoliExp!= "")
            {
                QFile file(path);
                file.remove();
            }
        }
    }

}

bool Logger::InitLogger()
{
    {
        QDir().mkdir(sLogFolderName);
    }

    DeleteOldLogs(); //delete old log files
    InitLogFile(); //create the logfile name

    QFile outFile(sLogFileName);
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qInstallMessageHandler(LogMessageHandler);
        return true;
    }
    else
    {
        return false;
    }

}
