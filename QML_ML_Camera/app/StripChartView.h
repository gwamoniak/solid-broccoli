#ifndef STRIPCHARTVIEW_H
#define STRIPCHARTVIEW_H

#include <QColor>
#include <QPointF>
#include <QQuickItem>
#include <QVector>

#include "GeigerService.h"

// Scrolling time-series plot for the radiation dose rate: hairline grid,
// yellow trace, red horizontal line at the alert threshold. Scene-graph
// sibling of SpectrumView, deliberately simpler — fixed time window, newest
// sample pinned to the right edge, y auto-scaled to the data and threshold.
// The compact form (sparkline on the Live-tab card) hides grid+threshold.
class StripChartView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(GeigerService* source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int windowSeconds READ windowSeconds WRITE setWindowSeconds NOTIFY windowSecondsChanged)
    Q_PROPERTY(bool compact READ compact WRITE setCompact NOTIFY compactChanged)
    Q_PROPERTY(double yMax READ yMax NOTIFY yMaxChanged)
    Q_PROPERTY(QColor gridColor MEMBER m_gridColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor traceColor MEMBER m_traceColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor thresholdColor MEMBER m_thresholdColor NOTIFY colorsChanged)

public:
    explicit StripChartView(QQuickItem* parent = nullptr);

    GeigerService* source() const { return m_source; }
    void setSource(GeigerService* source);
    int windowSeconds() const { return m_windowSeconds; }
    void setWindowSeconds(int seconds);
    bool compact() const { return m_compact; }
    void setCompact(bool compact);
    double yMax() const { return m_yMax; }

signals:
    void sourceChanged();
    void windowSecondsChanged();
    void compactChanged();
    void yMaxChanged();
    void colorsChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    void onReadingUpdated();

    GeigerService* m_source = nullptr;
    int m_windowSeconds = 60;
    bool m_compact = false;

    // Render copies, refreshed on the GUI thread; updatePaintNode only reads.
    QVector<QPointF> m_series;   // (timestampMs, µSv/h)
    double m_threshold = 0.5;
    double m_yMax = 1.0;

    QColor m_gridColor{0x23, 0x23, 0x27};
    QColor m_traceColor{0xFF, 0xE1, 0x00};
    QColor m_thresholdColor{0xFF, 0x45, 0x3A};
};

#endif // STRIPCHARTVIEW_H
