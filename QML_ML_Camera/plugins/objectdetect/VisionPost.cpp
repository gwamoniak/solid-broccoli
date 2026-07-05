#include "VisionPost.h"

#include <algorithm>

namespace VisionPost {

LetterboxMapping computeLetterbox(int sourceWidth, int sourceHeight, int modelSize)
{
    LetterboxMapping mapping;
    if (sourceWidth <= 0 || sourceHeight <= 0 || modelSize <= 0)
        return mapping;
    mapping.scale = double(modelSize) / std::max(sourceWidth, sourceHeight);
    mapping.padX = (modelSize - sourceWidth * mapping.scale) / 2.0;
    mapping.padY = (modelSize - sourceHeight * mapping.scale) / 2.0;
    return mapping;
}

QRectF mapToSource(const QRectF& modelBox, const LetterboxMapping& mapping,
                   int sourceWidth, int sourceHeight)
{
    if (sourceWidth <= 0 || sourceHeight <= 0 || mapping.scale <= 0.0)
        return QRectF();
    const double left = (modelBox.left() - mapping.padX) / mapping.scale;
    const double top = (modelBox.top() - mapping.padY) / mapping.scale;
    const double width = modelBox.width() / mapping.scale;
    const double height = modelBox.height() / mapping.scale;

    QRectF normalized(left / sourceWidth, top / sourceHeight,
                      width / sourceWidth, height / sourceHeight);
    return normalized.intersected(QRectF(0.0, 0.0, 1.0, 1.0));
}

QVector<RawDetection> decodeYolo(const float* data, int channels, int anchors,
                                 float confidenceThreshold)
{
    QVector<RawDetection> candidates;
    if (!data || channels < 5 || anchors <= 0)
        return candidates;

    const int numClasses = channels - 4;
    const auto at = [&](int channel, int anchor) {
        return data[channel * anchors + anchor];  // channel-major layout
    };

    for (int a = 0; a < anchors; ++a) {
        int bestClass = -1;
        float bestScore = confidenceThreshold;
        for (int c = 0; c < numClasses; ++c) {
            const float score = at(4 + c, a);
            if (score > bestScore) {
                bestScore = score;
                bestClass = c;
            }
        }
        if (bestClass < 0)
            continue;

        const float cx = at(0, a);
        const float cy = at(1, a);
        const float w = at(2, a);
        const float h = at(3, a);
        RawDetection d;
        d.box = QRectF(cx - w / 2.0, cy - h / 2.0, w, h);
        d.classId = bestClass;
        d.score = bestScore;
        candidates.append(d);
    }
    return candidates;
}

double intersectionOverUnion(const QRectF& a, const QRectF& b)
{
    const QRectF inter = a.intersected(b);
    const double interArea = inter.width() * inter.height();
    const double unionArea = a.width() * a.height() + b.width() * b.height() - interArea;
    return unionArea > 0.0 ? interArea / unionArea : 0.0;
}

QVector<RawDetection> nonMaximumSuppression(QVector<RawDetection> candidates,
                                            double iouThreshold)
{
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const RawDetection& a, const RawDetection& b) {
                         return a.score > b.score;
                     });

    QVector<RawDetection> kept;
    for (const RawDetection& candidate : candidates) {
        bool suppressed = false;
        for (const RawDetection& winner : kept) {
            if (winner.classId == candidate.classId
                && intersectionOverUnion(winner.box, candidate.box) > iouThreshold) {
                suppressed = true;
                break;
            }
        }
        if (!suppressed)
            kept.append(candidate);
    }
    return kept;
}

} // namespace VisionPost
