#include "SpectrumView.h"

#include <algorithm>
#include <cmath>

#include <QSGFlatColorMaterial>
#include <QSGGeometryNode>
#include <QSGNode>
#include <QHoverEvent>
#include <QWheelEvent>

#include "PeakMatcher.h"
#include "SpectrometerService.h"

namespace {

constexpr double kMinSpanNm = 5.0;

} // namespace

SpectrumView::SpectrumView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    setClip(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
}

void SpectrumView::setSource(SpectrometerService* source)
{
    if (source == m_source)
        return;
    if (m_source)
        disconnect(m_source, nullptr, this, nullptr);
    m_source = source;
    if (m_source) {
        connect(m_source, &SpectrometerService::spectrumUpdated,
                this, &SpectrumView::onSpectrumUpdated);
        connect(m_source, &SpectrometerService::calibrationChanged,
                this, &SpectrumView::onSpectrumUpdated);
        connect(m_source, &SpectrometerService::modeChanged, this, [this]() {
            // The value scale changes meaning between counts/T/A.
            if (m_autoScaleY)
                m_maxY = 0.0;
        });
        connect(m_source, &SpectrometerService::overlaysChanged, this, [this]() {
            m_overlays = m_source->overlaySpectra();
            update();
        });
        m_overlays = m_source->overlaySpectra();
    }
    emit sourceChanged();
}

void SpectrumView::onSpectrumUpdated()
{
    if (!m_source)
        return;
    m_live = m_source->displaySpectrum();
    m_reference = m_source->referenceSpectrum();
    m_dark = m_source->darkSpectrum();

    bool xChanged = false;
    if (!m_userZoomed && m_live.isValid()
        && (m_minX != m_live.wavelengthsNm.first()
            || m_maxX != m_live.wavelengthsNm.last())) {
        m_minX = m_live.wavelengthsNm.first();
        m_maxX = m_live.wavelengthsNm.last();
        xChanged = true;
    }
    const double oldMin = m_minY;
    const double oldMax = m_maxY;
    updateYScale();
    if (xChanged)
        emit viewRangeChanged();
    if (m_minY != oldMin || m_maxY != oldMax)
        emit yScaleChanged();
    update();
}

void SpectrumView::updateYScale()
{
    if (!m_autoScaleY || !m_live.isValid())
        return;

    double lo = 0.0;
    double hi = 0.0;
    bool any = false;
    for (int i = 0; i < m_live.counts.size(); ++i) {
        const double nm = m_live.wavelengthsNm[i];
        if (nm < m_minX || nm > m_maxX)
            continue;
        lo = std::min(lo, m_live.counts[i]);
        hi = std::max(hi, m_live.counts[i]);
        any = true;
    }
    if (!any)
        return;

    const double targetMax = hi * 1.08;
    // Jump up instantly, decay down slowly, so the trace never clips but the
    // scale doesn't jitter with per-frame noise.
    m_maxY = targetMax > m_maxY ? targetMax : std::max(targetMax, m_maxY * 0.95);
    m_minY = lo < 0.0 ? lo * 1.08 : 0.0;
    if (m_maxY - m_minY < 1e-9)
        m_maxY = m_minY + 1.0;
}

double SpectrumView::dataMinX() const
{
    return m_live.isValid() ? m_live.wavelengthsNm.first() : 340.0;
}

double SpectrumView::dataMaxX() const
{
    return m_live.isValid() ? m_live.wavelengthsNm.last() : 1020.0;
}

double SpectrumView::wavelengthToX(double nm) const
{
    if (m_maxX <= m_minX)
        return 0.0;
    return (nm - m_minX) / (m_maxX - m_minX) * width();
}

double SpectrumView::nmAtX(double x) const
{
    if (width() <= 0)
        return m_minX;
    return m_minX + x / width() * (m_maxX - m_minX);
}

double SpectrumView::valueToY(double value) const
{
    if (m_maxY <= m_minY)
        return 0.0;
    return height() - (value - m_minY) / (m_maxY - m_minY) * height();
}

