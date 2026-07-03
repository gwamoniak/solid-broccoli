#ifndef PROTOCOLCODEC_H
#define PROTOCOLCODEC_H

#include <functional>

#include <QByteArray>

#include "Spectrum.h"
#include "spectro-core_global.h"

// Sans-IO protocol translator: bytes in, readings out, commands to bytes.
// A codec never performs I/O and holds no transport handle, so it can be
// tested exhaustively by feeding it hand-built byte arrays (whole frames,
// split frames, corrupt frames). Implementations buffer partial input across
// feed() calls and must resynchronize after corruption.
class SPECTROCORE_EXPORT ProtocolCodec
{
public:
    virtual ~ProtocolCodec() = default;

    // May deliver 0..n readings to the sink per call.
    virtual void feed(const QByteArray& bytes) = 0;
    virtual void setSink(std::function<void(SensorReading)> sink) = 0;

    virtual QByteArray encodeStart() = 0;
    virtual QByteArray encodeStop() = 0;
    virtual QByteArray encodeParams(const AcquisitionParams& params) = 0;
};

#endif // PROTOCOLCODEC_H
