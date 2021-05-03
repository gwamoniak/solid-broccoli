import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import "."

PageTheme {

    property string albumName
    property int albumRowIndex

    toolbarTitle: albumName
    toolbarButtons: ColumnLayout {
        RoundButton{
            id: photoAdd
            //text: qsTr("ADD")
            //font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            antialiasing: true
            icon.source:"qrc:/images/png/add_photo.png"
            icon.width :Style.roundButtonWidth
            icon.height:Style.roundButtonHeight
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }

            onClicked: {
                dialog.open()
            }
        }

        RoundButton{
            id: renameAlbum
//            text: qsTr("RENAME")
//            font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            antialiasing: true
            icon.source:"qrc:/images/png/rename_album_gallery.png"
            icon.width :Style.roundButtonWidth -20
            icon.height:Style.roundButtonHeight
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonYellow
            }
            onClicked: {
                renameAlbumDialog.open()
                     }
        }
        RoundButton {
            id: removeAlbum
            //text: qsTr("DELETE")
            //font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  Style.roundButtonHeight
            Layout.preferredWidth:   Style.roundButtonWidth
            antialiasing: true
            icon.source:"qrc:/images/png/delete_album_gallery.png"
            icon.width :Style.roundButtonWidth
            icon.height:Style.roundButtonHeight
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonRed
            }
            onClicked: {
                albumModel.removeRows(albumRowIndex, 1)
                stackView.pop()
            }
        }

    }

    InputDialog {
        id: renameAlbumDialog
        title: "Rename album"
        label: "Album name:"
        hint: albumName

        onAccepted: {
            editText.focus = false
            albumModel.rename(albumRowIndex, editText.text)
            albumName = editText.text
        }
    }

    FileDialog {
        id: dialog
        title: "Open file"
        //folder: shortcuts.pictures
        onAccepted: {
            var pictureUrl = dialog.fileUrl
//            var id = pictureModel.getAlbumId()
//            console.log("albumId",id)
            pictureModel.addPictureFromUrl(pictureUrl)
            dialog.close()
        }
    }

    GridView {
        id: thumbnailList
        model: pictureModel
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 30
        cellWidth:  thumbnailSize
        cellHeight: thumbnailSize

        delegate: Rectangle {
            id: rect
            width:  thumbnailList.cellWidth  - 10
            height: thumbnailList.cellHeight - 10
            color: "transparent"

            Image{
                id: thumbnail
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                source: "image://pictures/" + index + "/thumbnail"

            }
            Rectangle{
                id: fileText
                color: "black"
                width: rect.width
                height: rect.height/6
                anchors{
                    bottom: thumbnail.bottom;
                }
                Text {
                    id: txt;
                    text: name;
                    color: Style.text;
                    elide: Text.ElideRight;
                    height: (width * 1 / 4);
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    maximumLineCount: 2;
                    verticalAlignment: Text.AlignVCenter;
                    horizontalAlignment: Text.AlignHCenter;
                    font {
                        family: Style.fontName;
                        pixelSize: Style.fontSize;
                    }
                    anchors {
                        top: fileText.top
                        left: fileText.left;
                        right: fileText.right;
                        bottom: fileText.bottom;
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    thumbnailList.currentIndex = index
                    pageStack.push("qrc:/PicturePage.qml", { pictureName: name, pictureIndex: index })
                }
            }

        }

    }

}