void SpectrumView::setIntegrationFromNm(double nm)
{
    m_integrationFromNm = nm;
    emit integrationRegionChanged();
    update();
}

void SpectrumView::setIntegrationToNm(double nm)
{
    m_integrationToNm = nm;
    emit integrationRegionChanged();
    update();
}

QVariantList SpectrumView::tickWavelengths() const
{
    QVariantList ticks;
    const double span = m_maxX - m_minX;
    if (span <= 0)
        return ticks;

    // Adapt the step so 4-12 labels are visible at any zoom.
    double step = m_xTickStep;
    while (span / step > 12.0)
        step *= 2.0;
    while (span / step < 4.0 && step > 1.0)
        step /= 2.0;

    for (double nm = std::ceil(m_minX / step) * step; nm <= m_maxX; nm += step)
        ticks.append(nm);
    return ticks;
}

void SpectrumView::setAutoScaleY(bool autoScale)
{
    if (autoScale == m_autoScaleY)
        return;
    m_autoScaleY = autoScale;
    emit autoScaleYChanged();
    update();
}

void SpectrumView::setXAxisTitle(const QString& title)
{
    if (title == m_xAxisTitle)
        return;
    m_xAxisTitle = title;
    emit xAxisTitleChanged();
}

void SpectrumView::setXTickStep(double step)
{
    if (step <= 0 || step == m_xTickStep)
        return;
    m_xTickStep = step;
    emit xTickStepChanged();
    emit viewRangeChanged();
    emit yScaleChanged();
}

void SpectrumView::setShowReference(bool show)
{
    if (show == m_showReference)
        return;
    m_showReference = show;
    emit traceVisibilityChanged();
    update();
}

void SpectrumView::setShowDark(bool show)
{
    if (show == m_showDark)
        return;
    m_showDark = show;
    emit traceVisibilityChanged();
    update();
}

void SpectrumView::centerOn(double nm)
{
    const double span = std::max(std::min(m_maxX - m_minX, 100.0), kMinSpanNm * 4);
    m_minX = nm - span / 2.0;
    m_maxX = nm + span / 2.0;
    m_userZoomed = true;
    clampView();
    updateYScale();
    emit viewRangeChanged();
    emit yScaleChanged();
    update();
}

void SpectrumView::resetView()
{
    m_minX = dataMinX();
    m_maxX = dataMaxX();
    m_userZoomed = false;
    m_maxY = 0.0;
    setAutoScaleY(true);
    updateYScale();
    emit viewRangeChanged();
    emit yScaleChanged();
    update();
}

void SpectrumView::zoomAround(double centerNm, double factor)
{
    const double span = m_maxX - m_minX;
    const double fullSpan = dataMaxX() - dataMinX();
    const double newSpan = std::clamp(span * factor, kMinSpanNm, fullSpan);
    const double rel = span > 0 ? (centerNm - m_minX) / span : 0.5;
    m_minX = centerNm - rel * newSpan;
    m_maxX = m_minX + newSpan;
    m_userZoomed = newSpan < fullSpan - 1e-9;
    clampView();
    updateYScale();
    emit viewRangeChanged();
    emit yScaleChanged();
    update();
}

void SpectrumView::panByNm(double deltaNm)
{
    m_minX += deltaNm;
    m_maxX += deltaNm;
    m_userZoomed = true;
    clampView();
    updateYScale();
    emit viewRangeChanged();
    emit yScaleChanged();
    update();
}

void SpectrumView::clampView()
{
    const double span = m_maxX - m_minX;
    if (m_minX < dataMinX()) {
        m_minX = dataMinX();
        m_maxX = m_minX + span;
    }
    if (m_maxX > dataMaxX()) {
        m_maxX = dataMaxX();
        m_minX = m_maxX - span;
    }
    m_minX = std::max(m_minX, dataMinX());
}

void SpectrumView::wheelEvent(QWheelEvent* event)
{
    const double notches = event->angleDelta().y() / 120.0;
    if (notches != 0.0)
        zoomAround(nmAtX(event->position().x()), std::pow(1.0 / 1.15, notches));
    event->accept();
}

