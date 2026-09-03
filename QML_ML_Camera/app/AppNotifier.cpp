#include "AppNotifier.h"

void AppNotifier::publish(const QString& message, Level level)
{
    if (message.isEmpty())
        return;
    m_message = message;
    m_level = level;
    ++m_revision;
    emit notificationChanged();
}
