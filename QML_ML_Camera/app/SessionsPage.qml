import QtQuick
import solid.broccoli 1.0
import QtQuick.Layouts
import QtQuick.Controls
import "."

// The lab notebook: one row per session, newest first.
NavPage {
    id: sessionsPage
    pageTitle: qsTr("Sessions")
    showLargeTitle: true

    StackView.onActivating: AppContext.sessionModel.refresh()

    ListView {
        id: sessionList
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        spacing: 10
        model: AppContext.sessionModel
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        delegate: Rectangle {
            id: sessionRow
            required property int index
            required property int id
            required property string name
            required property string created
            required property int count

            width: sessionList.width
            height: 64
            radius: Theme.radiusCard
            color: Theme.surface

            MouseArea {
                anchors.fill: parent
                onClicked: sessionsPage.owningStack.push("qrc:/SessionDetailPage.qml", {
                    owningStack: sessionsPage.owningStack,
                    sessionId: sessionRow.id,
                    sessionName: sessionRow.name,
                    sessionRow: sessionRow.index
                })
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Column {
                    spacing: 2
                    Layout.fillWidth: true
                    Label {
                        text: sessionRow.name
                        font.pointSize: Theme.body
                        color: Theme.label
                        elide: Text.ElideRight
                    }
                    Label {
                        text: sessionRow.created
                        font.pointSize: Theme.caption
                        color: Theme.secondaryLabel
                    }
                }

                Label {
                    text: sessionRow.count
                    font.family: Theme.readoutFontName
                    font.pointSize: Theme.subhead
                    color: Theme.accent
                }
                Label {
                    text: ">"
                    font.pointSize: Theme.body
                    color: Theme.secondaryLabel
                }
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 8
        visible: sessionList.count === 0

        Label {
            text: qsTr("No Sessions")
            font.pointSize: Theme.title
            color: Theme.secondaryLabel
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: qsTr("Save a capture from the Live tab to start one.")
            font.pointSize: Theme.subhead
            color: Theme.tertiaryLabel
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
