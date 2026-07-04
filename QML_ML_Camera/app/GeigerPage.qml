import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import solid.broccoli 1.0
import "."

// Full radiation view: big dose readout, scrolling strip chart with the red
// alert-threshold line, Start/Stop, Save-to-session, and CSV export. Thin
// view — every number and action goes through GeigerService.
NavPage {
    id: geigerPage
    pageTitle: qsTr("Radiation")
    showLargeTitle: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        spacing: Theme.gridGap

        // ── Readouts ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 32

            Column {
                spacing: 2
                Label {
                    text: qsTr("DOSE RATE")
                    font.pointSize: Theme.caption
                    font.letterSpacing: Theme.microLabelSpacing
                    color: Theme.secondaryLabel
                }
                Label {
                    text: GeigerService.doseMicroSvPerHour.toFixed(2) + " µSv/h"
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.largeTitle
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
                    font.pointSize: Theme.title
                    color: Theme.label
                }
            }
            Column {
                spacing: 2
                Label {
                    text: qsTr("CPS")
                    font.pointSize: Theme.caption
                    font.letterSpacing: Theme.microLabelSpacing
                    color: Theme.secondaryLabel
                }
                Label {
                    text: GeigerService.countsPerSecond.toFixed(0)
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.title
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

            Label {
                id: geigerError
                text: ""
                visible: text.length > 0
                font.pointSize: Theme.footnote
                color: Theme.destructive
                elide: Text.ElideRight
                Layout.maximumWidth: parent.width * 0.35
            }
        }

        // ── Strip chart ──
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Theme.radiusCard
            color: Theme.groupedBackground
            border.color: Theme.separator
            border.width: Theme.hairline

            StripChartView {
                id: stripChart
                anchors.fill: parent
                anchors.margins: 8
                source: GeigerService
                windowSeconds: 60
                gridColor: Theme.plotGrid
                traceColor: Theme.traceLive
                thresholdColor: Theme.destructive
            }

            Label {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 12
                text: stripChart.yMax.toFixed(2) + " µSv/h"
                font.family: Theme.readoutFontName
                font.pointSize: Theme.footnote
                color: Theme.tertiaryLabel
            }
            Label {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 12
                text: qsTr("last %1 s · alert %2 µSv/h")
                          .arg(stripChart.windowSeconds)
                          .arg(GeigerService.alertThreshold.toFixed(2))
                font.pointSize: Theme.footnote
                color: Theme.tertiaryLabel
            }
        }

        // ── Controls ──
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.gridGap

            Button {
                id: geigerStartButton
                Layout.preferredWidth: 120
                text: GeigerService.acquiring ? qsTr("Stop") : qsTr("Start")
                onClicked: GeigerService.acquiring ? GeigerService.stopAcquisition()
                                                   : GeigerService.startAcquisition()
                background: Rectangle {
                    radius: Theme.radiusControl
                    color: GeigerService.acquiring
                           ? Theme.fill
                           : (geigerStartButton.down ? Theme.accentPressed : Theme.accent)
                }
                contentItem: Text {
                    text: geigerStartButton.text
                    font.pointSize: Theme.body
                    color: GeigerService.acquiring ? Theme.label : Theme.textOverAccent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                id: saveMeasurementButton
                text: qsTr("Save")
                enabled: GeigerService.acquiring
                onClicked: saveMeasurementDialog.open()
                background: Rectangle {
                    radius: Theme.radiusControl
                    color: saveMeasurementButton.down ? Theme.accentPressed : Theme.fill
                }
                contentItem: Text {
                    text: saveMeasurementButton.text
                    font.pointSize: Theme.body
                    color: saveMeasurementButton.enabled ? Theme.label : Theme.tertiaryLabel
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 16
                    rightPadding: 16
                }
            }

            Button {
                id: exportGeigerButton
                text: qsTr("Export CSV")
                onClicked: GeigerService.exportWindowCsv()
                background: Rectangle {
                    radius: Theme.radiusControl
                    color: exportGeigerButton.down ? Theme.accentPressed : Theme.fill
                }
                contentItem: Text {
                    text: exportGeigerButton.text
                    font.pointSize: Theme.body
                    color: Theme.label
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 16
                    rightPadding: 16
                }
            }
        }
    }

    InputDialog {
        id: saveMeasurementDialog
        label: qsTr("Save measurement")
        hint: qsTr("Dose reading")
        onAccepted: GeigerService.saveMeasurement(editText.text, "")
    }

    Timer {
        id: geigerErrorTimer
        interval: 4000
        onTriggered: geigerError.text = ""
    }

    Connections {
        target: GeigerService
        function onErrorOccurred(message) {
            geigerError.text = message
            geigerErrorTimer.restart()
        }
    }
}
