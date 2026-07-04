#include "SpectrumOverlayProcessor.h"

#include <algorithm>

#include <QDateTime>
#include <QPainter>
#include <QPainterPath>

SpectrumOverlayProcessor::SpectrumOverlayProcessor(SnapshotGetter snapshot)
    : m_snapshot(std::move(snapshot))
{
}

QString SpectrumOverlayProcessor::name() const
{
    return QStringLiteral("Spectrum overlay");
}

QString SpectrumOverlayProcessor::description() const
{
    return QStringLiteral("Draws the live spectrum onto recorded video frames.");
}

bool SpectrumOverlayProcessor::initialize(QString*)
{
    return true;
}

QImage SpectrumOverlayProcessor::process(const QImage& frame)
{
    const Spectrum spectrum = m_snapshot ? m_snapshot() : Spectrum();
    if (!spectrum.isValid() || frame.isNull())
        return frame;

    QImage out = frame;  // detaches on paint
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);

    // Translucent instrument panel across the lower third.
    const int margin = out.width() / 60 + 4;
    const QRect panel(margin, out.height() * 2 / 3,
                      out.width() - 2 * margin, out.height() / 3 - margin);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(10, 10, 12, 170));
    painter.drawRoundedRect(panel, 10, 10);

    const int fontPx = std::max(10, panel.height() / 9);
    QFont font = painter.font();
    font.setPixelSize(fontPx);
    painter.setFont(font);

    const QRect plot = panel.adjusted(12, fontPx + 8, -12, -(fontPx + 8));
    const double maxCount = *std::max_element(spectrum.counts.cbegin(),
                                              spectrum.counts.cend());
    const double minCount = *std::min_element(spectrum.counts.cbegin(),
                                              spectrum.counts.cend());
    const double range = std::max(maxCount - minCount, 1e-9);

    // Trace, decimated to roughly one point per pixel column.
    const int n = spectrum.counts.size();
    const int stride = std::max(1, n / std::max(plot.width(), 1));
    QPainterPath path;
    int peakIndex = 0;
    for (int i = 0; i < n; i += stride) {
        if (spectrum.counts[i] > spectrum.counts[peakIndex])
            peakIndex = i;
        const double x = plot.left() + double(i) / (n - 1) * plot.width();
        const double y = plot.bottom()
                         - (spectrum.counts[i] - minCount) / range * plot.height();
        if (i == 0)
            path.moveTo(x, y);
        else
            path.lineTo(x, y);
    }
    painter.setPen(QPen(QColor(0xFF, 0xE1, 0x00), 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    // Readouts: peak wavelength (yellow), wavelength range, timestamp.
    painter.setPen(QColor(0xFF, 0xE1, 0x00));
    painter.drawText(QPoint(plot.left(), panel.top() + fontPx + 4),
                     QStringLiteral("λ peak %1 nm")
                         .arg(spectrum.wavelengthsNm[peakIndex], 0, 'f', 1));
    painter.setPen(QColor(0xF2, 0xF2, 0xF2));
    painter.drawText(QPoint(plot.left(), panel.bottom() - 6),
                     QString::number(spectrum.wavelengthsNm.first(), 'f', 0));
    const QString maxLabel = QString::number(spectrum.wavelengthsNm.last(), 'f', 0);
    painter.drawText(QPoint(plot.right() - painter.fontMetrics()
                                               .horizontalAdvance(maxLabel),
                            panel.bottom() - 6),
                     maxLabel);
    const QString stamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    painter.drawText(QPoint(panel.right() - 12
                                - painter.fontMetrics().horizontalAdvance(stamp),
                            panel.top() + fontPx + 4),
                     stamp);
    return out;
}
