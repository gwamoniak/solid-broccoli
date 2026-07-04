import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import solid.broccoli 1.0
import "."

// Captures of one session: overlay onto the live plot, export, rename,
// delete. All operations go through sessionSpectrumModel / the service.
NavPage {
    id: detailPage
    property int sessionId: -1
    property string sessionName: ""
    property int sessionRow: -1
    property int actionRow: -1
    property int reportsRefresh: 0  // bumped when a report is saved

    pageTitle: sessionName
    showLargeTitle: false

    Component.onCompleted: sessionSpectrumModel.setSessionId(sessionId)

    ListView {
        id: captureList
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        spacing: 8
        model: sessionSpectrumModel
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        // Dose measurements and documentation videos of this session.
        footer: Column {
            width: captureList.width
            spacing: 6
            property var videos: sessionSpectrumModel.sessionVideos(detailPage.sessionId)
            property var measurements: sessionSpectrumModel.sessionMeasurements(detailPage.sessionId)

            Label {
                visible: parent.measurements.length > 0
                topPadding: 16
                text: qsTr("MEASUREMENTS")
                font.pointSize: Theme.caption
                font.letterSpacing: Theme.microLabelSpacing
                color: Theme.secondaryLabel
            }

            Repeater {
                model: parent.measurements
                Rectangle {
                    id: measurementRow
                    required property var modelData
                    width: captureList.width
                    height: 40
                    radius: Theme.radiusControl
                    color: Theme.surface

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 10

                        // Dosimeter badge
                        Rectangle {
                            width: 56
                            height: 22
                            radius: 5
                            color: Theme.fill
                            Label {
                                anchors.centerIn: parent
                                text: qsTr("GEIGER")
                                font.pointSize: Theme.caption
                                font.letterSpacing: Theme.microLabelSpacing
                                color: Theme.secondaryLabel
                            }
                        }
                        Label {
                            text: JSON.parse(measurementRow.modelData.summary).name || qsTr("Dose")
                            font.pointSize: Theme.footnote
                            color: Theme.label
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: measurementRow.modelData.value.toFixed(2) + " "
                                  + measurementRow.modelData.unit
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.caption
                            color: Theme.accent
                        }
                    }
                }
            }

            Label {
                visible: parent.videos.length > 0
                topPadding: 16
                text: qsTr("VIDEOS")
                font.pointSize: Theme.caption
                font.letterSpacing: Theme.microLabelSpacing
                color: Theme.secondaryLabel
            }

            Repeater {
                model: parent.videos
                Rectangle {
                    id: videoRow
                    required property var modelData
                    width: captureList.width
                    height: 40
                    radius: Theme.radiusControl
                    color: Theme.surface

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        Label {
                            text: videoRow.modelData.name
                            font.pointSize: Theme.footnote
                            color: Theme.label
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: Math.round(videoRow.modelData.durationMs / 1000) + " s"
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.caption
                            color: Theme.secondaryLabel
                        }
                    }
                }
            }

            // AI reports of this session (viewing/export works even without
            // the AI build; only generation needs it).
            property var reports: {
                detailPage.reportsRefresh
                return sessionSpectrumModel.sessionReports(detailPage.sessionId)
            }

            Label {
                visible: parent.reports.length > 0
                topPadding: 16
                text: qsTr("REPORTS")
                font.pointSize: Theme.caption
                font.letterSpacing: Theme.microLabelSpacing
                color: Theme.secondaryLabel
            }

            Repeater {
                model: parent.reports
                Rectangle {
                    id: reportRow
                    required property var modelData
                    width: captureList.width
                    height: 40
                    radius: Theme.radiusControl
                    color: Theme.surface

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            reportText.text = reportRow.modelData.content
                            reportViewer.reportId = reportRow.modelData.id
                            reportViewer.streaming = false
                            reportViewer.open()
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 34
                            Layout.preferredHeight: 22
                            radius: 5
                            color: Theme.fill
                            Label {
                                anchors.centerIn: parent
                                text: qsTr("AI")
                                font.pointSize: Theme.caption
                                font.letterSpacing: Theme.microLabelSpacing
                                color: Theme.accent
                            }
                        }
                        Label {
                            text: reportRow.modelData.modelName
                            font.pointSize: Theme.footnote
                            color: Theme.label
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: reportRow.modelData.createdUtc
                            font.family: Theme.readoutFontName
                            font.pointSize: Theme.caption
                            color: Theme.secondaryLabel
                        }
                    }
                }
            }

            // Generation entry point — only with the AI build present.
            Loader {
                width: captureList.width
                active: aiAvailable
                sourceComponent: Button {
                    id: generateButton
                    enabled: !reportService.busy
                    text: reportService.busy ? qsTr("Generating…")
                                             : qsTr("Generate AI report")
                    onClicked: {
                        reportText.text = ""
                        reportViewer.reportId = -1
                        reportViewer.streaming = true
                        reportViewer.open()
                        reportService.generateReport(detailPage.sessionId)
                    }
                    background: Rectangle {
                        radius: Theme.radiusControl
                        color: generateButton.enabled
                               ? (generateButton.down ? Theme.accentPressed : Theme.accent)
                               : Theme.fill
                    }
                    contentItem: Text {
                        text: generateButton.text
                        font.pointSize: Theme.subhead
                        color: generateButton.enabled ? Theme.textOverAccent
                                                      : Theme.tertiaryLabel
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Item { height: 12; width: 1 }
        }

        delegate: Rectangle {
            id: captureRow
            required property int index
            required property int id
            required property string name
            required property string kind
            required property string created
            required property string tags

            width: captureList.width
            height: 60
            radius: Theme.radiusCard
            color: Theme.surface

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                // Kind badge
                Rectangle {
                    width: 46
                    height: 22
                    radius: 5
                    color: Theme.fill
                    Label {
                        anchors.centerIn: parent
                        text: captureRow.kind === "dark" ? qsTr("DARK")
                              : captureRow.kind === "reference" ? qsTr("REF") : qsTr("SMP")
                        font.pointSize: Theme.caption
                        font.letterSpacing: Theme.microLabelSpacing
                        color: captureRow.kind === "sample" ? Theme.accent : Theme.secondaryLabel
                    }
                }

                Column {
                    spacing: 2
                    Layout.fillWidth: true
                    Label {
                        text: captureRow.name
                        font.pointSize: Theme.subhead
                        color: Theme.label
                        elide: Text.ElideRight
                    }
                    Label {
                        text: captureRow.created
                              + (captureRow.tags.length > 0 ? "  ·  " + captureRow.tags : "")
                        font.pointSize: Theme.caption
                        color: Theme.secondaryLabel
                        elide: Text.ElideRight
                    }
                }

                // Overlay toggle
                Button {
                    id: overlayBtn
                    width: 52
                    height: 30
                    text: qsTr("Ovl")
                    onClicked: SpectrometerService.toggleOverlay(captureRow.id)
                    background: Rectangle {
                        radius: Theme.radiusControl
                        color: SpectrometerService.overlayIds.indexOf(captureRow.id) >= 0
                               ? Theme.accent : Theme.fill
                    }
                    contentItem: Text {
                        text: overlayBtn.text
                        font.pointSize: Theme.footnote
                        color: SpectrometerService.overlayIds.indexOf(captureRow.id) >= 0
                               ? Theme.textOverAccent : Theme.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                ToolButton {
                    icon.source: "qrc:/images/svg/save.svg"
                    icon.color: Theme.label
                    icon.width: 18
                    icon.height: 18
                    background: null
                    onClicked: SpectrometerService.exportCapture(captureRow.id)
                }

                ToolButton {
                    icon.source: "qrc:/images/svg/rename.svg"
                    icon.color: Theme.label
                    icon.width: 18
                    icon.height: 18
                    background: null
                    onClicked: {
                        detailPage.actionRow = captureRow.index
                        renameDialog.editText.text = captureRow.name
                        renameDialog.open()
                    }
                }

                ToolButton {
                    icon.source: "qrc:/images/svg/delete.svg"
                    icon.color: Theme.destructive
                    icon.width: 18
                    icon.height: 18
                    background: null
                    onClicked: {
                        detailPage.actionRow = captureRow.index
                        deleteDialog.open()
                    }
                }
            }
        }
    }

    Label {
        anchors.centerIn: parent
        visible: captureList.count === 0
        text: qsTr("No captures in this session")
        font.pointSize: Theme.subhead
        color: Theme.tertiaryLabel
    }

    InputDialog {
        id: renameDialog
        label: qsTr("Rename capture")
        onAccepted: sessionSpectrumModel.rename(detailPage.actionRow,
                                                renameDialog.editText.text)
    }

    Dialog {
        id: deleteDialog
        modal: true
        anchors.centerIn: parent
        width: Math.min(360, parent ? parent.width - 64 : 360)
        standardButtons: Dialog.NoButton

        background: Rectangle {
            color: Theme.surfaceElevated
            radius: Theme.radiusSheet
            border.color: Theme.separator
            border.width: Theme.hairline
        }

        contentItem: ColumnLayout {
            spacing: 14

            Label {
                text: qsTr("Delete this capture?")
                font.pointSize: Theme.headline
                color: Theme.label
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                Button {
                    id: cancelDelete
                    text: qsTr("Cancel")
                    onClicked: deleteDialog.reject()
                    background: Rectangle { radius: Theme.radiusControl; color: Theme.fill }
                    contentItem: Text {
                        text: cancelDelete.text
                        font.pointSize: Theme.subhead
                        color: Theme.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12; rightPadding: 12
                    }
                }
                Button {
                    id: confirmDelete
                    text: qsTr("Delete")
                    onClicked: {
                        sessionSpectrumModel.removeRows(detailPage.actionRow, 1)
                        deleteDialog.close()
                    }
                    background: Rectangle {
                        radius: Theme.radiusControl
                        color: confirmDelete.down ? Theme.destructivePressed : Theme.destructive
                    }
                    contentItem: Text {
                        text: confirmDelete.text
                        font.pointSize: Theme.subhead
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12; rightPadding: 12
                    }
                }
            }
        }
    }

    // ── Report viewer: shows a stored report, or streams a new one ──
    Dialog {
        id: reportViewer
        property int reportId: -1
        property bool streaming: false
        modal: true
        width: Math.min(640, detailPage.width - 48)
        height: Math.min(560, detailPage.height - 96)
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton

        background: Rectangle {
            color: Theme.surfaceElevated
            radius: Theme.radiusSheet
            border.color: Theme.separator
            border.width: Theme.hairline
        }

        contentItem: ColumnLayout {
            spacing: 12

            Label {
                text: reportViewer.streaming ? qsTr("AI REPORT — GENERATING")
                                             : qsTr("AI REPORT")
                font.pointSize: Theme.caption
                font.letterSpacing: Theme.microLabelSpacing
                color: Theme.secondaryLabel
            }

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentHeight: reportText.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                TextArea {
                    id: reportText
                    width: parent.width
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    textFormat: TextEdit.MarkdownText
                    color: Theme.label
                    font.pointSize: Theme.subhead
                    background: null
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                Button {
                    id: exportReportButton
                    visible: reportViewer.reportId > 0
                    text: qsTr("Export .md")
                    onClicked: sessionSpectrumModel.exportReportMarkdown(reportViewer.reportId)
                    background: Rectangle {
                        radius: Theme.radiusControl
                        color: exportReportButton.down ? Theme.accentPressed : Theme.fill
                    }
                    contentItem: Text {
                        text: exportReportButton.text
                        font.pointSize: Theme.subhead
                        color: Theme.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                        rightPadding: 12
                    }
                }

                Button {
                    id: closeReportButton
                    text: reportViewer.streaming ? qsTr("Cancel") : qsTr("Close")
                    onClicked: {
                        if (reportViewer.streaming && aiAvailable)
                            reportService.cancel()
                        reportViewer.close()
                    }
                    background: Rectangle {
                        radius: Theme.radiusControl
                        color: closeReportButton.down ? Theme.accentPressed : Theme.fill
                    }
                    contentItem: Text {
                        text: closeReportButton.text
                        font.pointSize: Theme.subhead
                        color: Theme.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                        rightPadding: 12
                    }
                }
            }
        }
    }

    // Streaming glue — only instantiated when the AI build is present, so
    // reportService is never referenced otherwise.
    Loader {
        active: aiAvailable
        sourceComponent: Item {
            Connections {
                target: reportService
                function onStreamTextChanged() {
                    if (reportViewer.streaming)
                        reportText.text = reportService.streamText
                }
                function onReportSaved(sessionId) {
                    if (sessionId === detailPage.sessionId)
                        detailPage.reportsRefresh++
                    reportViewer.streaming = false
                }
                function onErrorOccurred(message) {
                    if (reportViewer.streaming) {
                        reportText.text = qsTr("**Error:** ") + message
                        reportViewer.streaming = false
                    }
                }
            }
        }
    }
}
