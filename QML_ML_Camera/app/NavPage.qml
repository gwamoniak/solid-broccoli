import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

Page {
    id: navPage
    clip: true

    property string pageTitle: ""
    property bool showLargeTitle: false
    property bool showNavBar: true
    property Component trailing: null
    property StackView owningStack: null

    background: Rectangle { color: Theme.groupedBackground }

    header: Item {
        visible: navPage.showNavBar
        implicitHeight: navBar.implicitHeight + (largeTitleRow.visible ? largeTitleRow.implicitHeight : 0)

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                id: navBar
                width: parent.width
                implicitHeight: Theme.navBarHeight
                color: Theme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 4

                    ToolButton {
                        id: backBtn
                        visible: navPage.owningStack ? navPage.owningStack.depth > 1 : false
                        implicitWidth: 44
                        implicitHeight: 44
                        icon.source: "qrc:/images/svg/back.svg"
                        icon.color: Theme.accent
                        icon.width: 20
                        icon.height: 20
                        background: null
                        onClicked: navPage.owningStack.pop()
                    }

                    Item { visible: !backBtn.visible; implicitWidth: 8 }

                    Label {
                        visible: !navPage.showLargeTitle
                        text: navPage.pageTitle
                        font.pointSize: Theme.headline
                        font.weight: Font.DemiBold
                        color: Theme.label
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Item { visible: navPage.showLargeTitle; Layout.fillWidth: true }

                    Loader {
                        sourceComponent: navPage.trailing
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: Theme.hairline
                    color: Theme.separator
                }
            }

            Rectangle {
                id: largeTitleRow
                visible: navPage.showLargeTitle
                width: parent.width
                implicitHeight: 48
                color: Theme.surface

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.screenMargin
                    anchors.verticalCenter: parent.verticalCenter
                    text: navPage.pageTitle
                    font.pointSize: Theme.largeTitle
                    font.weight: Font.Bold
                    color: Theme.label
                }
            }
        }
    }
}