void SpectrumView::mousePressEvent(QMouseEvent* event)
{
    m_lastMouseX = event->position().x();
    inspectAtX(m_lastMouseX);
    event->accept();
}

void SpectrumView::hoverMoveEvent(QHoverEvent* event)
{
    inspectAtX(event->position().x());
    event->accept();
}

void SpectrumView::hoverLeaveEvent(QHoverEvent* event)
{
    clearCursor();
    event->accept();
}

void SpectrumView::inspectAtX(double x)
{
    if (!m_live.isValid()) {
        clearCursor();
        return;
    }

    const double targetNm = nmAtX(x);
    const auto it = std::lower_bound(m_live.wavelengthsNm.cbegin(),
                                     m_live.wavelengthsNm.cend(), targetNm);
    int index = int(it - m_live.wavelengthsNm.cbegin());
    if (index >= m_live.wavelengthsNm.size())
        index = m_live.wavelengthsNm.size() - 1;
    if (index > 0
        && std::abs(m_live.wavelengthsNm[index - 1] - targetNm)
           < std::abs(m_live.wavelengthsNm[index] - targetNm)) {
        --index;
    }

    m_cursorVisible = true;
    m_cursorWavelengthNm = m_live.wavelengthsNm[index];
    m_cursorValue = m_live.counts[index];
    m_cursorCandidate.clear();

    const bool localPeak = index > 0 && index + 1 < m_live.counts.size()
                           && m_live.counts[index] >= m_live.counts[index - 1]
                           && m_live.counts[index] >= m_live.counts[index + 1];
    if (localPeak) {
        const SpectroAnalysis::Peak peak{index, m_cursorWavelengthNm,
                                         m_cursorValue, 1.0};
        const auto candidates = PeakMatcher::match(
            {peak}, PeakMatcher::toleranceForGrid(m_live.wavelengthsNm));
        if (!candidates.isEmpty())
            m_cursorCandidate = candidates.first().species;
    }
    emit cursorChanged();
}

void SpectrumView::clearCursor()
{
    if (!m_cursorVisible)
        return;
    m_cursorVisible = false;
    emit cursorChanged();
}

void SpectrumView::mouseMoveEvent(QMouseEvent* event)
{
    const double dx = event->position().x() - m_lastMouseX;
    m_lastMouseX = event->position().x();
    if (width() > 0)
        panByNm(-dx * (m_maxX - m_minX) / width());
    event->accept();
}

void SpectrumView::mouseDoubleClickEvent(QMouseEvent* event)
{
    resetView();
    event->accept();
}

void SpectrumView::touchEvent(QTouchEvent* event)
{
    const auto& points = event->points();
    if (points.size() == 2) {
        const double distance = std::hypot(points[0].position().x() - points[1].position().x(),
                                           points[0].position().y() - points[1].position().y());
        const double centerX = (points[0].position().x() + points[1].position().x()) / 2.0;
        if (event->type() == QEvent::TouchUpdate && m_lastPinchDistance > 1.0)
            zoomAround(nmAtX(centerX), m_lastPinchDistance / distance);
        m_lastPinchDistance = distance;
    } else if (points.size() == 1 && event->type() == QEvent::TouchUpdate) {
        const double dx = points[0].position().x() - points[0].lastPosition().x();
        if (width() > 0)
            panByNm(-dx * (m_maxX - m_minX) / width());
        inspectAtX(points[0].position().x());
    } else if (points.size() == 1) {
        inspectAtX(points[0].position().x());
    } else {
        m_lastPinchDistance = 0.0;
    }
    event->accept();
}

bool SpectrumView::event(QEvent* event)
{
    if (event->type() == QEvent::NativeGesture) {
        auto* gesture = static_cast<QNativeGestureEvent*>(event);
        if (gesture->gestureType() == Qt::ZoomNativeGesture) {
            zoomAround(nmAtX(gesture->position().x()), 1.0 / (1.0 + gesture->value()));
            return true;
        }
    }
    return QQuickItem::event(event);
}

