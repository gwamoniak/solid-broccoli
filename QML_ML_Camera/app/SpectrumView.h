#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QColor>
#include <QQuickItem>
#include <QVariantList>

#include "Spectrum.h"

class SpectrometerService;

// GPU spectral plot: grid, axes, live/dark/reference polylines, zoom/pan.
// All geometry is built in C++ (scene-graph nodes); QML supplies colors from
// Theme tokens and places the axis tick labels via the mapping invokables.
// The x-axis is generic (title/tick step properties) so the same item can
// later plot echo time for A-scan ultrasound.
class SpectrumView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(SpectrometerService* source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(double minWavelength READ minWavelength NOTIFY viewRangeChanged)
    Q_PROPERTY(double maxWavelength READ maxWavelength NOTIFY viewRangeChanged)
    Q_PROPERTY(double yMax READ yMax NOTIFY yScaleChanged)
    Q_PROPERTY(bool autoScaleY READ autoScaleY WRITE setAutoScaleY NOTIFY autoScaleYChanged)
    Q_PROPERTY(QString xAxisTitle READ xAxisTitle WRITE setXAxisTitle NOTIFY xAxisTitleChanged)
    Q_PROPERTY(double xTickStep READ xTickStep WRITE setXTickStep NOTIFY xTickStepChanged)
    Q_PROPERTY(bool showReference READ showReference WRITE setShowReference NOTIFY traceVisibilityChanged)
    Q_PROPERTY(bool showDark READ showDark WRITE setShowDark NOTIFY traceVisibilityChanged)
    Q_PROPERTY(QVariantList tickWavelengths READ tickWavelengths NOTIFY viewRangeChanged)
    Q_PROPERTY(QColor gridColor MEMBER m_gridColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor axisColor MEMBER m_axisColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor traceColor MEMBER m_traceColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor referenceColor MEMBER m_referenceColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor darkTraceColor MEMBER m_darkTraceColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor regionColor MEMBER m_regionColor NOTIFY colorsChanged)
    Q_PROPERTY(double integrationFromNm READ integrationFromNm WRITE setIntegrationFromNm NOTIFY integrationRegionChanged)
    Q_PROPERTY(double integrationToNm READ integrationToNm WRITE setIntegrationToNm NOTIFY integrationRegionChanged)

public:
    explicit SpectrumView(QQuickItem* parent = nullptr);

    SpectrometerService* source() const { return m_source; }
    void setSource(SpectrometerService* source);

    double minWavelength() const { return m_minX; }
    double maxWavelength() const { return m_maxX; }
    double yMax() const { return m_maxY; }
    bool autoScaleY() const { return m_autoScaleY; }
    void setAutoScaleY(bool autoScale);
    QString xAxisTitle() const { return m_xAxisTitle; }
    void setXAxisTitle(const QString& title);
    double xTickStep() const { return m_xTickStep; }
    void setXTickStep(double step);
    bool showReference() const { return m_showReference; }
    void setShowReference(bool show);
    bool showDark() const { return m_showDark; }
    void setShowDark(bool show);
    QVariantList tickWavelengths() const;

    double integrationFromNm() const { return m_integrationFromNm; }
    void setIntegrationFromNm(double nm);
    double integrationToNm() const { return m_integrationToNm; }
    void setIntegrationToNm(double nm);

    Q_INVOKABLE double wavelengthToX(double nm) const;
    Q_INVOKABLE double valueToY(double value) const;
    Q_INVOKABLE void centerOn(double nm);
    Q_INVOKABLE void resetView();

signals:
    void sourceChanged();
    void viewRangeChanged();   // x-range/ticks — zoom, pan, reset only
    void yScaleChanged();      // value scale — may fire per frame
    void autoScaleYChanged();
    void xAxisTitleChanged();
    void xTickStepChanged();
    void traceVisibilityChanged();
    void colorsChanged();
    void integrationRegionChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void touchEvent(QTouchEvent* event) override;
    bool event(QEvent* event) override;

private:
    void onSpectrumUpdated();
    void updateYScale();
    double nmAtX(double x) const;
    void zoomAround(double centerNm, double factor);
    void panByNm(double deltaNm);
    void clampView();
    double dataMinX() const;
    double dataMaxX() const;

    SpectrometerService* m_source = nullptr;

    // Render copies, refreshed on the GUI thread per frame; updatePaintNode
    // (render thread, GUI blocked) only reads them.
    Spectrum m_live;
    Spectrum m_reference;
    Spectrum m_dark;

    double m_minX = 340.0;
    double m_maxX = 1020.0;
    double m_minY = 0.0;
    double m_maxY = 1000.0;
    bool m_autoScaleY = true;
    bool m_userZoomed = false;

    QString m_xAxisTitle = QStringLiteral("λ / nm");
    double m_xTickStep = 50.0;
    bool m_showReference = false;
    bool m_showDark = false;

    QColor m_gridColor{0x23, 0x23, 0x27};
    QColor m_axisColor{0x3A, 0x3A, 0x3F};
    QColor m_traceColor{0xFF, 0xE1, 0x00};
    QColor m_referenceColor{0x64, 0xD2, 0xFF};
    QColor m_darkTraceColor{0x8E, 0x8E, 0x93};
    QColor m_regionColor{255, 225, 0, 38};
    double m_integrationFromNm = qQNaN();
    double m_integrationToNm = qQNaN();

    double m_lastMouseX = 0.0;
    double m_lastPinchDistance = 0.0;
};

#endif // SPECTRUMVIEW_H
