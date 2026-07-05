#include "ScriptedTransport.h"

#include <QTimer>

ScriptedTransport::ScriptedTransport(QObject* parent)
    : SensorTransport(parent)
{
}

bool ScriptedTransport::open(QString* errorMessage)
{
    if (failNextOpen) {
        failNextOpen = false;
        if (errorMessage)
            *errorMessage = QStringLiteral("scripted open failure");
        return false;
    }
    m_open = true;
    return true;
}

void ScriptedTransport::close()
{
    if (!m_open)
        return;
    m_open = false;
    emit closed();
}

void ScriptedTransport::writeBytes(const QByteArray& bytes)
{
    m_written.append(bytes);
}

void ScriptedTransport::setFaultPolicy(const FaultPolicy& policy)
{
    m_faults = policy;
    m_rng.seed(policy.seed);
}

void ScriptedTransport::enqueueIncoming(const QByteArray& bytes, int delayMs)
{
    if (delayMs <= 0) {
        deliver(bytes);
        return;
    }
    QTimer::singleShot(delayMs, this, [this, bytes]() { deliver(bytes); });
}

void ScriptedTransport::deliver(QByteArray bytes)
{
    if (!m_open)
        return;

    if (m_faults.corruptByteProbability > 0.0) {
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        std::uniform_int_distribution<int> flip(1, 255);
        for (char& b : bytes) {
            if (chance(m_rng) < m_faults.corruptByteProbability)
                b = char(uchar(b) ^ uchar(flip(m_rng)));
        }
    }

    if (m_chunkSize <= 0 || bytes.size() <= m_chunkSize) {
        emit bytesReceived(bytes);
        return;
    }
    for (int offset = 0; offset < bytes.size(); offset += m_chunkSize)
        emit bytesReceived(bytes.mid(offset, m_chunkSize));
}
