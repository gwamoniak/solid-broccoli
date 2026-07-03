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

        // ── Plot ──
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
                }
            }
        }
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
