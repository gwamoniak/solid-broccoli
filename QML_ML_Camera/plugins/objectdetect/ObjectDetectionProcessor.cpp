#include "ObjectDetectionProcessor.h"

#include <array>
#include <vector>

#include <QDateTime>
#include <QFileInfo>
#include <QPainter>
#include <QThread>
#include <QThreadPool>

#include <onnxruntime_cxx_api.h>

#include "VisionPost.h"
#include "loggingcategories.h"

namespace {

// COCO-80, the class set of every stock YOLO export.
const char* const kCocoNames[] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train",
    "truck", "boat", "traffic light", "fire hydrant", "stop sign",
    "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag",
    "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite",
    "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon",
    "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
    "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant",
    "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote",
    "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"};
constexpr int kCocoCount = int(sizeof(kCocoNames) / sizeof(kCocoNames[0]));

QString classLabel(int classId)
{
    if (classId >= 0 && classId < kCocoCount)
        return QString::fromLatin1(kCocoNames[classId]);
    return QStringLiteral("class %1").arg(classId);
}

} // namespace

ObjectDetectionProcessor::ObjectDetectionProcessor() = default;

ObjectDetectionProcessor::~ObjectDetectionProcessor()
{
    // An in-flight inference references this object; wait it out.
    while (m_inferenceBusy.load())
        QThread::msleep(5);
}

QString ObjectDetectionProcessor::name() const
{
    return QStringLiteral("Object detection");
}

QString ObjectDetectionProcessor::description() const
{
    return QStringLiteral("Detects and labels objects in the preview "
                          "(YOLO ONNX model via ONNX Runtime).");
}

bool ObjectDetectionProcessor::initialize(QString*)
{
    // Always ready to pass frames through; the session loads lazily once a
    // model path is configured (Settings VISION section).
    return true;
}

void ObjectDetectionProcessor::setResultSink(std::function<void(const QVariantMap&)> sink)
{
    m_sink = std::move(sink);
}

void ObjectDetectionProcessor::configure(const QVariantMap& options)
{
    QMutexLocker locker(&m_configMutex);
    if (options.contains(QStringLiteral("modelPath"))) {
        const QString path = options.value(QStringLiteral("modelPath")).toString();
        if (path != m_modelPath) {
            m_modelPath = path;
            m_loadFailed = false;  // a new path earns a fresh load attempt
        }
    }
    if (options.contains(QStringLiteral("stride")))
        m_stride = qBound(1, options.value(QStringLiteral("stride")).toInt(), 10);
    if (options.contains(QStringLiteral("confidence")))
        m_confidenceThreshold = qBound(
            0.05f, float(options.value(QStringLiteral("confidence")).toDouble()), 0.9f);
}

bool ObjectDetectionProcessor::ensureSession()
{
    // Called on the inference thread only.
    QString path;
    {
        QMutexLocker locker(&m_configMutex);
        if (m_loadFailed || m_modelPath.isEmpty())
            return m_session && m_loadedModelPath == m_modelPath;
        path = m_modelPath;
    }
    if (m_session && m_loadedModelPath == path)
        return true;

    if (!QFileInfo::exists(path)) {
        qWarning(logWarning()) << "ObjectDetection: model file missing:" << path;
        QMutexLocker locker(&m_configMutex);
        m_loadFailed = true;
        return false;
    }

    try {
        if (!m_env)
            m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING,
                                               "solid-broccoli");
        Ort::SessionOptions options;
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        m_session = std::make_unique<Ort::Session>(*m_env, path.toUtf8().constData(),
                                                   options);

        // Model input is [1, 3, S, S]; read S so 320/640 exports both work.
        const auto shape =
            m_session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
        if (shape.size() == 4 && shape[2] > 0)
            m_modelSize = int(shape[2]);
        m_loadedModelPath = path;
        qDebug(logInfo()) << "ObjectDetection: model loaded:" << path
                          << "input" << m_modelSize << "px";
        return true;
    } catch (const Ort::Exception& e) {
        qWarning(logCritical()) << "ObjectDetection: model load failed:" << e.what();
        m_session.reset();
        QMutexLocker locker(&m_configMutex);
        m_loadFailed = true;
        return false;
    }
}

