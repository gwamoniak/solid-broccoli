#ifndef GEIGERCODEC_H
#define GEIGERCODEC_H

#include <QByteArray>

#include "BridgeFrameParser.h"
#include "ProtocolCodec.h"
#include "spectro-core_global.h"

// Codec for the bridge contract's geiger frames (type 0x02: u32 LE timestamp
// ms + f32 LE counts-per-second), delivering GeigerReading through the
// SensorReading sink. Sans-IO like As7265xCodec, sharing the same framing
// and resynchronization via BridgeFrameParser; well-formed spectrum frames
// on the link are consumed silently. Derived quantities (rolling CPM, dose
// rate) are computed app-side — the codec reports what the tube said.
class SPECTROCORE_EXPORT GeigerCodec : public ProtocolCodec
{
public:
    void feed(const QByteArray& bytes) override;
    void setSink(std::function<void(SensorReading)> sink) override;

    QByteArray encodeStart() override;
    QByteArray encodeStop() override;
    QByteArray encodeParams(const AcquisitionParams& params) override;

private:
    void parseGeigerPayload(const uchar* payload, int size);

    BridgeFrameParser m_parser{QStringLiteral("GeigerCodec")};
    std::function<void(SensorReading)> m_sink;
};

#endif // GEIGERCODEC_H
