import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import solid.broccoli 1.0
import "."

NavPage {
    id: cameraPage
    showNavBar: false
    background: Rectangle { color: "#000000" }

    Component.onCompleted: {
        CameraService.attachVideoOutput(videoOutput)
        CameraService.setActive(true)
    }

    Component.onDestruction: {
        CameraService.stopRecording()
        CameraService.setActive(false)
    }

    property bool photoMode: true

    Connections {
        target: CameraService
        function onCaptureError(message) {
            errorLabel.text = message
            errorLabel.visible = true
            errorTimer.restart()
        }
        function onImageSaved(filePath) {
            if (AppSettings.shutterFlash)
                flashOverlay.opacity = 0.8
        }
    }

    // ── Full-bleed viewfinder ──
    VideoOutput {
        id: videoOutput
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: controlBar.top
        fillMode: VideoOutput.PreserveAspectCrop

        transform: Scale {
            origin.x: videoOutput.width / 2
            xScale: AppSettings.mirrorPreview ? -1 : 1
        }
    }

    Rectangle {
        id: flashOverlay
        anchors.fill: videoOutput
        color: "white"
        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 300 } }
        onOpacityChanged: if (opacity > 0.5) opacity = 0
    }

    // Recording duration badge
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        visible: CameraService.recording
        width: durationLabel.implicitWidth + 24
        height: 28
        radius: 14
        color: Theme.destructive

        Label {
            id: durationLabel
            anchors.centerIn: parent
            text: "● " + CameraService.recordingDuration
            color: "#FFFFFF"
            font.pointSize: Theme.footnote
            font.weight: Font.Medium
        }
    }

    Label {
        id: errorLabel
        anchors.bottom: videoOutput.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 12
        color: Theme.destructive
        font.pointSize: Theme.subhead
        visible: false
        Timer {
            id: errorTimer
            interval: 4000
            onTriggered: errorLabel.visible = false
        }
    }

    // ── Bottom control bar ──
    Rectangle {
        id: controlBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 160
        color: "#000000"

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 8
            spacing: 16

            // Photo / Video mode switch
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 24

                Label {
                    text: qsTr("PHOTO")
                    font.pointSize: Theme.subhead
                    font.weight: Font.Medium
                    color: cameraPage.photoMode ? Theme.accent : Theme.secondaryLabel
                    MouseArea {
                        anchors.fill: parent
                        onClicked: cameraPage.photoMode = true
                    }
                }
                Label {
                    text: qsTr("VIDEO")
                    font.pointSize: Theme.subhead
                    font.weight: Font.Medium
                    color: !cameraPage.photoMode ? Theme.accent : Theme.secondaryLabel
                    MouseArea {
                        anchors.fill: parent
                        onClicked: cameraPage.photoMode = false
                    }
                }
            }

            // Shutter / record + settings
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 48, Theme.maxContentWidth)
                spacing: 0

                Item { Layout.fillWidth: true }

                // Shutter / record button
                Rectangle {
                    width: Theme.shutterSize
                    height: Theme.shutterSize
                    radius: Theme.shutterSize / 2
                    color: "transparent"
                    border.width: Theme.accentRing
                    border.color: cameraPage.photoMode ? Theme.accent : Theme.destructive

                    Rectangle {
                        anchors.centerIn: parent
                        width: cameraPage.photoMode ? Theme.shutterSize - 12 : 28
                        height: cameraPage.photoMode ? Theme.shutterSize - 12 : 28
                        radius: cameraPage.photoMode ? (Theme.shutterSize - 12) / 2 : 6
                        color: cameraPage.photoMode ? "#FFFFFF" : Theme.destructive

                        Behavior on width  { NumberAnimation { duration: 150 } }
                        Behavior on height { NumberAnimation { duration: 150 } }
                        Behavior on radius { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (cameraPage.photoMode) {
                                CameraService.captureImage()
                            } else {
                                if (CameraService.recording)
                                    CameraService.stopRecording()
                                else
                                    CameraService.startRecording()
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Settings gear
                ToolButton {
                    icon.source: "qrc:/images/svg/settings.svg"
                    icon.color: "#FFFFFF"
                    icon.width: 26; icon.height: 26
                    background: null
                    onClicked: settingsDrawer.open()
                }

                Item { Layout.preferredWidth: 16 }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // ── Camera settings drawer ──
    Drawer {
        id: settingsDrawer
        edge: Qt.BottomEdge
        width: parent.width
        height: Math.min(400, cameraPage.height * 0.5)

        background: Rectangle {
            color: Theme.surface
            radius: Theme.radiusSheet
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.radiusSheet
                color: Theme.surface
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: 36; height: 5
                radius: 3
                color: Theme.fill
            }

            Label {
                text: qsTr("Camera Settings")
                font.pointSize: Theme.headline
                font.weight: Font.DemiBold
                color: Theme.label
            }

            Label { text: qsTr("Device"); font.pointSize: Theme.footnote; color: Theme.secondaryLabel }
            ComboBox {
                Layout.fillWidth: true
                model: CameraService.availableCameras
                currentIndex: CameraService.currentCameraIndex
                onActivated: function(idx) { CameraService.currentCameraIndex = idx }
            }

            Label { text: qsTr("Resolution"); font.pointSize: Theme.footnote; color: Theme.secondaryLabel }
            ComboBox {
                Layout.fillWidth: true
                model: CameraService.availableFormats
                currentIndex: CameraService.currentFormatIndex
                onActivated: function(idx) { CameraService.currentFormatIndex = idx }
            }

            Label { text: qsTr("Processors"); font.pointSize: Theme.footnote; color: Theme.secondaryLabel }

            ListView {
                id: processorList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: pluginModel
                spacing: 4
                delegate: RowLayout {
                    width: processorList.width
                    spacing: 8
                    Column {
                        Layout.fillWidth: true
                        Label { text: name; font.pointSize: Theme.body; color: Theme.label }
                        Label { text: description; font.pointSize: Theme.caption; color: Theme.secondaryLabel; wrapMode: Text.WordWrap }
                    }
                    Switch {
                        checked: model.enabled
                        onToggled: pluginModel.setEnabled(index, checked)
                    }
                }
                Label {
                    anchors.centerIn: parent
                    visible: processorList.count === 0
                    text: qsTr("No processors installed")
                    color: Theme.secondaryLabel
                }
            }
        }
    }
}
