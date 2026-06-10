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

            Item { Layout.fillHeight: true }
        }
    }

    toolbarButtons: ColumnLayout {
        spacing: 5

        RoundButton {
            id: savePicture
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/png/save_photo.png"
            icon.width: Style.roundButtonWidth - 15
            icon.height: Style.roundButtonHeight - 15
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }

            onClicked: {
                CameraService.captureImage()
            }
        }

        RoundButton {
            id: cameraSettings
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/png/cogwheel.png"
            icon.width: Style.roundButtonWidth - 15
            icon.height: Style.roundButtonHeight - 15
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
