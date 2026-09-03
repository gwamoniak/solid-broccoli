#ifndef APPNOTIFIER_H
#define APPNOTIFIER_H

#include <QObject>
#include <QString>

class AppNotifier final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message NOTIFY notificationChanged)
    Q_PROPERTY(Level level READ level NOTIFY notificationChanged)
    Q_PROPERTY(quint64 revision READ revision NOTIFY notificationChanged)

public:
    enum Level { Info, Warning, Error };
    Q_ENUM(Level)

    explicit AppNotifier(QObject* parent = nullptr) : QObject(parent) {}

    QString message() const { return m_message; }
    Level level() const { return m_level; }
    quint64 revision() const { return m_revision; }

public slots:
    void showInfo(const QString& message) { publish(message, Info); }
    void showWarning(const QString& message) { publish(message, Warning); }
    void showError(const QString& message) { publish(message, Error); }

signals:
    void notificationChanged();

private:
    void publish(const QString& message, Level level);

    QString m_message;
    Level m_level = Info;
    quint64 m_revision = 0;
};

#endif // APPNOTIFIER_H
