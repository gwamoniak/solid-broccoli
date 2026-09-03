#ifndef HELPCONTENT_H
#define HELPCONTENT_H

#include <QObject>
#include <QString>

class HelpContent final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString markdown READ markdown CONSTANT)

public:
    explicit HelpContent(QObject* parent = nullptr);
    QString markdown() const { return m_markdown; }

private:
    QString m_markdown;
};

#endif // HELPCONTENT_H
