#include "TraceRecorder.h"

TraceRecorder::TraceRecorder(SensorTransport* inner, const QString& traceFilePath,
                             QObject* parent)
    : SensorTransport(parent)
    , m_inner(inner)
    , m_file(traceFilePath)
{
    connect(m_inner, &SensorTransport::bytesReceived,
            this, [this](const QByteArray& bytes) {
                record("rx", bytes);
                emit bytesReceived(bytes);
            });
    connect(m_inner, &SensorTransport::transportError,
            this, &SensorTransport::transportError);
    connect(m_inner, &SensorTransport::closed,
            this, &SensorTransport::closed);
}

bool TraceRecorder::open(QString* errorMessage)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("cannot open trace file: %1")
                                .arg(m_file.fileName());
        return false;
    }
    if (!m_inner->open(errorMessage)) {
        m_file.close();
        return false;
    }
    m_clock.start();
    return true;
}

void TraceRecorder::close()
{
    m_inner->close();
    m_file.close();
}

void TraceRecorder::writeBytes(const QByteArray& bytes)
{
    record("tx", bytes);
    m_inner->writeBytes(bytes);
}

void TraceRecorder::record(const char* direction, const QByteArray& bytes)
{
    if (!m_file.isOpen())
        return;
    m_file.write(QByteArray::number(qint64(m_clock.elapsed())));
    m_file.write(" ");
    m_file.write(direction);
    m_file.write(" ");
    m_file.write(bytes.toHex());
    m_file.write("\n");
    m_file.flush();
}
