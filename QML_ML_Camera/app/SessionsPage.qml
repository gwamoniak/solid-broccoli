import QtQuick
import QtQuick.Controls
import "."

// Placeholder until Milestone 7 brings the session store.
NavPage {
    pageTitle: qsTr("Sessions")
    showLargeTitle: true

    Column {
        anchors.centerIn: parent
        spacing: 8

        Label {
            text: qsTr("No Sessions")
            font.pointSize: Theme.title
            color: Theme.secondaryLabel
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: qsTr("Saved captures will appear here.")
            font.pointSize: Theme.subhead
            color: Theme.tertiaryLabel
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
