#ifndef GRAYSCALEPROCESSOR_H
#define GRAYSCALEPROCESSOR_H

#include <QObject>

#include "FrameProcessor.h"

// Proof-of-contract plugin: converts every preview frame to grayscale. It
// proves the loading/processing pipeline that a future opencv- or yolo- based
// plugin will follow; those are out of scope here. Built as a MODULE library so
// QPluginLoader can load it at runtime.
class GrayscaleProcessor : public QObject, public FrameProcessor
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FrameProcessor_iid)
    Q_INTERFACES(FrameProcessor)

public:
    QString name() const override;
    QString description() const override;
    bool initialize(QString* errorMessage) override;
    QImage process(const QImage& frame) override;
};

#endif // GRAYSCALEPROCESSOR_H
