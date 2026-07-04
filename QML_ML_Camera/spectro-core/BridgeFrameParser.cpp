#include "BridgeFrameParser.h"

#include <utility>

#include <QtEndian>

#include "BridgeContract.h"
#include "loggingcategories.h"

namespace {
constexpr int kPrefixSize = 4;  // u32 LE length
constexpr int kHeaderSize = 4;  // u16 magic + u8 version + u8 type
}

BridgeFrameParser::BridgeFrameParser(QString codecName)
    : m_name(std::move(codecName))
{
}

void BridgeFrameParser::feed(const QByteArray& bytes, const FrameHandler& handler)
{
    m_buffer.append(bytes);

    while (m_buffer.size() >= kPrefixSize + kHeaderSize) {
        const auto* p = reinterpret_cast<const uchar*>(m_buffer.constData());
        const quint32 length = qFromLittleEndian<quint32>(p);
        const quint16 magic = qFromLittleEndian<quint16>(p + kPrefixSize);
        const quint8 version = p[kPrefixSize + 2];

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
            qWarning(logCritical()) << m_name << ": resynchronized after skipping"
                                    << m_skippedBytes << "corrupt byte(s).";
            m_skippedBytes = 0;
        }

        const quint8 type = p[kPrefixSize + 3];
        if (handler)
            handler(type, p + kPrefixSize + kHeaderSize, int(length) - kHeaderSize);

        m_buffer.remove(0, kPrefixSize + int(length));
    }
}
