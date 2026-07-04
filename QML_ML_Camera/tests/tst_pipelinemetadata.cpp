#include <QSignalSpy>
#include <QtTest>

#include "Detection.h"
#include "DetectionModel.h"
#include "FrameProcessor.h"

// The v1.1 result channel end-to-end without a real model: a fake analyzing
// processor publishes through the sink from a worker thread (as the ONNX
// plugin does), delivery to DetectionModel is queued onto the GUI thread,
// and pixel-only v1.0-style processors are untouched by the new interface.
class TstPipelineMetadata : public QObject
{
    Q_OBJECT

private:
    // Publishes one fixed detection per processed frame — the minimal
    // analyzing processor.
    class FakeAnalyzer : public FrameProcessor
    {
    public:
        QString name() const override { return QStringLiteral("Fake analyzer"); }
        QString description() const override { return QString(); }
        bool initialize(QString*) override { return true; }
        void setResultSink(std::function<void(const QVariantMap&)> sink) override
        {
            m_sink = std::move(sink);
        }
        QImage process(const QImage& frame) override
        {
            if (m_sink) {
                Detection d;
                d.box = QRectF(0.1, 0.2, 0.3, 0.4);
                d.label = QStringLiteral("broccoli");
                d.confidence = 0.93f;
                m_sink(detectionsResult({d}, 42));
            }
            return frame;
        }

    private:
        std::function<void(const QVariantMap&)> m_sink;
    };

    // A v1.0-style pixel processor: overrides nothing from v1.1.
    class PlainProcessor : public FrameProcessor
    {
    public:
        QString name() const override { return QStringLiteral("Plain"); }
        QString description() const override { return QString(); }
        bool initialize(QString*) override { return true; }
        QImage process(const QImage& frame) override { return frame; }
    };

private slots:
    void detectionRoundTripsThroughVariantMap()
    {
        Detection d;
        d.box = QRectF(0.25, 0.25, 0.5, 0.5);
        d.label = QStringLiteral("cup");
        d.confidence = 0.75f;
        const Detection back = Detection::fromVariant(d.toVariant());
        QCOMPARE(back.box, d.box);
        QCOMPARE(back.label, d.label);
        QCOMPARE(back.confidence, d.confidence);
    }

    void sinkDeliversQueuedToTheModel()
    {
        DetectionModel model;
        QSignalSpy countSpy(&model, &DetectionModel::countChanged);

        FakeAnalyzer analyzer;
        // The main.cpp wiring pattern: queued delivery to the GUI thread.
        analyzer.setResultSink([&model](const QVariantMap& result) {
            QMetaObject::invokeMethod(
                &model, [&model, result]() { model.updateFromResult(result); },
                Qt::QueuedConnection);
        });

        QImage frame(64, 48, QImage::Format_RGBA8888);
        frame.fill(Qt::black);
        QCOMPARE(analyzer.process(frame), frame);  // pixels untouched

        QCOMPARE(model.rowCount(), 0);  // queued: nothing until the loop spins
        QTRY_COMPARE(model.rowCount(), 1);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(model.data(model.index(0), DetectionModel::LabelRole).toString(),
                 QStringLiteral("broccoli"));
        QCOMPARE(model.data(model.index(0), DetectionModel::ConfidenceTextRole)
                     .toString(),
                 QStringLiteral("93%"));

        // Thread-safe snapshot for the Milestone 13 report generator.
        const auto snapshot = model.latestDetections();
        QCOMPARE(snapshot.size(), 1);
        QCOMPARE(snapshot.first().box, QRectF(0.1, 0.2, 0.3, 0.4));

        model.clear();
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.latestDetections().size(), 0);
    }

    void publishingFromAWorkerThreadIsSafe()
    {
        DetectionModel model;
        FakeAnalyzer analyzer;
        analyzer.setResultSink([&model](const QVariantMap& result) {
            QMetaObject::invokeMethod(
                &model, [&model, result]() { model.updateFromResult(result); },
                Qt::QueuedConnection);
        });

        // Run process() off the GUI thread, exactly like the camera worker.
        QImage frame(32, 32, QImage::Format_RGBA8888);
        QThread* thread = QThread::create([&analyzer, frame]() {
            analyzer.process(frame);
        });
        thread->start();
        QVERIFY(thread->wait(2000));
        delete thread;

        QTRY_COMPARE(model.rowCount(), 1);
    }

    void plainProcessorIgnoresTheNewInterface()
    {
        PlainProcessor plain;
        // Default no-ops must be callable and harmless.
        plain.setResultSink([](const QVariantMap&) { QFAIL("must never fire"); });
        plain.configure({{QStringLiteral("modelPath"), QStringLiteral("/nope")}});

        QImage frame(16, 16, QImage::Format_RGBA8888);
        frame.fill(Qt::red);
        QCOMPARE(plain.process(frame), frame);
    }

    void foreignResultTypesAreIgnored()
    {
        DetectionModel model;
        model.updateFromResult({{QStringLiteral("type"), QStringLiteral("histogram")}});
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_MAIN(TstPipelineMetadata)
#include "tst_pipelinemetadata.moc"
