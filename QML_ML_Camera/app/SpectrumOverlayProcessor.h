#ifndef SPECTRUMOVERLAYPROCESSOR_H
#define SPECTRUMOVERLAYPROCESSOR_H

#include <functional>

#include "FrameProcessor.h"
#include "Spectrum.h"

// App-internal frame processor (constructed in main, not a loaded plugin)
// that burns the live spectrum into camera frames before encoding — the
// documentation overlay. process() runs on the camera worker thread; the
// snapshot getter is the single cross-thread touch point and must return a
// mutex-guarded copy (SpectrometerService::latestSpectrumSnapshot). With no
// valid spectrum the frame passes through untouched.
class SpectrumOverlayProcessor : public FrameProcessor
{
public:
    using SnapshotGetter = std::function<Spectrum()>;

    explicit SpectrumOverlayProcessor(SnapshotGetter snapshot);

    QString name() const override;
    QString description() const override;
    bool initialize(QString* errorMessage) override;
    QImage process(const QImage& frame) override;

private:
    SnapshotGetter m_snapshot;
};

#endif // SPECTRUMOVERLAYPROCESSOR_H
