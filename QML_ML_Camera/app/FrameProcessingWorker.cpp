#include "FrameProcessingWorker.h"

#include <QMutexLocker>

#include "FrameProcessor.h"

FrameProcessingWorker::FrameProcessingWorker(QObject* parent)
    : QObject(parent)
{
}

void FrameProcessingWorker::setProcessors(const QList<FrameProcessor*>& processors)
{
    QMutexLocker locker(&m_mutex);
    m_processors = processors;
}

void FrameProcessingWorker::process(const QVideoFrame& frame)
{
    QList<FrameProcessor*> processors;
    {
        QMutexLocker locker(&m_mutex);
        processors = m_processors;
    }

    QImage result = frame.toImage();
    for (FrameProcessor* processor : processors) {
        result = processor->process(result);
    }
    emit processed(result);
}
