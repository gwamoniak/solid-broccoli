import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtMultimedia
import QtQuick.Controls.Material 2.2
import QtQuick.Dialogs
import Qt.labs.settings 1.0
import solid.broccoli 1.0
import "."

PageTheme {
    width: window.width
    property string pictureName

    MediaDevices {
        id: mediaDevices
    }

    RoundButton {
        id: drawerOpen
        Layout.preferredHeight:  Style.roundButtonHeight
        Layout.preferredWidth:   Style.roundButtonWidth
        //text: qsTr("::")
        //font.pointSize: 18
        Layout.alignment: Qt.AlignRight| Qt.AlignTop
        icon.source:"qrc:/images/svg/settings.svg"
        icon.color: Style.iconColor
        icon.width : Style.iconSize
        icon.height: Style.iconSize
        background: Rectangle {
            radius: Style.roundButtonRadius
            color: Style.buttonBackground
        }

        onClicked: settingsDrawer.opened ? settingsDrawer.close()
                                         : settingsDrawer.open()
    }

    toolbarButtons:ColumnLayout{
        spacing: 3

        RoundButton {
            id:pictureAlbumButton
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            smooth: true
            antialiasing: true
            //text: qsTr("ALBUM")
            Layout.alignment: Qt.AlignLeft | Qt.AnchorTop
            icon.source: "qrc:/images/svg/gallery.svg"
            icon.color: Style.iconColor
            icon.width : Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }

            //font.pointSize: 18
            //anchors.right: quit.left
            //rightPadding: 5
            onClicked: {
                console.log("Navigation: opening AlbumListPage")
                pageStack.replace("qrc:/AlbumListPage.qml", {}, StackView.Immediate)
            }

        }
        RoundButton {
            id: movieAlbumButton
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            smooth: true
            antialiasing: true
            Layout.alignment: Qt.AlignLeft | Qt.AnchorTop
            icon.source: "qrc:/images/svg/movie.svg"
            icon.color: Style.iconColor
            icon.width: Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }
            onClicked: {
                console.log("Navigation: opening MovieAlbumPage")
                pageStack.replace("qrc:/MovieAlbumPage.qml", {}, StackView.Immediate)
            }
        }
        RoundButton {
            id: cameraPage
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            smooth: true

            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
            icon.source: "qrc:/images/svg/camera.svg"
            icon.color: Style.iconColor
            icon.width : Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonYellow
            }
            onClicked: {
                console.log("Navigation: opening CameraPage")
                pageStack.replace("qrc:/CameraPage.qml", {}, StackView.Immediate)
            }
        }

        RoundButton {
            id: quit
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            smooth: true
            //text: qsTr("-Quit-")
            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
            icon.source: "qrc:/images/svg/quit.svg"
            icon.color: Style.iconColor
            icon.width : Style.iconSize
            icon.height: Style.iconSize
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonRed
            }

            onClicked: {
                Qt.quit()
            }
        }
    }

    Drawer {
        id: settingsDrawer
        y: header.height
        height: window.height - header.height
        width: Math.max(320, 0.32 * window.width)

        background: Rectangle {
            color: Style.pageBackground
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Label {
                text: qsTr("Settings")
                color: Style.text
                font.family: Style.fontName
                font.pointSize: 22
                font.bold: true
                Layout.bottomMargin: 4
            }

            Label {
                text: qsTr("CAMERA")
                color: Style.text
                opacity: 0.6
                font.family: Style.fontName
                font.pointSize: Style.fontSize - 4
                font.bold: true
            }

            ComboBox {
                id: cameraCombo
                Layout.fillWidth: true
                model: mediaDevices.videoInputs
                textRole: "description"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.topMargin: 4
                height: 1
                color: Qt.rgba(1, 1, 1, 0.12)
            }

            Label {
                text: qsTr("GENERAL")
                color: Style.text
                opacity: 0.6
                font.family: Style.fontName
                font.pointSize: Style.fontSize - 4
                font.bold: true
            }

            SettingSwitcher {
                Layout.fillWidth: true
                text: qsTr("Shutter flash")
                value: AppSettings.shutterFlash
                onToggled: AppSettings.shutterFlash = checkedState
            }

            SettingSwitcher {
                Layout.fillWidth: true
                text: qsTr("Mirror preview")
                value: AppSettings.mirrorPreview
                onToggled: AppSettings.mirrorPreview = checkedState
            }

            Item { Layout.fillHeight: true }

            Button {
                id: loggerButton
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: qsTr("Open Logger")
                font.family: Style.fontName
                font.pointSize: Style.fontSize
                icon.source: "qrc:/images/svg/log.svg"
                icon.color: Style.iconColor
                icon.width: Style.fontSize + 4
                icon.height: Style.fontSize + 4
                palette.buttonText: Style.iconColor

                background: Rectangle {
                    radius: 10
                    color: loggerButton.down ? Qt.darker(Style.buttonBackground, 1.2)
                                             : Style.buttonBackground
                }

                onClicked: {
                    console.log("Navigation: opening LoggerPage")
                    pageStack.replace("qrc:/LoggerPage.qml", {}, StackView.Immediate)
                    settingsDrawer.close()
                }
            }
        }

    }


}
