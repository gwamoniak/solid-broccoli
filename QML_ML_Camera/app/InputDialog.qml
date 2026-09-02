import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."


Dialog {
    id: rootDialog
    property string label: "New item"
    property string hint: "value"
    property alias editText: editTextItem

    modal: true
    width: Math.min(520, parent ? parent.width - 64 : 520)
    height: 220
    anchors.centerIn: parent
    standardButtons: Dialog.NoButton

    onVisibleChanged: {
        if (visible) {
            editTextItem.forceActiveFocus()
            editTextItem.selectAll()
        }
    }

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: 6
        border.color: Theme.separator
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Label {
            text: label
            color: Style.text
            font.pointSize: 16
            Layout.fillWidth: true
        }

        TextField {
            id: editTextItem
            text: hint
            color: Style.text
            selectByMouse: true
            font.pointSize: 18
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhNone
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Cancel")
                onClicked: rootDialog.reject()
            }

            Button {
                text: qsTr("OK")
                highlighted: true
                onClicked: rootDialog.accept()
            }
        }
    }
}