void ObjectDetectionProcessor::runInference(const QImage& frame)
{
    if (!ensureSession()) {
        m_inferenceBusy.store(false);
        return;
    }

    float confidence;
    {
        QMutexLocker locker(&m_configMutex);
        confidence = m_confidenceThreshold;
    }

    try {
        // Letterbox into the model square, RGB float CHW normalized to [0,1].
        const auto mapping =
            VisionPost::computeLetterbox(frame.width(), frame.height(), m_modelSize);
        QImage square(m_modelSize, m_modelSize, QImage::Format_RGB888);
        square.fill(Qt::black);
        {
            QPainter painter(&square);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawImage(QRectF(mapping.padX, mapping.padY,
                                     frame.width() * mapping.scale,
                                     frame.height() * mapping.scale),
                              frame);
        }

        std::vector<float> input(size_t(3) * m_modelSize * m_modelSize);
        const int plane = m_modelSize * m_modelSize;
        for (int y = 0; y < m_modelSize; ++y) {
            const uchar* line = square.constScanLine(y);
            for (int x = 0; x < m_modelSize; ++x) {
                const int p = y * m_modelSize + x;
                input[size_t(p)] = line[x * 3 + 0] / 255.0f;
                input[size_t(plane + p)] = line[x * 3 + 1] / 255.0f;
                input[size_t(2 * plane + p)] = line[x * 3 + 2] / 255.0f;
            }
        }

        const std::array<int64_t, 4> inputShape{1, 3, m_modelSize, m_modelSize};
        Ort::MemoryInfo memory =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memory, input.data(), input.size(), inputShape.data(), inputShape.size());

        Ort::AllocatorWithDefaultOptions allocator;
        const auto inputName = m_session->GetInputNameAllocated(0, allocator);
        const auto outputName = m_session->GetOutputNameAllocated(0, allocator);
        const char* inputNames[] = {inputName.get()};
        const char* outputNames[] = {outputName.get()};

        auto outputs = m_session->Run(Ort::RunOptions{nullptr}, inputNames,
                                      &inputTensor, 1, outputNames, 1);

        const auto outShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
        if (outShape.size() != 3) {
            qWarning(logWarning()) << "ObjectDetection: unexpected output rank"
                                   << int(outShape.size());
            m_inferenceBusy.store(false);
            return;
        }
        const int channels = int(outShape[1]);
        const int anchors = int(outShape[2]);
        const float* data = outputs[0].GetTensorData<float>();

        auto candidates = VisionPost::decodeYolo(data, channels, anchors, confidence);
        const auto winners = VisionPost::nonMaximumSuppression(std::move(candidates), 0.45);

        QVector<Detection> detections;
        detections.reserve(winners.size());
        for (const auto& raw : winners) {
            Detection d;
            d.box = VisionPost::mapToSource(raw.box, mapping,
                                            frame.width(), frame.height());
            if (d.box.isEmpty())
                continue;
            d.label = classLabel(raw.classId);
            d.confidence = raw.score;
            detections.append(d);
        }

        {
            QMutexLocker locker(&m_resultMutex);
            m_latest = detections;
        }
        if (m_sink)
            m_sink(detectionsResult(detections,
                                    QDateTime::currentMSecsSinceEpoch()));
    } catch (const Ort::Exception& e) {
        qWarning(logCritical()) << "ObjectDetection: inference failed:" << e.what();
    }
    m_inferenceBusy.store(false);
}

void ObjectDetectionProcessor::drawDetections(QImage& frame) const
{
    QVector<Detection> detections;
    {
        QMutexLocker locker(&m_resultMutex);
        detections = m_latest;
    }
    if (detections.isEmpty())
        return;

    QPainter painter(&frame);
    painter.setRenderHint(QPainter::Antialiasing);
    const int fontPx = std::max(11, frame.height() / 40);
    QFont font = painter.font();
    font.setPixelSize(fontPx);
    painter.setFont(font);

    for (const Detection& d : detections) {
        const QRectF box(d.box.x() * frame.width(), d.box.y() * frame.height(),
                         d.box.width() * frame.width(),
                         d.box.height() * frame.height());
        painter.setPen(QPen(QColor(0xFF, 0xE1, 0x00), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(box);

        const QString text = QStringLiteral("%1 %2%")
                                 .arg(d.label)
                                 .arg(qRound(d.confidence * 100));
        const int textWidth = painter.fontMetrics().horizontalAdvance(text);
        QRectF chip(box.left(), box.top() - fontPx - 8, textWidth + 12, fontPx + 8);
        if (chip.top() < 0)
            chip.moveTop(box.top());
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0xFF, 0xE1, 0x00));
        painter.drawRect(chip);
        painter.setPen(QColor(0, 0, 0));
        painter.drawText(chip, Qt::AlignCenter, text);
    }
}

QImage ObjectDetectionProcessor::process(const QImage& frame)
{
    if (frame.isNull())
        return frame;

    int stride;
    bool configured;
    {
        QMutexLocker locker(&m_configMutex);
        stride = m_stride;
        configured = !m_modelPath.isEmpty() && !m_loadFailed;
    }

    // Submit every Nth frame to the pool; never block the camera worker.
    ++m_frameCounter;
    if (configured && m_frameCounter % quint64(stride) == 0
        && !m_inferenceBusy.exchange(true)) {
        const QImage copy =
            frame.convertToFormat(QImage::Format_RGB888);  // deep, thread-safe copy
        QThreadPool::globalInstance()->start(
            [this, copy]() { runInference(copy); });
    }

    QImage out = frame;
    drawDetections(out);
    return out;
}
