#include "GrayscaleProcessor.h"

QString GrayscaleProcessor::name() const
{
    return QStringLiteral("Grayscale");
}

QString GrayscaleProcessor::description() const
{
    return QStringLiteral("Converts the live preview to black and white.");
}

bool GrayscaleProcessor::initialize(QString* /*errorMessage*/)
{
    // Nothing to load or allocate; always ready.
    return true;
}

QImage GrayscaleProcessor::process(const QImage& frame)
{
    // Convert back to RGBA8888 so the pipeline always hands the camera output
    // sink a frame in a format it can wrap (Format_Grayscale8 alone is fine for
    // QImage but keeps the conversion explicit and predictable).
    return frame.convertToFormat(QImage::Format_Grayscale8)
                .convertToFormat(QImage::Format_RGBA8888);
}
