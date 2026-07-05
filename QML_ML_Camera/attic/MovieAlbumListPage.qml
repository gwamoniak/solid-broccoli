import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import "."

PageTheme {
    toolbarTitle: "Movie Albums"
    toolbarButtons: ToolButton {
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        //text:qsTr("ADD")
        //font.pointSize: 18
        Layout.preferredHeight:  65
        Layout.preferredWidth:   65
        antialiasing: true
        background: Image {
            source: "qrc:/images/png/add_movie_gallery.png"
            width: 65
            height: 65
            smooth: true
        }
        onClicked: {
            newMovieAlbumDialog.open()
        }

    }

    InputDialog {
        id: newMovieAlbumDialog
        title: "New movie album"
        label: "Movies Album name:"
        hint: "My Movies Album"

        onAccepted: {
            editText.focus = false
            movieAlbumModel.addAlbumFromName(editText.text)
        }
    }
    ListView {

        id: movieAlbumListPage
        anchors.topMargin: 80
        model: movieAlbumModel
        spacing: 5
        anchors.fill: parent
        delegate: Rectangle{
            width: parent.width - parent.width/13
            height: 120
            color: Style.buttonBackground

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    movieAlbumListPage.currentIndex = index
                    movieModel.setAlbumId(id)
                    pageStack.push("qrc:/MovieAlbumPage.qml",
                                   { movieAlbumName: name, albumRowIndex: index})

                }
            }

            Text{
                text: name
                font.pointSize: 16
                color: Style.text
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }


        }
    }
}
