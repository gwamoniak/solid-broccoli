#ifndef DETECTION_H
#define DETECTION_H

#include <QRectF>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

// One detected object, as published by an analyzing frame processor through
// the v1.1 result sink. The box is normalized to [0,1] frame coordinates so
// consumers never need the processed frame's pixel size.
struct Detection
{
    QRectF box;
    QString label;
    float confidence = 0.0f;

    QVariantMap toVariant() const
    {
        return {{QStringLiteral("box"), box},
                {QStringLiteral("label"), label},
                {QStringLiteral("confidence"), double(confidence)}};
    }

    static Detection fromVariant(const QVariantMap& map)
    {
        Detection d;
        d.box = map.value(QStringLiteral("box")).toRectF();
        d.label = map.value(QStringLiteral("label")).toString();
        d.confidence = float(map.value(QStringLiteral("confidence")).toDouble());
        return d;
    }
};

// Result-map conventions for the sink (see FrameProcessor::setResultSink):
// {"type": "detections", "timestampMs": qint64, "detections": QVariantList}.
inline QVariantMap detectionsResult(const QVector<Detection>& detections,
                                    qint64 timestampMs)
{
    QVariantList list;
    list.reserve(detections.size());
    for (const Detection& d : detections)
        list.append(d.toVariant());
    return {{QStringLiteral("type"), QStringLiteral("detections")},
            {QStringLiteral("timestampMs"), timestampMs},
            {QStringLiteral("detections"), list}};
}

#endif // DETECTION_H
