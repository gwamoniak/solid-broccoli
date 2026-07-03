pragma Singleton
import QtQuick

QtObject {
    // ── Colors (Nikon-inspired dark instrument palette) ──
    // Yellow is the single accent: active tab, primary action, selected value,
    // the live trace. Red means recording, radiation above the alert
    // threshold, or destruction — nothing else.
    property color accent:            "#FFE100"
    property color accentPressed:     "#C7B000"
    property color destructive:       "#FF453A"
    property color destructivePressed: "#D93A30"

    property color groupedBackground: "#0E0E10"
    property color surface:           "#1A1A1D"
    property color surfaceElevated:   "#242428"
    property color label:             "#F2F2F2"
    property color secondaryLabel:    "#98989F"
    property color tertiaryLabel:     "#5A5A60"
    property color separator:         "#2C2C30"
    property color fill:              "#242428"
    property color textOverAccent:    "#000000"

    // ── Instrument colors ──
    property color live:              "#30D158"
    property color traceLive:         "#FFE100"
    property color traceReference:    "#64D2FF"
    property color traceDark:         "#8E8E93"
    property color traceOverlay1:     "#FF9F0A"
    property color traceOverlay2:     "#BF5AF2"
    property color traceOverlay3:     "#5AC8FA"
    property color plotGrid:          "#232327"
    property color plotAxis:          "#3A3A3F"

    // ── Type scale (point sizes) ──
    property int largeTitle: 30
    property int title:      22
    property int headline:   17
    property int body:       17
    property int callout:    16
    property int subhead:    15
    property int footnote:   13
    property int caption:    11

    property string fontName: ""
    // Monospaced font for live numeric readouts (wavelengths, counts,
    // timers) so digits don't jitter horizontally as values change.
    property string readoutFontName: "Menlo"
    // Letter-spacing for uppercase micro-labels like "PEAK λ".
    property real microLabelSpacing: 1.2

    // ── Metrics ──
    property int navBarHeight:  52
    property int tabBarHeight:  56
    property int rowHeight:     44
    property int screenMargin:  16
    property int gridGap:       12
    property int radiusCard:    12
    property int radiusControl: 10
    property int radiusSheet:   14
    property int radiusThumb:   10
    property int hairline:      1
    property int shutterSize:   66
    property int accentRing:    4

    property int maxContentWidth: 800
}
