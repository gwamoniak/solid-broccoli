#include "HelpContent.h"

#include <QFile>

#include "loggingcategories.h"

HelpContent::HelpContent(QObject* parent)
    : QObject(parent)
{
    QFile file(QStringLiteral(":/docs/USER_MANUAL.md"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        m_markdown = QString::fromUtf8(file.readAll());
    else
        qWarning(logWarning()) << "HelpContent: bundled user manual is unavailable.";
}
