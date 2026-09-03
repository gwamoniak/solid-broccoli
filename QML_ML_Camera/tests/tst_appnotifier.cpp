#include <QSignalSpy>
#include <QtTest>

#include "AppNotifier.h"

class TstAppNotifier : public QObject
{
    Q_OBJECT

private slots:
    void publishesTypedNotifications()
    {
        AppNotifier notifier;
        QSignalSpy changed(&notifier, &AppNotifier::notificationChanged);

        notifier.showWarning(QStringLiteral("Check reference"));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(notifier.message(), QStringLiteral("Check reference"));
        QCOMPARE(notifier.level(), AppNotifier::Warning);
        QCOMPARE(notifier.revision(), quint64(1));

        notifier.showError(QStringLiteral("Disconnected"));
        QCOMPARE(changed.count(), 2);
        QCOMPARE(notifier.level(), AppNotifier::Error);
        QCOMPARE(notifier.revision(), quint64(2));

        notifier.showInfo(QString());
        QCOMPARE(changed.count(), 2);
    }
};

QTEST_GUILESS_MAIN(TstAppNotifier)
#include "tst_appnotifier.moc"
