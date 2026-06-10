import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

PageTheme {
    id: movieAlbumPage

    toolbarTitle: qsTr("Recordings")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        anchors.rightMargin: Style.roundButtonWidth + 24
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Style.buttonBackground

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 12

                Label { text: qsTr("Name"); color: Style.text; font.bold: true; Layout.fillWidth: true }
                Label { text: qsTr("Duration"); color: Style.text; font.bold: true; Layout.preferredWidth: 100 }
                Label { text: qsTr("Recorded"); color: Style.text; font.bold: true; Layout.preferredWidth: 200 }
            }
        }

        ListView {
            id: movieList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: movieModel

            delegate: Rectangle {
                width: movieList.width
                height: 44
                color: index % 2 === 0 ? "#f5f5f5" : "#e7e7e7"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { text: name; color: "#202020"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.fillWidth: true }
                    Text { text: duration; color: "#202020"; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 100 }
                    Text { text: createdAt; color: "#202020"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 200 }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        pageStack.push("qrc:/MoviePage.qml",
                                       { movieName: name, movieUrl: url, movieIndex: index })
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: movieList.count === 0
                text: qsTr("No recordings yet — use the camera page to record.")
                color: Style.text
                font.pointSize: 14
            }
        }
    }
}
