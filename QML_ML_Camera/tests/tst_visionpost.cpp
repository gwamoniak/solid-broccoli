#include <QtTest>

#include "VisionPost.h"

// Ground truth for the detector's pure math: letterbox geometry must
// round-trip exactly, YOLO decoding must pick the right class from a
// hand-built tensor, and NMS must keep the right survivors. No ONNX
// Runtime, no model — these tests run on a bare machine.
class TstVisionPost : public QObject
{
    Q_OBJECT

private slots:
    void letterboxGeometryAsymmetric()
    {
        // 1280x720 into 640: scale by 0.5 -> 640x360, padY = 140.
        const auto m = VisionPost::computeLetterbox(1280, 720, 640);
        QCOMPARE(m.scale, 0.5);
        QCOMPARE(m.padX, 0.0);
        QCOMPARE(m.padY, 140.0);

        // Portrait: 720x1280 pads X instead.
        const auto p = VisionPost::computeLetterbox(720, 1280, 640);
        QCOMPARE(p.scale, 0.5);
        QCOMPARE(p.padX, 140.0);
        QCOMPARE(p.padY, 0.0);
    }

    void letterboxMappingRoundTrips()
    {
        const int srcW = 1280, srcH = 720;
        const auto m = VisionPost::computeLetterbox(srcW, srcH, 640);

        // A box occupying the center quarter of the source, pushed forward
        // into model space by hand, must map back to exactly that quarter.
        const QRectF sourceBox(320, 180, 640, 360);  // center 50% x 50%
        const QRectF modelBox(sourceBox.x() * m.scale + m.padX,
                              sourceBox.y() * m.scale + m.padY,
                              sourceBox.width() * m.scale,
                              sourceBox.height() * m.scale);
        const QRectF normalized = VisionPost::mapToSource(modelBox, m, srcW, srcH);
        QCOMPARE(normalized, QRectF(0.25, 0.25, 0.5, 0.5));
    }

    void mapToSourceClampsToFrame()
    {
        const auto m = VisionPost::computeLetterbox(1280, 720, 640);
        // A model box reaching into the top padding clamps to y = 0.
        const QRectF spillover(0.0, 100.0, 320.0, 200.0);  // pad is 140 deep
        const QRectF normalized = VisionPost::mapToSource(spillover, m, 1280, 720);
        QCOMPARE(normalized.top(), 0.0);
        QVERIFY(normalized.bottom() > 0.0);
        QVERIFY(QRectF(0, 0, 1, 1).contains(normalized));
    }

    void decodeYoloPicksBestClassAboveThreshold()
    {
        // Hand-built [1, 4+3, 2] tensor, channel-major: two anchors, three
        // classes. Anchor 0: class 1 at 0.9; anchor 1: best class 0.2 (below
        // the 0.25 threshold, dropped).
        //            anchor:   0      1
        const float data[] = {320.f, 100.f,   // cx
                              240.f, 100.f,   // cy
                              100.f,  50.f,   // w
                               80.f,  50.f,   // h
                               0.1f,  0.2f,   // class 0
                               0.9f,  0.1f,   // class 1
                               0.3f,  0.15f}; // class 2
        const auto candidates = VisionPost::decodeYolo(data, 7, 2, 0.25f);

        QCOMPARE(candidates.size(), 1);
        QCOMPARE(candidates.first().classId, 1);
        QCOMPARE(candidates.first().score, 0.9f);
        // cx/cy/w/h converted to corner form.
        QCOMPARE(candidates.first().box, QRectF(270.0, 200.0, 100.0, 80.0));
    }

    void nmsKeepsBestPerOverlapAndRespectsClasses()
    {
        using VisionPost::RawDetection;
        QVector<RawDetection> candidates;
        // Two heavily overlapping class-0 boxes: only the stronger survives.
        candidates.append({QRectF(100, 100, 100, 100), 0, 0.8f});
        candidates.append({QRectF(105, 105, 100, 100), 0, 0.7f});
        // Same place, different class: survives (class-wise suppression).
        candidates.append({QRectF(102, 102, 100, 100), 1, 0.6f});
        // Far away, same class: survives.
        candidates.append({QRectF(400, 400, 50, 50), 0, 0.5f});

        const auto kept = VisionPost::nonMaximumSuppression(candidates, 0.45);
        QCOMPARE(kept.size(), 3);
        QCOMPARE(kept[0].score, 0.8f);   // sorted by descending score
        QCOMPARE(kept[1].classId, 1);
        QCOMPARE(kept[2].box.x(), 400.0);
    }

    void iouOfDisjointAndIdenticalBoxes()
    {
        const QRectF a(0, 0, 10, 10);
        QCOMPARE(VisionPost::intersectionOverUnion(a, a), 1.0);
        QCOMPARE(VisionPost::intersectionOverUnion(a, QRectF(20, 20, 5, 5)), 0.0);
    }
};

QTEST_GUILESS_MAIN(TstVisionPost)
#include "tst_visionpost.moc"
