#include "As7265xCodec.h"

#include <bit>

#include <QtEndian>

#include "BridgeContract.h"
#include "loggingcategories.h"

namespace {
constexpr int kPrefixSize = 4;  // u32 LE length
constexpr int kHeaderSize = 4;  // u16 magic + u8 version + u8 type
}

QVector<double> As7265xCodec::wavelengthTable()
{
    return {410, 435, 460, 485, 510, 535, 560, 585, 610,
            645, 680, 705, 730, 760, 810, 860, 900, 940};
}

void As7265xCodec::setSink(std::function<void(SensorReading)> sink)
{
    m_sink = std::move(sink);
}

void As7265xCodec::feed(const QByteArray& bytes)
{
    m_buffer.append(bytes);

    while (m_buffer.size() >= kPrefixSize + kHeaderSize) {
        const auto* p = reinterpret_cast<const uchar*>(m_buffer.constData());
        const quint32 length = qFromLittleEndian<quint32>(p);
        const quint16 magic = qFromLittleEndian<quint16>(p + kPrefixSize);
        const quint8 version = p[kPrefixSize + 2];

        // Trust nothing until length, magic, and version agree: on any
        // mismatch shift one byte and rescan. Byte-wise scanning is the only
        // strategy that recovers regardless of which field was corrupted.
        if (magic != BridgeContract::magic || version != BridgeContract::version
            || length < quint32(kHeaderSize)
            || length > quint32(BridgeContract::maxFrameLength)) {
            m_buffer.remove(0, 1);
            ++m_skippedBytes;
            continue;
        }

        if (m_buffer.size() < kPrefixSize + int(length))
            break;  // frame incomplete — wait for the next feed()

        if (m_skippedBytes > 0) {
            qWarning(logCritical()) << "As7265xCodec: resynchronized after skipping"
                                    << m_skippedBytes << "corrupt byte(s).";
            m_skippedBytes = 0;
        }

        const quint8 type = p[kPrefixSize + 3];
        if (type == BridgeContract::frameSpectrum)
            parseSpectrumPayload(p + kPrefixSize + kHeaderSize, int(length) - kHeaderSize);
        // Other frame types (geiger 0x02, future) are not this codec's
        // modality; a well-formed frame is consumed silently.

        m_buffer.remove(0, kPrefixSize + int(length));
    }
}

void As7265xCodec::parseSpectrumPayload(const uchar* payload, int size)
{
    constexpr int kMetaSize = 1 + 4;  // u8 channel count + u32 timestamp
    if (size < kMetaSize) {
        qWarning(logCritical()) << "As7265xCodec: spectrum payload truncated:" << size;
        return;
    }

    const int channels = payload[0];
    if (size != kMetaSize + channels * 4) {
        qWarning(logCritical()) << "As7265xCodec: payload size" << size
                                << "does not match channel count" << channels;
        return;
    }

    Spectrum spectrum;
    spectrum.wavelengthsNm = wavelengthTable();
    if (channels != spectrum.wavelengthsNm.size()) {
        qWarning(logCritical()) << "As7265xCodec: unexpected channel count" << channels;
        return;
    }

    spectrum.timestampMs = qFromLittleEndian<quint32>(payload + 1);
    spectrum.params = m_lastParams;
    spectrum.counts.resize(channels);
    for (int i = 0; i < channels; ++i) {
        const quint32 bits = qFromLittleEndian<quint32>(payload + kMetaSize + i * 4);
        spectrum.counts[i] = double(std::bit_cast<float>(bits));
    }

    if (m_sink)
        m_sink(std::move(spectrum));
}

QByteArray As7265xCodec::encodeStart()
{
    return QByteArray(1, char(BridgeContract::cmdStart));
}

QByteArray As7265xCodec::encodeStop()
{
    return QByteArray(1, char(BridgeContract::cmdStop));
}

QByteArray As7265xCodec::encodeParams(const AcquisitionParams& params)
{
    m_lastParams = params;
    QByteArray out(4, Qt::Uninitialized);
    out[0] = char(BridgeContract::cmdSetParams);
    qToLittleEndian<quint16>(quint16(qBound(0, params.integrationTimeMs, 65535)),
                             out.data() + 1);
    out[3] = char(quint8(qBound(1, params.averaging, 255)));
    return out;
}
