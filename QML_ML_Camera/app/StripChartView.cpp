#include "StripChartView.h"

#include <algorithm>

#include <QSGFlatColorMaterial>
#include <QSGGeometryNode>

#include "GeigerService.h"

namespace {

QSGGeometryNode* makeLineNode(const QColor& color, int vertexCount,
                              QSGGeometry::DrawingMode mode)
{
    auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                                     vertexCount);
    geometry->setDrawingMode(mode);

    auto* material = new QSGFlatColorMaterial;
    material->setColor(color);

    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

} // namespace

StripChartView::StripChartView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void StripChartView::setSource(GeigerService* source)
{
    if (source == m_source)
        return;
    if (m_source)
        disconnect(m_source, nullptr, this, nullptr);
    m_source = source;
    if (m_source) {
        connect(m_source, &GeigerService::readingUpdated,
                this, &StripChartView::onReadingUpdated);
        connect(m_source, &GeigerService::alertThresholdChanged,
                this, &StripChartView::onReadingUpdated);
        onReadingUpdated();
    }
    emit sourceChanged();
}

void StripChartView::setWindowSeconds(int seconds)
{
    seconds = std::clamp(seconds, 10, 600);
    if (seconds == m_windowSeconds)
        return;
    m_windowSeconds = seconds;
    emit windowSecondsChanged();
    update();
}

void StripChartView::setCompact(bool compact)
{
    if (compact == m_compact)
        return;
    m_compact = compact;
    emit compactChanged();
    update();
}

void StripChartView::onReadingUpdated()
{
    if (!m_source)
        return;
    m_series = m_source->doseSeries();
    m_threshold = m_source->alertThreshold();

    double dataMax = 0.0;
    for (const QPointF& p : m_series)
        dataMax = std::max(dataMax, p.y());
    // Headroom above the data; in full mode the threshold line must always
    // be on screen so "how far from the alert" reads at a glance.
    double target = dataMax * 1.15;
    if (!m_compact)
        target = std::max(target, m_threshold * 1.25);
    target = std::max(target, 0.1);
    if (!qFuzzyCompare(target, m_yMax)) {
        m_yMax = target;
        emit yMaxChanged();
    }
    update();
}

QSGNode* StripChartView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    delete oldNode;
    if (width() <= 0 || height() <= 0)
        return nullptr;

    auto* root = new QSGNode;
    const double w = width();
    const double h = height();
    const double windowMs = m_windowSeconds * 1000.0;
    const double newestMs = m_series.isEmpty() ? 0.0 : m_series.last().x();

    const auto xAt = [&](double tMs) {
        return w - (newestMs - tMs) / windowMs * w;
    };
    const auto yAt = [&](double value) {
        return h - std::clamp(value / m_yMax, 0.0, 1.0) * h;
    };

    if (!m_compact) {
        // Hairline grid: vertical every 10 s, horizontal quarters.
        const int verticals = m_windowSeconds / 10;
        const int lineCount = verticals + 3;
        auto* grid = makeLineNode(m_gridColor, lineCount * 2, QSGGeometry::DrawLines);
        auto* v = grid->geometry()->vertexDataAsPoint2D();
        int i = 0;
        for (int g = 1; g <= verticals; ++g) {
            const float x = float(w * g / double(verticals + 0));
            v[i++].set(x, 0.0f);
            v[i++].set(x, float(h));
        }
        for (int g = 1; g <= 3; ++g) {
            const float y = float(h * g / 4.0);
            v[i++].set(0.0f, y);
            v[i++].set(float(w), y);
        }
        root->appendChildNode(grid);

        // Alert threshold line.
        auto* threshold = makeLineNode(m_thresholdColor, 2, QSGGeometry::DrawLines);
        auto* t = threshold->geometry()->vertexDataAsPoint2D();
        t[0].set(0.0f, float(yAt(m_threshold)));
        t[1].set(float(w), float(yAt(m_threshold)));
        root->appendChildNode(threshold);
    }

    if (m_series.size() >= 2) {
        auto* trace = makeLineNode(m_traceColor, m_series.size(),
                                   QSGGeometry::DrawLineStrip);
        auto* p = trace->geometry()->vertexDataAsPoint2D();
        for (int i = 0; i < m_series.size(); ++i)
            p[i].set(float(xAt(m_series[i].x())), float(yAt(m_series[i].y())));
        root->appendChildNode(trace);
    }

    return root;
}
