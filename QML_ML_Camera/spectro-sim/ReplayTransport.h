#ifndef REPLAYTRANSPORT_H
#define REPLAYTRANSPORT_H

#include <QByteArray>
#include <QList>

#include "SensorTransport.h"
#include "spectro-sim_global.h"

// Plays a TraceRecorder file back as if the device were attached: rx lines
// are emitted as bytesReceived (with original timing, or synchronously
// during open() in Instant mode for tests); tx lines and live writes are
// ignored — they document what the app sent historically.
class SPECTROSIM_EXPORT ReplayTransport : public SensorTransport
{
    Q_OBJECT

public:
    enum class Mode { RealTime, Instant };

    explicit ReplayTransport(const QString& traceFilePath, Mode mode = Mode::RealTime,
                             QObject* parent = nullptr);

    static ReplayTransport* instant(const QString& traceFilePath,
                                    QObject* parent = nullptr);

    bool open(QString* errorMessage) override;
    void close() override;
    void writeBytes(const QByteArray&) override {}

private:
    struct Event {
        qint64 msOffset;
        QByteArray bytes;
    };

    QString m_path;
    Mode m_mode;
    bool m_open = false;
    QList<Event> m_events;
};

#endif // REPLAYTRANSPORT_H
