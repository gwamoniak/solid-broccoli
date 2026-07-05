#ifndef BRIDGECONTRACT_H
#define BRIDGECONTRACT_H

#include <QByteArray>
#include <QUuid>
#include <QtEndian>
#include <QtGlobal>

#include "Spectrum.h"

// The wire contract between this app and the ESP32 sensor bridge firmware
// (the firmware itself is out of scope — SimulatedBridgeTransport in
// spectro-sim is its executable specification). Constants only: the parsing
// lives in the codecs, the byte generation in the simulated bridge, and the
// two are kept as independent implementations on purpose.
//
// Frame layout (data characteristic, may span multiple BLE notifications):
//     u32 LE length          bytes that follow the prefix (header + payload)
//     u16 LE magic  0x5BEC
//     u8  version   0x01
//     u8  frame type          0x01 spectrum, 0x02 geiger (Milestone 10)
//     payload...
// Spectrum payload (type 0x01):
//     u8 channel count | u32 LE timestamp ms | channelCount * f32 LE counts
// Control commands (control characteristic, no framing):
//     0x01 start | 0x02 stop | 0x03 + u16 LE integration ms + u8 averaging
namespace BridgeContract {

inline QUuid serviceUuid()
{
    return QUuid(QStringLiteral("{5B0C0001-8E2B-4D8B-9C60-0A5B3D1EAD01}"));
}
inline QUuid dataCharacteristicUuid()
{
    return QUuid(QStringLiteral("{5B0C0002-8E2B-4D8B-9C60-0A5B3D1EAD01}"));
}
inline QUuid controlCharacteristicUuid()
{
    return QUuid(QStringLiteral("{5B0C0003-8E2B-4D8B-9C60-0A5B3D1EAD01}"));
}

constexpr quint16 magic = 0x5BEC;
constexpr quint8 version = 0x01;
constexpr quint8 frameSpectrum = 0x01;
constexpr quint8 frameGeiger = 0x02;

constexpr quint8 cmdStart = 0x01;
constexpr quint8 cmdStop = 0x02;
constexpr quint8 cmdSetParams = 0x03;

// Upper bound a decoder accepts for the length field; anything larger is
// treated as corruption during resynchronization.
constexpr int maxFrameLength = 2048;

// Control-command encoders, shared by every codec on this link — the command
// set is a property of the bridge, not of a sensor modality.
inline QByteArray encodeStartCommand()
{
    return QByteArray(1, char(cmdStart));
}
inline QByteArray encodeStopCommand()
{
    return QByteArray(1, char(cmdStop));
}
inline QByteArray encodeParamsCommand(const AcquisitionParams& params)
{
    QByteArray out(4, Qt::Uninitialized);
    out[0] = char(cmdSetParams);
    qToLittleEndian<quint16>(quint16(qBound(0, params.integrationTimeMs, 65535)),
                             out.data() + 1);
    out[3] = char(quint8(qBound(1, params.averaging, 255)));
    return out;
}

} // namespace BridgeContract

#endif // BRIDGECONTRACT_H
