#ifndef FRAMEPROCESSINGWORKER_H
#define FRAMEPROCESSINGWORKER_H

#include <QImage>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QVideoFrame>

class FrameProcessor;

// Runs the enabled frame processors on a dedicated thread so per-frame work
// never blocks the GUI. CameraService moves one instance onto a QThread and
// feeds it the latest preview frame via process(); the result comes back on the
// GUI thread through processed(). The QVideoFrame-to-QImage conversion happens
// here too: it can involve a GPU readback, the most expensive step besides the
// processors themselves, so it must not run on the GUI thread. The processor
// list is swapped under a mutex because it is written from the GUI thread and
// read here on the worker thread.
class FrameProcessingWorker : public QObject
{
    Q_OBJECT
public:
    explicit FrameProcessingWorker(QObject* parent = nullptr);

    void setProcessors(const QList<FrameProcessor*>& processors);

public slots:
    void process(const QVideoFrame& frame);

signals:
    void processed(const QImage& result);

private:
    QList<FrameProcessor*> m_processors;
    mutable QMutex m_mutex;
};

#endif // FRAMEPROCESSINGWORKER_H
