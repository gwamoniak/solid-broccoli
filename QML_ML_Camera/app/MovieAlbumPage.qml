import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtMultimedia 5.9
import "."



PageTheme {

    property string movieAlbumName
    property int albumRowIndex
    property var movieUrl
    property var thumbnailUrl
    property string url


    toolbarTitle: movieAlbumName
    toolbarButtons: ColumnLayout {
        RoundButton{
            id: movieAdd
            //text: qsTr("ADD")
            //font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            //antialiasing: true
            background: Image {
                source: "qrc:/images/png/add_movie.png"
                width:  65
                height: 65
                smooth: true
            }

            onClicked: {
                dialogMovie.open()
            }
        }

        RoundButton{
            id: renameMovieAlbum
//            text: qsTr("RENAME")
//            font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            //antialiasing: true
            background: Image {
                source: "qrc:/images/png/rename_movie_gallery.png"
                width:  65
                height: 65
                smooth: true
            }
            onClicked: {
                renameAlbumDialog.open()
                     }
        }
        RoundButton {
            id: removeMovieAlbum
            //text: qsTr("DELETE")
            //font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            //antialiasing: true
            background: Image {
                source: "qrc:/images/png/delete_movie_gallery.png"
                width: 65
                height: 65
                smooth: true
            }
            onClicked: {
                movieAlbumModel.removeRows(albumRowIndex, 1)
                stackView.pop()
            }
        }

    }

    InputDialog {
        id: renameAlbumDialog
        title: "Rename Movie album"
        label: "Movie album name:"
        hint: movieAlbumName

        onAccepted: {
            editText.focus = false
            movieAlbumModel.rename(albumRowIndex, editText.text)
            movieAlbumName = editText.text
        }
    }

    FileDialog {
        id: dialogMovie
        title: "Open file"
        //folder: shortcuts.pictures
        onAccepted: {
            movieUrl = dialogMovie.fileUrl
            if(movieProcessor.getLocalFileForUrl(movieUrl))
            {
                thumbnailUrl = movieProcessor.getThumbnail()

                movieModel.addMovieFromUrl(movieUrl,thumbnailUrl)
            }
            else
            {
                // popup message box for error
                messageWDialog.open()
            }
            dialogMovie.close()

        }
    }

    MessageDialog {
        id: messageWDialog
        icon: StandardIcon.Warning
        title: "Warning!!!"
        text:  "The Movie has not been added to the album. It might be a problem to read a frame"
        onAccepted: {
            console.log("User accepted or other")
         }

    }

    GridView {
        id: thumbnailList
        model: movieModel
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 30
        cellWidth:  thumbnailSize
        cellHeight: thumbnailSize

        delegate: Rectangle {
            id: rectM
            width:  thumbnailList.cellWidth  - 10
            height: thumbnailList.cellHeight - 10
            color: "transparent"

            Image{
                id: thumbnail
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                source: "image://movies/" + index + "/thumbnail"

            }
            Rectangle{
                id: fileText
                color: "black"
                width: rectM.width
                height: rectM.height/6
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
                    pageStack.push("qrc:/MoviePage.qml", { movieName: name, movieIndex: index })
                }
            }

        }

    }

}
