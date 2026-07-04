#ifndef AS7265XCODEC_H
#define AS7265XCODEC_H

#include <QByteArray>
#include <QVector>

#include "ProtocolCodec.h"
#include "spectro-core_global.h"

// Codec for the bridge contract's spectrum frames (BridgeContract.h) as
// produced by an AS7265x behind the ESP32 bridge. Sans-IO: buffers bytes
// across feed() calls, reassembles length-prefixed frames, and delivers
// 18-point Spectrum readings; resynchronizes byte-by-byte after corruption.
class SPECTROCORE_EXPORT As7265xCodec : public ProtocolCodec
{
public:
    // The chip's fixed 18-channel wavelength table (nm) — a hardware
    // property of the AS7265x, so it lives with the codec.
    static QVector<double> wavelengthTable();

    void feed(const QByteArray& bytes) override;
    void setSink(std::function<void(SensorReading)> sink) override;

    QByteArray encodeStart() override;
    QByteArray encodeStop() override;
    QByteArray encodeParams(const AcquisitionParams& params) override;

private:
    void parseSpectrumPayload(const uchar* payload, int size);

    QByteArray m_buffer;
    std::function<void(SensorReading)> m_sink;
    AcquisitionParams m_lastParams;  // stamped onto emitted spectra
    qint64 m_skippedBytes = 0;
};

#endif // AS7265XCODEC_H
