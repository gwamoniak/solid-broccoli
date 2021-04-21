import QtQuick 2.6
import QtQuick.Controls 2.2

ItemDelegate {
    id: titleText
    width: parent.width
    topPadding: 5

    Switch {
        id: control
        topPadding: 30
        rightPadding: 30
        width: 250
        height: 40
        checked: true
        wheelEnabled: false
        transformOrigin: Item.Center
        scale: 1
        anchors.rightMargin: 33
        anchors.right: parent.right
        contentItem: Text {
                  anchors.left: control.right
                  leftPadding: -15
                  topPadding:  -15
                  text: control.checked ? qsTr("OFF") : qsTr("ON")
                  font.family: "Times New Roman"
                  font.bold: true
                  font.pointSize: 12
                  color: control.checked ? "red" : "green"
              }

              indicator: Rectangle {
                  implicitWidth: 130
                  implicitHeight: 26
                  x: control.width - width - control.rightPadding
                  y: parent.height / 2 - height / 2
                  radius: 13
                  border.color: "lightblue"
                  color: control.checked ? "pink" : "lightgreen"

                  Rectangle {
                      x: control.checked ? parent.width - width : 0
                      width:  30
                      height: 30
                      radius: 13
                      color: control.checked ? "red" : "green"
                  }
              }

    }

}
