#ifndef TRACERECORDER_H
#define TRACERECORDER_H

#include <QElapsedTimer>
#include <QFile>

#include "SensorTransport.h"
#include "spectro-sim_global.h"

// Decorator transport: forwards everything to the wrapped transport while
// appending timestamped events to a trace file, one per line:
//     <ms-offset> <rx|tx> <hex-bytes>
// Traces recorded against real hardware become regression fixtures for
// ReplayTransport. The inner transport is not owned.
class SPECTROSIM_EXPORT TraceRecorder : public SensorTransport
{
    Q_OBJECT

public:
    TraceRecorder(SensorTransport* inner, const QString& traceFilePath,
                  QObject* parent = nullptr);

    bool open(QString* errorMessage) override;
    void close() override;
    void writeBytes(const QByteArray& bytes) override;

private:
    void record(const char* direction, const QByteArray& bytes);

    SensorTransport* m_inner;
    QFile m_file;
    QElapsedTimer m_clock;
};

#endif // TRACERECORDER_H
