import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import "."

NavPage {
    property string albumName
    property int albumRowIndex
    pageTitle: albumName

    trailing: Component {
        Row {
            spacing: 4
            ToolButton {
                icon.source: "qrc:/images/svg/add-photo.svg"
                icon.color: Theme.accent
                icon.width: 22; icon.height: 22
                background: null
                onClicked: fileDialog.open()
            }
            ToolButton {
                icon.source: "qrc:/images/svg/rename.svg"
                icon.color: Theme.accent
                icon.width: 20; icon.height: 20
                background: null
                onClicked: renameAlbumDialog.open()
            }
            ToolButton {
                icon.source: "qrc:/images/svg/delete.svg"
                icon.color: Theme.destructive
                icon.width: 20; icon.height: 20
                background: null
                onClicked: {
                    albumModel.removeRows(albumRowIndex, 1)
                    if (owningStack) owningStack.pop()
                }
            }
        }
    }

    InputDialog {
        id: renameAlbumDialog
        title: qsTr("Rename album")
        label: qsTr("Album name:")
        hint: albumName
        onAccepted: {
            editText.focus = false
            albumModel.rename(albumRowIndex, editText.text)
            albumName = editText.text
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open file")
        onAccepted: {
            pictureModel.addPictureFromUrl(fileDialog.selectedFile)
        }
    }

    GridView {
        id: photoGrid
        anchors.fill: parent
        anchors.margins: 2
        cellWidth: Math.floor(width / Math.max(3, Math.floor(width / 160)))
        cellHeight: cellWidth
        model: pictureModel

        delegate: Item {
            width: photoGrid.cellWidth
            height: photoGrid.cellHeight

            Image {
                anchors.fill: parent
                anchors.margins: 1
                fillMode: Image.PreserveAspectCrop
                cache: false
                source: "image://pictures/" + index + "/thumbnail"
                asynchronous: true

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 28
                    color: Qt.rgba(0, 0, 0, 0.5)

                    Label {
                        anchors.centerIn: parent
                        text: name
                        color: "#FFFFFF"
                        font.pointSize: Theme.caption
                        elide: Text.ElideRight
                        width: parent.width - 8
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (owningStack) {
                        owningStack.push("qrc:/PicturePage.qml",
                            { pictureName: name, pictureIndex: index, owningStack: owningStack })
                    }
                }
            }
        }

        Label {
            anchors.centerIn: parent
            visible: photoGrid.count === 0
            text: qsTr("No photos yet")
            font.pointSize: Theme.body
            color: Theme.secondaryLabel
        }
    }
}
