#include "BridgeFrameParser.h"

#include <utility>

#include <QtEndian>

#include "BridgeContract.h"
#include "loggingcategories.h"

namespace {
constexpr int kPrefixSize = 4;  // u32 LE length
constexpr int kHeaderSize = 4;  // u16 magic + u8 version + u8 type

bool validFrameAt(const QByteArray& buffer, int offset, bool requireComplete)
{
    if (offset < 0 || buffer.size() - offset < kPrefixSize + kHeaderSize)
        return false;
    const auto* p = reinterpret_cast<const uchar*>(buffer.constData() + offset);
    const quint32 length = qFromLittleEndian<quint32>(p);
    const quint16 magic = qFromLittleEndian<quint16>(p + kPrefixSize);
    const quint8 version = p[kPrefixSize + 2];
    if (magic != BridgeContract::magic || version != BridgeContract::version
        || length < quint32(kHeaderSize)
        || length > quint32(BridgeContract::maxFrameLength)) {
        return false;
    }
    return !requireComplete || buffer.size() - offset >= kPrefixSize + int(length);
}

int nextCompleteFrame(const QByteArray& buffer, int from, int before)
{
    const int last = qMin(before, buffer.size() - (kPrefixSize + kHeaderSize));
    for (int offset = from; offset <= last; ++offset) {
        if (validFrameAt(buffer, offset, true))
            return offset;
    }
    return -1;
}
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

        const int candidateSize = kPrefixSize + int(length);
        if (m_buffer.size() < candidateSize) {
            // A corrupted length can otherwise make a plausible header hold
            // the parser hostage while a complete intact frame is already
            // present later in the buffer. Prefer that later complete frame.
            const int recovery = nextCompleteFrame(m_buffer, 1, m_buffer.size());
            if (recovery < 0)
                break;  // genuinely incomplete — wait for the next feed()
            m_buffer.remove(0, recovery);
            m_skippedBytes += recovery;
            continue;
        }

        // Likewise, a slightly enlarged corrupt length can swallow the first
        // bytes of the following intact frame. A complete nested header before
        // the claimed boundary is stronger evidence than the suspect length.
        const int recovery = nextCompleteFrame(m_buffer, 1, candidateSize - 1);
        if (recovery >= 0) {
            m_buffer.remove(0, recovery);
            m_skippedBytes += recovery;
            continue;
        }

        if (m_skippedBytes > 0) {
            qWarning(logCritical()) << m_name << ": resynchronized after skipping"
                                    << m_skippedBytes << "corrupt byte(s).";
            m_skippedBytes = 0;
        }

        const quint8 type = p[kPrefixSize + 3];
        if (handler)
            handler(type, p + kPrefixSize + kHeaderSize, int(length) - kHeaderSize);

        m_buffer.remove(0, candidateSize);
    }
}
