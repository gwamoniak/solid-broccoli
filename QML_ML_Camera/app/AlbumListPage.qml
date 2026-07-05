import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

NavPage {
    pageTitle: qsTr("Photos")
    showLargeTitle: true

    trailing: Component {
        ToolButton {
            icon.source: "qrc:/images/svg/add-album.svg"
            icon.color: Theme.accent
            icon.width: 22
            icon.height: 22
            background: null
            onClicked: newAlbumDialog.open()
        }
    }

    InputDialog {
        id: newAlbumDialog
        title: qsTr("New album")
        label: qsTr("Album name:")
        hint: qsTr("My Album")
        onAccepted: {
            editText.focus = false
            albumModel.addAlbumFromName(editText.text)
        }
    }

    GridView {
        id: albumGrid
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        cellWidth: (width - Theme.gridGap) / 2
        cellHeight: cellWidth + 48
        model: albumModel

        delegate: Item {
            width: albumGrid.cellWidth
            height: albumGrid.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: Theme.gridGap / 2
                radius: Theme.radiusCard
                color: Theme.surface

                Column {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        width: parent.width
                        height: parent.width
                        radius: Theme.radiusCard
                        color: Theme.fill

                        Label {
                            anchors.centerIn: parent
                            text: name.charAt(0).toUpperCase()
                            font.pointSize: 32
                            color: Theme.secondaryLabel
                        }
                    }

                    Label {
                        text: name
                        font.pointSize: Theme.subhead
                        font.weight: Font.Medium
                        color: Theme.label
                        elide: Text.ElideRight
                        width: parent.width
                        leftPadding: 10
                        topPadding: 8
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        pictureModel.setAlbumId(id)
                        if (owningStack) {
                            owningStack.push("qrc:/AlbumPage.qml",
                                { albumName: name, albumRowIndex: index, owningStack: owningStack })
                        }
                    }
                }
            }
        }

        Label {
            anchors.centerIn: parent
            visible: albumGrid.count === 0
            text: qsTr("No albums yet")
            font.pointSize: Theme.body
            color: Theme.secondaryLabel
        }
    }
}