QSGNode* SpectrumView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    delete oldNode;
    auto* root = new QSGNode;

    const double w = width();
    const double h = height();
    if (w <= 0 || h <= 0 || m_maxX <= m_minX || m_maxY <= m_minY)
        return root;

    const auto toX = [&](double nm) {
        return float((nm - m_minX) / (m_maxX - m_minX) * w);
    };
    const auto toY = [&](double v) {
        return float(h - (v - m_minY) / (m_maxY - m_minY) * h);
    };

    const auto makeNode = [](QSGGeometry* geometry, const QColor& color) {
        auto* node = new QSGGeometryNode;
        auto* material = new QSGFlatColorMaterial;
        material->setColor(color);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
        return node;
    };

    // Grid: adaptive vertical wavelength lines + 5 horizontal divisions.
    {
        const QVariantList ticks = tickWavelengths();
        const int hLines = 5;
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                                         2 * (ticks.size() + hLines - 1));
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        auto* v = geometry->vertexDataAsPoint2D();
        int i = 0;
        for (const QVariant& tick : ticks) {
            const float x = toX(tick.toDouble());
            v[i++].set(x, 0.0f);
            v[i++].set(x, float(h));
        }
        for (int line = 1; line < hLines; ++line) {
            const float y = float(h * line / hLines);
            v[i++].set(0.0f, y);
            v[i++].set(float(w), y);
        }
        root->appendChildNode(makeNode(geometry, m_gridColor));
    }

    // Integration region: translucent band under the traces.
    if (!qIsNaN(m_integrationFromNm) && !qIsNaN(m_integrationToNm)
        && m_integrationToNm > m_minX && m_integrationFromNm < m_maxX) {
        const float x0 = toX(std::max(m_integrationFromNm, m_minX));
        const float x1 = toX(std::min(m_integrationToNm, m_maxX));
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
        auto* v = geometry->vertexDataAsPoint2D();
        v[0].set(x0, 0.0f);
        v[1].set(x0, float(h));
        v[2].set(x1, 0.0f);
        v[3].set(x1, float(h));
        root->appendChildNode(makeNode(geometry, m_regionColor));
    }

    // Frame: baseline + top edge in the stronger axis color.
    {
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        auto* v = geometry->vertexDataAsPoint2D();
        v[0].set(0.0f, float(h) - 1.0f);
        v[1].set(float(w), float(h) - 1.0f);
        v[2].set(0.0f, 1.0f);
        v[3].set(float(w), 1.0f);
        root->appendChildNode(makeNode(geometry, m_axisColor));
    }

    // Polylines, visible range only (plus one sample beyond each edge so the
    // trace runs to the clip border).
    const auto addTrace = [&](const Spectrum& s, const QColor& color) {
        if (!s.isValid() || s.counts.size() != s.wavelengthsNm.size())
            return;
        const auto& wl = s.wavelengthsNm;
        int first = int(std::lower_bound(wl.cbegin(), wl.cend(), m_minX) - wl.cbegin());
        int last = int(std::upper_bound(wl.cbegin(), wl.cend(), m_maxX) - wl.cbegin());
        first = std::max(0, first - 1);
        last = std::min(int(wl.size()), last + 1);
        const int count = last - first;
        if (count < 2)
            return;

        // Metal (the macOS RHI backend) supports only 1px lines; a thicker
        // trace needs triangle-strip ribbons — candidate for the polish pass.
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), count);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        auto* v = geometry->vertexDataAsPoint2D();
        for (int i = 0; i < count; ++i)
            v[i].set(toX(wl[first + i]), toY(s.counts[first + i]));
        root->appendChildNode(makeNode(geometry, color));
    };

    if (m_showDark)
        addTrace(m_dark, m_darkTraceColor);
    if (m_showReference)
        addTrace(m_reference, m_referenceColor);
    const QColor overlayColors[3] = {m_overlayColor1, m_overlayColor2, m_overlayColor3};
    for (int i = 0; i < m_overlays.size(); ++i)
        addTrace(m_overlays[i], overlayColors[i % 3]);
    addTrace(m_live, m_traceColor);

    return root;
}
