#include <QDataStream>
#include <QRandomGenerator>
#include <QtTest>

#include "BridgeFrameParser.h"

class TstBridgeFrameParser : public QObject
{
    Q_OBJECT

    static QByteArray frame(quint32 sequence)
    {
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out << quint32(8) << quint16(0x5BEC) << quint8(0x01) << quint8(0x7F)
            << sequence;
        return bytes;
    }

private slots:
    void intactFramesEmbeddedInSeededNoiseAreRecovered()
    {
        QRandomGenerator random(0x5B0C);
        QByteArray stream;
        for (quint32 sequence = 1; sequence <= 200; ++sequence) {
            const int noiseSize = int(random.bounded(33u));
            for (int i = 0; i < noiseSize; ++i)
                stream.append(char(0x80 | random.bounded(0x70u)));
            stream += frame(sequence);
        }

        QVector<quint32> observed;
        BridgeFrameParser parser(QStringLiteral("property-test"));
        int offset = 0;
        while (offset < stream.size()) {
            const int chunk = qMin(int(random.bounded(1u, 24u)), stream.size() - offset);
            parser.feed(stream.mid(offset, chunk), [&](quint8 type, const uchar* payload, int size) {
                QCOMPARE(type, quint8(0x7F));
                QCOMPARE(size, 4);
                observed.append(qFromLittleEndian<quint32>(payload));
            });
            offset += chunk;
        }

        QCOMPARE(observed.size(), 200);
        for (int i = 0; i < observed.size(); ++i)
            QCOMPARE(observed[i], quint32(i + 1));
    }

    void everyHeaderByteCorruptionResynchronizes()
    {
        const QByteArray original = frame(1);
        for (int byte = 0; byte < 8; ++byte) {
            QByteArray corrupt = original;
            corrupt[byte] = char(uchar(corrupt[byte]) ^ 0x5A);

            QVector<quint32> observed;
            BridgeFrameParser parser(QStringLiteral("single-byte-test"));
            parser.feed(corrupt + frame(2), [&](quint8, const uchar* payload, int size) {
                if (size == 4)
                    observed.append(qFromLittleEndian<quint32>(payload));
            });

            QVERIFY2(!observed.isEmpty(), qPrintable(QStringLiteral("byte %1 stalled recovery").arg(byte)));
            QCOMPARE(observed.last(), quint32(2));
        }
    }

    void arbitrarySeededInputNeverCrashesOrGrowsPastRecovery()
    {
        for (quint32 seed = 0; seed < 128; ++seed) {
            QRandomGenerator random(seed);
            BridgeFrameParser parser(QStringLiteral("fuzz-test"));
            for (int batch = 0; batch < 32; ++batch) {
                QByteArray noise(256, Qt::Uninitialized);
                for (char& byte : noise)
                    byte = char(random.generate() & 0xFF);
                parser.feed(noise, {});
            }

            int recovered = 0;
            parser.feed(frame(seed), [&](quint8 type, const uchar* payload, int size) {
                if (type == 0x7F && size == 4
                    && qFromLittleEndian<quint32>(payload) == seed) {
                    ++recovered;
                }
            });
            QCOMPARE(recovered, 1);
        }
    }
};

QTEST_GUILESS_MAIN(TstBridgeFrameParser)
#include "tst_bridgeframeparser.moc"
