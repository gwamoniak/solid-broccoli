#ifndef BRIDGEFRAMEPARSER_H
#define BRIDGEFRAMEPARSER_H

#include <functional>

#include <QByteArray>
#include <QString>

#include "spectro-core_global.h"

// Shared reassembly + resynchronization for the bridge contract's framing
// (BridgeContract.h), extracted so every codec on this link parses frames
// identically. Buffers partial input across feed() calls; a frame is trusted
// only when length, magic, and version all agree at the same offset — on any
// mismatch the buffer shifts one byte and rescans, the only strategy that
// recovers regardless of which header byte was corrupted.
class SPECTROCORE_EXPORT BridgeFrameParser
{
public:
    // type + payload of each well-formed frame, any type byte.
    using FrameHandler = std::function<void(quint8 type, const uchar* payload, int size)>;

    explicit BridgeFrameParser(QString codecName);

    void feed(const QByteArray& bytes, const FrameHandler& handler);

private:
    QString m_name;  // codec name for log lines
    QByteArray m_buffer;
    qint64 m_skippedBytes = 0;
};

#endif // BRIDGEFRAMEPARSER_H
