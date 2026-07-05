#include <QtTest>
#include <QImage>
#include <QColor>

#include "GrayscaleProcessor.h"

// Unit-tests the proof plugin's pure logic by compiling the processor straight
// into the test (no QPluginLoader): a colored frame comes back as an
// equal-channel grayscale image in a predictable format.
class TestFrameProcessorGrayscale : public QObject
{
    Q_OBJECT
private slots:
    void identity();
    void convertsColorToGray();
    void preservesSize();
};

void TestFrameProcessorGrayscale::identity()
{
    GrayscaleProcessor proc;
    QString error;
    QVERIFY2(proc.initialize(&error), qPrintable(error));
    QCOMPARE(proc.name(), QStringLiteral("Grayscale"));
    QVERIFY(!proc.description().isEmpty());
}

void TestFrameProcessorGrayscale::convertsColorToGray()
{
    GrayscaleProcessor proc;
    QString error;
    QVERIFY(proc.initialize(&error));

    QImage input(4, 4, QImage::Format_RGB32);
    input.fill(QColor(200, 40, 40)); // saturated red

    const QImage output = proc.process(input);

    QCOMPARE(output.format(), QImage::Format_RGBA8888);
    for (int y = 0; y < output.height(); ++y) {
        for (int x = 0; x < output.width(); ++x) {
            const QColor c = output.pixelColor(x, y);
            QCOMPARE(c.red(), c.green());
            QCOMPARE(c.green(), c.blue());
        }
    }
}

void TestFrameProcessorGrayscale::preservesSize()
{
    GrayscaleProcessor proc;
    QString error;
    QVERIFY(proc.initialize(&error));

    QImage input(7, 3, QImage::Format_RGB32);
    input.fill(Qt::green);

    const QImage output = proc.process(input);
    QCOMPARE(output.size(), input.size());
}

QTEST_GUILESS_MAIN(TestFrameProcessorGrayscale)
#include "tst_frameprocessor_grayscale.moc"
