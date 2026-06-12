#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QImage>
#include <QString>
#include <QtPlugin>

// A "frame-processor plugin" is a shared library, loaded at runtime, that
// receives each live preview frame and may return a modified image. This is
// the extension point where image filters or neural-network inference live
// WITHOUT the application itself linking those libraries: the app links only
// this header-only interface, and OpenCV / deep-learning code stays inside the
// plugin.
//
// process() is called once per preview frame on a dedicated worker thread (not
// the GUI thread), so implementations must be self-contained and must not touch
// GUI objects. Frames are exchanged as QImage. A plugin that wants to use
// OpenCV converts in-place without copying:
//
//     cv::Mat mat(frame.height(), frame.width(), CV_8UC4,
//                 const_cast<uchar*>(frame.bits()), frame.bytesPerLine());
//
// (use QImage::Format_RGBA8888 / CV_8UC4 to match channel order), processes the
// cv::Mat, and returns a QImage wrapping the result. No other reference is
// needed to author a plugin.
class FrameProcessor
{
public:
    virtual ~FrameProcessor() = default;

    // Human-readable identity shown in the UI; name() must be stable because it
    // is also the key under which the enabled state is persisted.
    virtual QString name() const = 0;
    virtual QString description() const = 0;

    // Called once after loading. Use it to load models or allocate buffers.
    // Return false and set *errorMessage to refuse activation.
    virtual bool initialize(QString* errorMessage) = 0;

    // Called per frame on a worker thread. Return the transformed image; return
    // the input unchanged to act as a pass-through.
    virtual QImage process(const QImage& frame) = 0;
};

#define FrameProcessor_iid "broccoli.FrameProcessor/1.0"
Q_DECLARE_INTERFACE(FrameProcessor, FrameProcessor_iid)

#endif // FRAMEPROCESSOR_H
