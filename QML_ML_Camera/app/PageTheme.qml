import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

Page {
    id: page
    clip: true

    property alias toolbarButtons: buttonsLoader.sourceComponent
    property alias toolbarTitle: titleLabel.text

    background: Rectangle {
        color: Style.pageBackground
    }

    header: ToolBarTheme {
        id: toolBarTheme
        height: 86

        Label {
            id: titleLabel
            anchors.centerIn: parent
            color: Style.text
            elide: Text.ElideRight
            font.pointSize: 20
            horizontalAlignment: Text.AlignHCenter
            width: parent.width - (Style.roundButtonWidth + 24) * 2
        }

        ColumnLayout {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.rightMargin: 8
            spacing: 5
            width: Style.roundButtonWidth
            z: 10

            RoundButton {
                id: backButton
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                Layout.preferredHeight: Style.roundButtonHeight
                Layout.preferredWidth: Style.roundButtonWidth
                icon.source: "qrc:/images/png/back.png"
                icon.width: Style.roundButtonWidth - 12
                icon.height: Style.roundButtonHeight - 12
                background: Rectangle {
                    radius: Style.roundButtonRadius
                    color: Style.buttonBackground
                }

                onClicked: {
                    if (stackView.depth > 1) {
                        stackView.pop()
                    } else {
                        window.goHome()
                    }
                }
            }

            Loader {
                id: buttonsLoader
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                Layout.preferredWidth: Style.roundButtonWidth
                clip: true
            }
        }
    }

    footer: ToolBar {
        height: 74
        background: Rectangle {
            color: Style.toolBackground
        }

        RoundButton {
            id: homeButton
            width: Style.roundButtonWidth
            height: Style.roundButtonHeight
            anchors.centerIn: parent
            icon.source: "qrc:/images/png/home_button.png"
            icon.width: Style.roundButtonWidth - 12
            icon.height: Style.roundButtonHeight - 12
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonHome
            }

            onClicked: {
                window.goHome()
            }
        }
    }
}
