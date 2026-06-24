import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import solid.broccoli 1.0
import "."

PageTheme {
    id: cameraPage

    property string albumName
    property int albumRowIndex

    toolbarTitle: qsTr("Camera")

    Component.onCompleted: {
        CameraService.attachVideoOutput(videoOutput)
        CameraService.setActive(true)
    }

    Component.onDestruction: {
        CameraService.stopRecording()
        CameraService.setActive(false)
    }

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

    GroupBox {
        id: videoGroupbox
        anchors.fill: parent
        anchors.margins: 12
        title: qsTr("Video Preview")

        label: Label {
            color: Style.pictureText
            text: videoGroupbox.title
            font.pointSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectFit
            autoOrientation: true

            // Mirror only the on-screen preview (a horizontal flip); captured
            // stills come from the camera stream and stay un-mirrored.
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

            Behavior on opacity {
                NumberAnimation { duration: 300 }
            }

            onOpacityChanged: if (opacity > 0.5) opacity = 0
        }

        Label {
            id: durationLabel
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 8
            color: Style.roundButtonRed
            font.pointSize: 16
            font.bold: true
            text: "● " + CameraService.recordingDuration
            visible: CameraService.recording
        }

        Label {
            id: errorLabel
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 8
            color: Style.roundButtonRed
            font.pointSize: 14
            visible: false

            Timer {
                id: errorTimer
                interval: 4000
                onTriggered: errorLabel.visible = false
            }
        }
    }

    Drawer {
        id: settingsDrawer
        edge: Qt.RightEdge
        width: Math.min(420, cameraPage.width * 0.6)
        height: cameraPage.height

        background: Rectangle {
            color: Style.pageBackground
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                text: qsTr("Camera Settings")
                color: Style.text
                font.pointSize: 18
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: qsTr("Device")
                color: Style.text
                font.pointSize: Style.fontSize
            }

            ComboBox {
                Layout.fillWidth: true
                model: CameraService.availableCameras
                currentIndex: CameraService.currentCameraIndex
                onActivated: function(index) {
                    CameraService.currentCameraIndex = index
                }
            }

            Label {
                text: qsTr("Resolution / Format")
                color: Style.text
                font.pointSize: Style.fontSize
            }

            ComboBox {
                Layout.fillWidth: true
                model: CameraService.availableFormats
                currentIndex: CameraService.currentFormatIndex
                onActivated: function(index) {
                    CameraService.currentFormatIndex = index
                }
            }

            Label {
                text: qsTr("Processors")
                color: Style.text
                font.pointSize: Style.fontSize
            }

            Label {
                text: qsTr("Processors transform the live preview only; captured photos stay original.")
                color: Style.text
                font.pointSize: Style.fontSize - 4
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                opacity: 0.7
            }

            ListView {
                id: processorList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: pluginModel
                spacing: 6

                delegate: RowLayout {
                    width: processorList.width
                    spacing: 8

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Label {
                            text: name
                            color: Style.text
                            font.pointSize: Style.fontSize
                        }
                        Label {
                            text: description
                            color: Style.text
                            font.pointSize: Style.fontSize - 4
                            opacity: 0.7
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
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
                    color: Style.text
                    opacity: 0.6
                }
            }
        }
    }

    toolbarButtons: ColumnLayout {
        spacing: 5

        RoundButton {
            id: savePicture
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/svg/save.svg"
            icon.color: Style.iconColor
            icon.width: Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }

            onClicked: {
                CameraService.captureImage()
            }
        }

        RoundButton {
            id: recordButton
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/svg/record.svg"
            icon.color: CameraService.recording ? Style.iconColor : Style.dangerColor
            icon.width: Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: CameraService.recording ? Style.roundButtonRed : Style.buttonBackground

                SequentialAnimation on opacity {
                    running: CameraService.recording
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.4; duration: 600 }
                    NumberAnimation { from: 0.4; to: 1.0; duration: 600 }
                    onRunningChanged: if (!running) parent.opacity = 1.0
                }
            }

            onClicked: {
                if (CameraService.recording) {
                    CameraService.stopRecording()
                } else {
                    CameraService.startRecording()
                }
            }
        }

        RoundButton {
            id: cameraSettings
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/svg/settings.svg"
            icon.color: Style.iconColor
            icon.width: Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.buttonBackground
            }

            onClicked: {
                settingsDrawer.open()
            }
        }
    }
}
