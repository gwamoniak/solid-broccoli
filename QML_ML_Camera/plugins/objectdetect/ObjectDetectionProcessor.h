#ifndef OBJECTDETECTIONPROCESSOR_H
#define OBJECTDETECTIONPROCESSOR_H

#include <atomic>
#include <functional>
#include <memory>

#include <QMutex>
#include <QObject>
#include <QVariantMap>

#include "Detection.h"
#include "FrameProcessor.h"

namespace Ort {
class Env;
class Session;
}

// Object detection on the live preview: a YOLO ONNX model (v8/v11 export,
// 640 px input) through ONNX Runtime. The application never links ORT —
// that is the point of the plugin boundary.
//
// Threading: process() runs on the camera worker and must never block on
// the model. Every Nth frame (configurable stride) is copied to a pool
// thread for inference; every frame gets the most recent completed result
// drawn onto it, so preview rate and detection rate are decoupled.
// Detections are also published as data through the v1.1 result sink.
class ObjectDetectionProcessor : public QObject, public FrameProcessor
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FrameProcessor_iid)
    Q_INTERFACES(FrameProcessor)

public:
    ObjectDetectionProcessor();
    ~ObjectDetectionProcessor() override;

    QString name() const override;
    QString description() const override;
    bool initialize(QString* errorMessage) override;
    QImage process(const QImage& frame) override;
    void setResultSink(std::function<void(const QVariantMap&)> sink) override;
    void configure(const QVariantMap& options) override;

private:
    bool ensureSession();  // lazy, one attempt per configured path
    void runInference(const QImage& frame);
    void drawDetections(QImage& frame) const;

    // Configuration (written from the GUI thread via configure, read on the
    // camera worker; guarded by m_configMutex).
    mutable QMutex m_configMutex;
    QString m_modelPath;
    int m_stride = 3;
    float m_confidenceThreshold = 0.25f;

    std::unique_ptr<Ort::Env> m_env;
    std::unique_ptr<Ort::Session> m_session;
    QString m_loadedModelPath;   // session built for this path
    bool m_loadFailed = false;   // one loud failure, then passthrough
    int m_modelSize = 640;

    quint64 m_frameCounter = 0;
    std::atomic<bool> m_inferenceBusy{false};

    mutable QMutex m_resultMutex;
    QVector<Detection> m_latest;

    std::function<void(const QVariantMap&)> m_sink;
};

#endif // OBJECTDETECTIONPROCESSOR_H
