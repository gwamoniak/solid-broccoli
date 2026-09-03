import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import solid.broccoli 1.0
import "."

// The spectroscopist's numbers: processing, peaks, integration, capture.
// Thin view over SpectrometerService; needs a reference to the plot item
// for centerOn / "Set from view".
Rectangle {
    id: panel
    color: Theme.surface

    property var plotView: null
    signal saveRequested()

    Rectangle {
        anchors.left: parent.left
        width: Theme.hairline
        height: parent.height
        color: Theme.separator
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        contentHeight: panelCol.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ColumnLayout {
            id: panelCol
            width: parent.width
            spacing: 16

            component SectionLabel: Label {
                font.pointSize: Theme.caption
                font.letterSpacing: Theme.microLabelSpacing
                color: Theme.secondaryLabel
            }

            // ── PROCESSING ──
            SectionLabel { text: qsTr("PROCESSING") }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Smoothing")
                    font.pointSize: Theme.subhead
                    color: Theme.label
                }
                Item { Layout.fillWidth: true }
                Button {
                    id: smoothMinus
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    text: "−"
                    enabled: SpectrometerService.smoothingWindow > 0
                    onClicked: SpectrometerService.smoothingWindow =
                                   SpectrometerService.smoothingWindow <= 5
                                   ? 0 : SpectrometerService.smoothingWindow - 2
                    background: Rectangle { radius: Theme.radiusControl; color: Theme.fill }
                    contentItem: Text {
                        text: smoothMinus.text
                        color: smoothMinus.enabled ? Theme.label : Theme.tertiaryLabel
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Label {
                    text: SpectrometerService.smoothingWindow > 0
                          ? SpectrometerService.smoothingWindow : qsTr("off")
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.subhead
                    color: SpectrometerService.smoothingWindow > 0 ? Theme.accent : Theme.tertiaryLabel
                    horizontalAlignment: Text.AlignHCenter
                    Layout.preferredWidth: 34
                }
                Button {
                    id: smoothPlus
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    text: "+"
                    enabled: SpectrometerService.smoothingWindow < 25
                    onClicked: SpectrometerService.smoothingWindow =
                                   SpectrometerService.smoothingWindow === 0
                                   ? 5 : SpectrometerService.smoothingWindow + 2
                    background: Rectangle { radius: Theme.radiusControl; color: Theme.fill }
                    contentItem: Text {
                        text: smoothPlus.text
                        color: smoothPlus.enabled ? Theme.label : Theme.tertiaryLabel
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: Theme.hairline; color: Theme.separator }

            // ── PEAKS ──
            SectionLabel { text: qsTr("PEAKS") }

            ListView {
                id: peakList
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 5 * 34)
                clip: true
                model: SpectrometerService.peakModel
                boundsBehavior: Flickable.StopAtBounds

                delegate: Item {
                    id: peakRow
                    required property double wavelengthNm
                    required property string wavelengthText
                    required property string valueText
                    required property string prominenceText
                    width: peakList.width
                    height: 34

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (panel.plotView) panel.plotView.centerOn(peakRow.wavelengthNm)
                    }

                    RowLayout {
                        anchors.fill: parent
                        spacing: 8
                        Label {
                            text: peakRow.wavelengthText
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.footnote
                            color: Theme.accent
                            Layout.preferredWidth: 76
                        }
                        Label {
                            text: peakRow.valueText
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.footnote
                            color: Theme.label
                            Layout.fillWidth: true
                        }
                        Label {
                            text: peakRow.prominenceText
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.caption
                            color: Theme.secondaryLabel
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: peakList.count === 0
                    text: qsTr("No peaks")
                    font.pointSize: Theme.footnote
                    color: Theme.tertiaryLabel
                }
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: Theme.hairline; color: Theme.separator }

            // ── INTEGRATION ──
            SectionLabel { text: qsTr("INTEGRATION") }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: SpectrometerService.hasIntegrationRegion
                          ? SpectrometerService.integrationFromNm.toFixed(0) + "–"
                            + SpectrometerService.integrationToNm.toFixed(0) + " nm"
                          : qsTr("No region")
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.footnote
                    color: SpectrometerService.hasIntegrationRegion
                           ? Theme.label : Theme.tertiaryLabel
                }
                Item { Layout.fillWidth: true }
                Button {
                    id: setRegionBtn
                    text: qsTr("Set from view")
                    Layout.preferredHeight: 30
                    onClicked: if (panel.plotView)
                                   SpectrometerService.setIntegrationRegion(
                                       panel.plotView.minWavelength,
                                       panel.plotView.maxWavelength)
                    background: Rectangle { radius: Theme.radiusControl; color: Theme.fill }
                    contentItem: Text {
                        text: setRegionBtn.text
                        font.pointSize: Theme.footnote
                        color: Theme.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 8
                        rightPadding: 8
                    }
                }
            }

            Label {
                text: SpectrometerService.hasIntegrationRegion
                          && !isNaN(SpectrometerService.integralValue)
                      ? "∫ " + SpectrometerService.integralValue.toPrecision(5)
                      : "∫ —"
                font.family: Theme.readoutFontName
                font.pointSize: Theme.title
                color: Theme.accent
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: Theme.hairline; color: Theme.separator }

            // ── CAPTURE ──
            SectionLabel { text: qsTr("CAPTURE") }

            Button {
                id: saveBtn
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                text: qsTr("Save capture")
                enabled: SpectrometerService.acquiring || SpectrometerService.hold
                onClicked: panel.saveRequested()
                background: Rectangle {
                    radius: Theme.radiusControl
                    color: saveBtn.down ? Theme.accentPressed : Theme.accent
                    opacity: saveBtn.enabled ? 1.0 : 0.4
                }
                contentItem: Text {
                    text: saveBtn.text
                    font.pointSize: Theme.subhead
                    color: Theme.textOverAccent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.preferredHeight: 8 }
        }
    }
}
