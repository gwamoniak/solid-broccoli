#include "As7265xCodec.h"

#include <bit>

#include <QtEndian>

#include "BridgeContract.h"
#include "loggingcategories.h"

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
    m_parser.feed(bytes, [this](quint8 type, const uchar* payload, int size) {
        if (type == BridgeContract::frameSpectrum)
            parseSpectrumPayload(payload, size);
        // Other frame types (geiger 0x02, future) are not this codec's
        // modality; a well-formed frame is consumed silently.
    });
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
    return BridgeContract::encodeStartCommand();
}

QByteArray As7265xCodec::encodeStop()
{
    return BridgeContract::encodeStopCommand();
}

QByteArray As7265xCodec::encodeParams(const AcquisitionParams& params)
{
    m_lastParams = params;
    return BridgeContract::encodeParamsCommand(params);
}
