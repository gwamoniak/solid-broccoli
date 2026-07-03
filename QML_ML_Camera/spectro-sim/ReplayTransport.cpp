#include "ReplayTransport.h"

#include <QFile>
#include <QTimer>

ReplayTransport::ReplayTransport(const QString& traceFilePath, Mode mode, QObject* parent)
    : SensorTransport(parent)
    , m_path(traceFilePath)
    , m_mode(mode)
{
}

ReplayTransport* ReplayTransport::instant(const QString& traceFilePath, QObject* parent)
{
    return new ReplayTransport(traceFilePath, Mode::Instant, parent);
}

bool ReplayTransport::open(QString* errorMessage)
{
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("cannot open trace file: %1").arg(m_path);
        return false;
    }

    m_events.clear();
    while (!file.atEnd()) {
        const QList<QByteArray> parts = file.readLine().trimmed().split(' ');
        if (parts.size() != 3 || parts[1] != "rx")
            continue;
        m_events.append({parts[0].toLongLong(), QByteArray::fromHex(parts[2])});
    }
    m_open = true;

    if (m_mode == Mode::Instant) {
        for (const Event& e : m_events) {
            if (!m_open)
                break;
            emit bytesReceived(e.bytes);
        }
    } else {
        for (const Event& e : m_events) {
            QTimer::singleShot(e.msOffset, this, [this, bytes = e.bytes]() {
                if (m_open)
                    emit bytesReceived(bytes);
            });
        }
    }
    return true;
}

void ReplayTransport::close()
{
    if (!m_open)
        return;
    m_open = false;
    emit closed();
}
