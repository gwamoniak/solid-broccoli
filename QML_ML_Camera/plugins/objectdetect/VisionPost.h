#ifndef VISIONPOST_H
#define VISIONPOST_H

#include <QRectF>
#include <QVector>

// Pure pre/post-processing math for YOLO-style detectors: letterbox
// geometry, output decoding, and non-maximum suppression. Deliberately free
// of ONNX Runtime (and of any I/O) so every function ground-truth tests on
// machines without the library or a model — the sans-IO discipline applied
// to inference.
namespace VisionPost {

// Geometry of an aspect-preserving resize into a square model input with
// symmetric padding ("letterbox").
struct LetterboxMapping
{
    double scale = 1.0;   // source pixels -> model pixels
    double padX = 0.0;    // left padding in model pixels
    double padY = 0.0;    // top padding in model pixels
};

LetterboxMapping computeLetterbox(int sourceWidth, int sourceHeight, int modelSize);

// Model-space box (pixels, corner form) back to normalized [0,1] source
// coordinates, clamped to the frame.
QRectF mapToSource(const QRectF& modelBox, const LetterboxMapping& mapping,
                   int sourceWidth, int sourceHeight);

// One raw candidate before NMS: box in model pixels (corner form).
struct RawDetection
{
    QRectF box;
    int classId = -1;
    float score = 0.0f;
};

// Decodes the YOLOv8/v11 output layout [1, 4+numClasses, anchors]
// (channel-major: all cx, then all cy, ...): per anchor, the best class
// above the confidence threshold becomes a candidate; cx/cy/w/h are model
// pixels and convert to corner form here.
QVector<RawDetection> decodeYolo(const float* data, int channels, int anchors,
                                 float confidenceThreshold);

double intersectionOverUnion(const QRectF& a, const QRectF& b);

// Class-wise greedy NMS: candidates sorted by descending score; a candidate
// is dropped when it overlaps a kept candidate of the same class above the
// IoU threshold. Returned survivors remain sorted by descending score.
QVector<RawDetection> nonMaximumSuppression(QVector<RawDetection> candidates,
                                            double iouThreshold);

} // namespace VisionPost

#endif // VISIONPOST_H
