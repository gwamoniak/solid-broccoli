#include <QtTest>

#include "SpectrumOverlayProcessor.h"
#include "SpectrumSynthesizer.h"

// The overlay must pass frames through untouched when there is no spectrum
// and visibly draw onto the lower third when there is one.
class TstSpectrumOverlay : public QObject
{
    Q_OBJECT

private slots:
    void passthroughWithoutSpectrum()
    {
        SpectrumOverlayProcessor processor([]() { return Spectrum(); });
        QVERIFY(processor.initialize(nullptr));

        QImage frame(64, 48, QImage::Format_RGBA8888);
        frame.fill(Qt::blue);
        QCOMPARE(processor.process(frame), frame);
    }

    void overlayDrawsOntoLowerThird()
    {
        Spectrum s;
        s.wavelengthsNm = SpectrumSynthesizer::standardGrid();
        s.counts = SpectrumSynthesizer::mercuryLamp(s.wavelengthsNm, 2.0);
        for (double& c : s.counts)
            c *= 1000.0;
        s.timestampMs = 1;

        SpectrumOverlayProcessor processor([s]() { return s; });
        QImage frame(320, 240, QImage::Format_RGBA8888);
        frame.fill(Qt::blue);

        const QImage out = processor.process(frame);
        QVERIFY(out != frame);
        // Upper region untouched, lower-third panel darkened.
        QCOMPARE(out.pixelColor(160, 20), QColor(Qt::blue));
        QVERIFY(out.pixelColor(160, 200) != QColor(Qt::blue));
    }
};

// QTEST_MAIN (not GUILESS): QPainter text drawing needs QGuiApplication.
QTEST_MAIN(TstSpectrumOverlay)
#include "tst_spectrumoverlay.moc"
