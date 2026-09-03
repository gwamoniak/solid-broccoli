import QtQuick
import solid.broccoli 1.0
import QtQuick.Layouts
import QtQuick.Controls
import "."

NavPage {
    id: movieAlbumPage
    pageTitle: qsTr("Videos")
    showLargeTitle: true

    ListView {
        id: movieList
        anchors.fill: parent
        anchors.topMargin: Theme.screenMargin
        clip: true
        model: AppContext.movieModel
        spacing: 1

        delegate: Rectangle {
            width: movieList.width
            height: Theme.rowHeight + 12
            color: Theme.surface

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.screenMargin
                anchors.rightMargin: Theme.screenMargin
                spacing: 12

                Rectangle {
                    Layout.preferredWidth: 44
                    Layout.preferredHeight: 44
                    radius: 6
                    color: Theme.fill

                    Label {
                        anchors.centerIn: parent
                        text: "▶"
                        font.pointSize: 16
                        color: Theme.secondaryLabel
                    }
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: name
                        font.pointSize: Theme.body
                        color: Theme.label
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Label {
                        text: duration + "  ·  " + createdAt
                        font.pointSize: Theme.footnote
                        color: Theme.secondaryLabel
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                Label {
                    text: ">"
                    font.pointSize: Theme.body
                    color: Theme.tertiaryLabel
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: Theme.screenMargin + 56
                anchors.right: parent.right
                height: Theme.hairline
                color: Theme.separator
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (owningStack)
                        owningStack.push("qrc:/MoviePage.qml",
                            { movieName: name, movieUrl: url, movieIndex: index, owningStack: owningStack })
                }
            }
        }

        Label {
            anchors.centerIn: parent
            visible: movieList.count === 0
            text: qsTr("No recordings yet")
            font.pointSize: Theme.body
            color: Theme.secondaryLabel
        }
    }
}
