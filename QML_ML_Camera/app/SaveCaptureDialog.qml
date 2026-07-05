import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

// Name + tags entry for saving a capture into the session store.
Dialog {
    id: saveDialog
    property alias nameText: nameField.text
    property alias tagsText: tagsField.text

    modal: true
    width: Math.min(440, parent ? parent.width - 64 : 440)
    anchors.centerIn: parent
    standardButtons: Dialog.NoButton

    onVisibleChanged: {
        if (visible) {
            nameField.text = ""
            tagsField.text = ""
            nameField.forceActiveFocus()
        }
    }

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: Theme.radiusSheet
        border.color: Theme.separator
        border.width: Theme.hairline
    }

    contentItem: ColumnLayout {
        spacing: 12

        Label {
            text: qsTr("Save capture")
            font.pointSize: Theme.headline
            font.weight: Font.DemiBold
            color: Theme.label
        }

        TextField {
            id: nameField
            placeholderText: qsTr("Name (optional)")
            font.pointSize: Theme.body
            color: Theme.label
            placeholderTextColor: Theme.tertiaryLabel
            selectByMouse: true
            Layout.fillWidth: true
            background: Rectangle {
                color: Theme.fill
                radius: Theme.radiusControl
            }
        }

        TextField {
            id: tagsField
            placeholderText: qsTr("Tags, comma separated (optional)")
            font.pointSize: Theme.body
            color: Theme.label
            placeholderTextColor: Theme.tertiaryLabel
            selectByMouse: true
            Layout.fillWidth: true
            background: Rectangle {
                color: Theme.fill
                radius: Theme.radiusControl
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }

            Button {
                id: cancelBtn
                text: qsTr("Cancel")
                onClicked: saveDialog.reject()
                background: Rectangle { radius: Theme.radiusControl; color: Theme.fill }
                contentItem: Text {
                    text: cancelBtn.text
                    font.pointSize: Theme.subhead
                    color: Theme.label
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12; rightPadding: 12
                }
            }

            Button {
                id: saveBtn
                text: qsTr("Save")
                onClicked: saveDialog.accept()
                background: Rectangle {
                    radius: Theme.radiusControl
                    color: saveBtn.down ? Theme.accentPressed : Theme.accent
                }
                contentItem: Text {
                    text: saveBtn.text
                    font.pointSize: Theme.subhead
                    color: Theme.textOverAccent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12; rightPadding: 12
                }
            }
        }
    }
}
