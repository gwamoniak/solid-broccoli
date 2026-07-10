import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import solid.broccoli 1.0
import "."

// The spectral viewfinder: readout strip, live plot, instrument controls.
// Thin view — every value shown and every action taken goes through
// SpectrometerService; the plot geometry lives in the C++ SpectrumView.
NavPage {
    id: livePage
    pageTitle: qsTr("Live")
    showLargeTitle: false

    readonly property bool calibrated: SpectrometerService.hasDark && SpectrometerService.hasReference

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Readout strip ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 64
            color: Theme.surface

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.screenMargin
                anchors.rightMargin: Theme.screenMargin
                spacing: 24

                component Readout: Column {
                    property string label: ""
                    property string value: ""
                    property color valueColor: Theme.label
                    spacing: 2
                    Label {
                        text: parent.label
                        font.pointSize: Theme.caption
                        font.letterSpacing: Theme.microLabelSpacing
                        color: Theme.secondaryLabel
                    }
                    Label {
                        text: parent.value
                        font.family: Theme.readoutFontName
                        font.pointSize: Theme.callout
                        color: parent.valueColor
                    }
                }

                Readout {
                    label: qsTr("PEAK λ")
                    value: SpectrometerService.peakWavelengthNm > 0
                           ? SpectrometerService.peakWavelengthNm.toFixed(1) + " nm" : "—"
                    valueColor: Theme.accent
                }
                Readout {
                    label: qsTr("PEAK")
                    value: SpectrometerService.peakValue > 0
                           ? SpectrometerService.peakValue.toFixed(0) : "—"
                    valueColor: Theme.accent
                }
                Readout {
                    label: qsTr("INTEGRATION")
                    value: SpectrometerService.integrationTimeMs + " ms"
                }
                Readout {
                    label: qsTr("FPS")
                    value: SpectrometerService.framesPerSecond > 0
                           ? SpectrometerService.framesPerSecond.toFixed(1) : "—"
                }

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: Theme.live
                    visible: SpectrometerService.acquiring
                }

                Item { Layout.fillWidth: true }

                Label {
                    id: errorLabel
                    text: ""
                    visible: text.length > 0
                    font.pointSize: Theme.footnote
                    color: Theme.destructive
                    elide: Text.ElideRight
                    Layout.maximumWidth: parent.width * 0.4
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.hairline
                color: Theme.separator
            }
        }

        // ── RADIATION card (visible while a Geiger device is connected) ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 72
            color: Theme.surface
            visible: GeigerService.connected

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (owningStack)
                        owningStack.push("qrc:/GeigerPage.qml", { owningStack: owningStack })
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.screenMargin
                anchors.rightMargin: Theme.screenMargin
                spacing: 24

                Column {
                    spacing: 2
                    Label {
                        text: qsTr("RADIATION")
                        font.pointSize: Theme.caption
                        font.letterSpacing: Theme.microLabelSpacing
                        color: Theme.secondaryLabel
                    }
                    Label {
                        text: GeigerService.doseMicroSvPerHour.toFixed(2) + " µSv/h"
                        font.family: Theme.readoutFontName
                        font.pointSize: Theme.title
                        color: GeigerService.aboveThreshold ? Theme.destructive : Theme.accent
                    }
                }
                Column {
                    spacing: 2
                    Label {
                        text: qsTr("CPM")
                        font.pointSize: Theme.caption
                        font.letterSpacing: Theme.microLabelSpacing
                        color: Theme.secondaryLabel
                    }
                    Label {
                        text: GeigerService.countsPerMinute.toFixed(0)
                        font.family: Theme.readoutFontName
                        font.pointSize: Theme.callout
                        color: Theme.label
                    }
                }

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: Theme.live
                    visible: GeigerService.acquiring
                }

                Item { Layout.fillWidth: true }

                StripChartView {
                    source: GeigerService
                    compact: true
                    windowSeconds: 60
                    traceColor: GeigerService.aboveThreshold ? Theme.destructive : Theme.accent
                    Layout.preferredWidth: 140
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: ">"
                    font.pointSize: Theme.body
                    color: Theme.secondaryLabel
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.hairline
                color: Theme.separator
            }
        }

        // ── Plot + docked analysis panel (tablet layout at ≥ 900 px) ──
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            SpectrumView {
                id: spectrumView
                anchors.fill: parent
                anchors.leftMargin: Theme.screenMargin
                anchors.rightMargin: Theme.screenMargin
                anchors.topMargin: 8
                anchors.bottomMargin: 24
                source: SpectrometerService
                gridColor: Theme.plotGrid
                axisColor: Theme.plotAxis
                traceColor: Theme.traceLive
                referenceColor: Theme.traceReference
                darkTraceColor: Theme.traceDark
                overlayColor1: Theme.traceOverlay1
                overlayColor2: Theme.traceOverlay2
                overlayColor3: Theme.traceOverlay3
                regionColor: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.15)
                integrationFromNm: SpectrometerService.integrationFromNm
                integrationToNm: SpectrometerService.integrationToNm
            }

            // Peak markers: triangle + λ, placed by the item's mapping
            // functions, updating in place as the model rows change.
            Repeater {
                model: SpectrometerService.peakModel
                delegate: Column {
                    id: peakMarker
                    required property double wavelengthNm
                    required property double value
                    spacing: 0
                    visible: peakMarker.wavelengthNm >= spectrumView.minWavelength
                             && peakMarker.wavelengthNm <= spectrumView.maxWavelength
                    x: {
                        // Touch the view's ranges so the binding re-evaluates on pan/zoom.
                        spectrumView.minWavelength; spectrumView.maxWavelength; spectrumView.width
                        return spectrumView.x + spectrumView.wavelengthToX(peakMarker.wavelengthNm) - width / 2
                    }
                    y: {
                        spectrumView.yMax
                        return Math.max(spectrumView.y,
                                        spectrumView.y + spectrumView.valueToY(peakMarker.value) - height - 6)
                    }

                    Label {
                        text: peakMarker.wavelengthNm.toFixed(1)
                        font.family: Theme.readoutFontName
                        font.pointSize: Theme.caption
                        color: Theme.accent
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Label {
                        text: "▾"
                        font.pointSize: Theme.caption
                        color: Theme.accent
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // Value-scale readout (top-left, inside the plot).
            Label {
                anchors.top: spectrumView.top
                anchors.left: spectrumView.left
                anchors.margins: 6
                text: SpectrometerService.mode === 0
                      ? Math.round(spectrumView.yMax)
                      : spectrumView.yMax.toFixed(2)
                font.family: Theme.readoutFontName
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
            }

            // Wavelength tick labels, placed by the item's mapping function.
            Repeater {
                model: spectrumView.tickWavelengths
                Label {
                    required property var modelData
                    parent: spectrumView.parent
                    x: spectrumView.x + spectrumView.wavelengthToX(modelData) - implicitWidth / 2
                    y: spectrumView.y + spectrumView.height + 4
                    text: Math.round(modelData)
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.caption
                    color: Theme.secondaryLabel
                }
            }

            Label {
                anchors.right: spectrumView.right
                anchors.bottom: spectrumView.bottom
                anchors.margins: 6
                text: spectrumView.xAxisTitle
                font.pointSize: Theme.caption
                color: Theme.tertiaryLabel
            }
        }

            AnalysisPanel {
                visible: livePage.width >= 900
                Layout.fillHeight: true
                Layout.preferredWidth: 320
                plotView: spectrumView
                onSaveRequested: saveCaptureDialog.open()
            }
        }

        // ── Control bar ──
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 92
            color: Theme.surface

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: Theme.hairline
                color: Theme.separator
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.screenMargin
                anchors.rightMargin: Theme.screenMargin
                spacing: 12

                // Mode segments
                Column {
                    spacing: 4
                    Row {
                        spacing: 4
                        Repeater {
                            model: [qsTr("RAW"), qsTr("TRANS"), qsTr("ABS")]
                            Button {
                                id: modeButton
                                required property int index
                                required property string modelData
                                width: 64
                                height: 32
                                enabled: index === 0 || livePage.calibrated
                                onClicked: SpectrometerService.mode = index
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: SpectrometerService.mode === modeButton.index
                                           ? Theme.accent : Theme.fill
                                    opacity: modeButton.enabled ? 1.0 : 0.4
                                }
                                contentItem: Text {
                                    text: modeButton.modelData
                                    font.pointSize: Theme.footnote
                                    font.letterSpacing: Theme.microLabelSpacing
                                    color: SpectrometerService.mode === modeButton.index
                                           ? Theme.textOverAccent
                                           : (modeButton.enabled ? Theme.label : Theme.tertiaryLabel)
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                    Label {
                        visible: !livePage.calibrated
                        text: qsTr("Capture Dark and Ref to enable")
                        font.pointSize: Theme.caption
                        color: Theme.tertiaryLabel
                    }
                }

                Item { Layout.fillWidth: true }

                // Start / Stop
                Rectangle {
                    id: startButton
                    width: Theme.shutterSize
                    height: Theme.shutterSize
                    radius: Theme.shutterSize / 2
                    color: "transparent"
                    border.color: Theme.accent
                    border.width: Theme.accentRing

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 2 * (Theme.accentRing + 3)
                        height: width
                        radius: width / 2
                        color: SpectrometerService.acquiring
                               ? Theme.surfaceElevated
                               : (startMouse.pressed ? Theme.accentPressed : Theme.accent)

                        Text {
                            anchors.centerIn: parent
                            text: SpectrometerService.acquiring ? "■" : "▶"
                            font.pointSize: 18
                            color: SpectrometerService.acquiring ? Theme.accent : Theme.textOverAccent
                        }
                    }

                    MouseArea {
                        id: startMouse
                        anchors.fill: parent
                        onClicked: SpectrometerService.acquiring
                                   ? SpectrometerService.stopAcquisition()
                                   : SpectrometerService.startAcquisition()
                    }
                }

                Item { Layout.fillWidth: true }

                // Dark / Ref / Hold
                Row {
                    spacing: 8

                    component CaptureButton: Button {
                        id: captureBtn
                        property bool captured: false
                        property bool active: false
                        width: 60
                        height: 36
                        background: Rectangle {
                            radius: Theme.radiusControl
                            color: captureBtn.active
                                   ? Theme.accent
                                   : (captureBtn.down ? Theme.surfaceElevated : Theme.fill)
                            opacity: captureBtn.enabled ? 1.0 : 0.4

                            Rectangle {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.margins: 4
                                width: 6
                                height: 6
                                radius: 3
                                color: Theme.accent
                                visible: captureBtn.captured
                            }
                        }
                        contentItem: Text {
                            text: captureBtn.text
                            font.pointSize: Theme.footnote
                            color: captureBtn.active
                                   ? Theme.textOverAccent
                                   : (captureBtn.enabled ? Theme.label : Theme.tertiaryLabel)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CaptureButton {
                        text: qsTr("Dark")
                        enabled: SpectrometerService.acquiring
                        captured: SpectrometerService.hasDark
                        onClicked: SpectrometerService.captureDark()
                    }
                    CaptureButton {
                        text: qsTr("Ref")
                        enabled: SpectrometerService.acquiring
                        captured: SpectrometerService.hasReference
                        onClicked: SpectrometerService.captureReference()
                    }
                    CaptureButton {
                        text: qsTr("Hold")
                        enabled: SpectrometerService.acquiring || SpectrometerService.hold
                        active: SpectrometerService.hold
                        onClicked: SpectrometerService.hold = !SpectrometerService.hold
                    }

                    CaptureButton {
                        text: "⋯"
                        visible: livePage.width < 900
                        width: 40
                        onClicked: analysisDrawer.open()
                    }
                }
            }
        }
    }

    Drawer {
        id: analysisDrawer
        edge: Qt.RightEdge
        width: 320
        height: livePage.height

        AnalysisPanel {
            anchors.fill: parent
            plotView: spectrumView
            onSaveRequested: {
                analysisDrawer.close()
                saveCaptureDialog.open()
            }
        }
    }

    SaveCaptureDialog {
        id: saveCaptureDialog
        onAccepted: SpectrometerService.saveCapture(nameText, tagsText)
    }

    Connections {
        target: SpectrometerService
        function onErrorOccurred(message) {
            errorLabel.text = message
            errorTimer.restart()
        }
    }

    Timer {
        id: errorTimer
        interval: 4000
        onTriggered: errorLabel.text = ""
    }
}
