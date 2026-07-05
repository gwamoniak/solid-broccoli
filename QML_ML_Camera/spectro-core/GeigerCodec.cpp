#include "GeigerCodec.h"

#include <bit>

#include <QtEndian>

#include "BridgeContract.h"
#include "loggingcategories.h"

void GeigerCodec::setSink(std::function<void(SensorReading)> sink)
{
    m_sink = std::move(sink);
}

void GeigerCodec::feed(const QByteArray& bytes)
{
    m_parser.feed(bytes, [this](quint8 type, const uchar* payload, int size) {
        if (type == BridgeContract::frameGeiger)
            parseGeigerPayload(payload, size);
        // Spectrum frames (0x01) on the same link are not this codec's
        // modality; a well-formed frame is consumed silently.
    });
}

void GeigerCodec::parseGeigerPayload(const uchar* payload, int size)
{
    constexpr int kPayloadSize = 4 + 4;  // u32 timestamp + f32 cps
    if (size != kPayloadSize) {
        qWarning(logCritical()) << "GeigerCodec: unexpected payload size" << size;
        return;
    }

    GeigerReading reading;
    reading.timestampMs = qFromLittleEndian<quint32>(payload);
    reading.countsPerSecond =
        double(std::bit_cast<float>(qFromLittleEndian<quint32>(payload + 4)));

    if (m_sink)
        m_sink(reading);
}

QByteArray GeigerCodec::encodeStart()
{
    return BridgeContract::encodeStartCommand();
}

QByteArray GeigerCodec::encodeStop()
{
    return BridgeContract::encodeStopCommand();
}

QByteArray GeigerCodec::encodeParams(const AcquisitionParams& params)
{
    return BridgeContract::encodeParamsCommand(params);
}
