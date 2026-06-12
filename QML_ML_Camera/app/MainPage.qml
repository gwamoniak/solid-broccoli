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

        onClicked: {
            settingsDrawer.open()
            if(settingsDrawer.opened)
                settingsDrawer.close()
        }
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
        width: 0.27* window.width
        Rectangle {
            id: drawerRect
            anchors.fill: parent
            color: Style.toolBackground

            Column{
                id: settingButtons
                spacing: 1
                //height:0.15 *window.width
                ComboBox
                {
                    id: cameraCombo
                    width: drawerRect.width
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignRight| Qt.AlignTop
                    model: mediaDevices.videoInputs
                    textRole: "description"
                    delegate: ItemDelegate
                    {
                        text: modelData.description
                    }

                }
                Button {
                    id: loggerButton
                    width: drawerRect.width
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignRight| Qt.AlignTop
                    text: qsTr("Logger")
                    smooth: true

                    onClicked: {
                        console.log("Navigation: opening LoggerPage")
                        pageStack.replace("qrc:/LoggerPage.qml", {}, StackView.Immediate)
                        settingsDrawer.close()
                    }
                }

            }
            ListView {
                id: listView
                width: settingsDrawer.width
                height: settingsDrawer.height - settingButtons.height
                anchors.top: settingButtons.bottom
                model:2

                delegate: SettingSwitcher{

                    text: qsTr("Title of Setting %1").arg(index + 1)
                    font.bold: true
                    font.pointSize: 10

                }

                ScrollIndicator.vertical: ScrollIndicator { }

            }
        }

    }


}
