import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

// A modern settings row: a label on the left and an iOS-style pill toggle on
// the right. Recolored from Style so it matches the rest of the app.
ItemDelegate {
    id: row

    // Two-way contract with the backing setting: bind `value` to the C++
    // property and handle `toggled` to write it back. (`checked` is FINAL on
    // ItemDelegate, so the state lives on the inner Switch instead.)
    property bool value: false
    signal toggled(bool checkedState)

    implicitHeight: 48
    padding: 0
    background: null

    contentItem: RowLayout {
        spacing: 12

        Label {
            text: row.text
            color: Style.text
            font.family: Style.fontName
            font.pointSize: Style.fontSize
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Switch {
            id: control
            checked: row.value
            onToggled: row.toggled(checked)

            // Collapse the control to exactly the pill so it lines up flush
            // with the rest of the column and never spills past the drawer edge.
            padding: 0
            implicitWidth: 48
            implicitHeight: 28
            contentItem: Item {}

            indicator: Rectangle {
                width: control.width
                height: control.height
                radius: height / 2
                color: control.checked ? Style.buttonBackground
                                       : Qt.rgba(1, 1, 1, 0.22)
                Behavior on color { ColorAnimation { duration: 150 } }

                Rectangle {
                    width: 22
                    height: 22
                    radius: height / 2
                    y: 3
                    x: control.checked ? parent.width - width - 3 : 3
                    color: "white"
                    Behavior on x {
                        NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                    }
                }
            }
        }
    }
}
