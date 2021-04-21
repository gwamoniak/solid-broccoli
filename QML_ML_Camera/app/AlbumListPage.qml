import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import "."


PageTheme {
    toolbarTitle: "Picture Albums"
    toolbarButtons: ToolButton {
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        //text:qsTr("ADD")
        //font.pointSize: 18
        Layout.preferredHeight:  65
        Layout.preferredWidth:   65
        antialiasing: true
        background: Image {
            source: "qrc:/images/png/add_album_gallery.png"
            width: 65
            height: 65
            smooth: true
        }
        onClicked: {
            newAlbumDialog.open()
        }

    }

    InputDialog {
        id: newAlbumDialog
        title: "New pictures album"
        label: "Pictures Album name:"
        hint: "My Pictures Album"

        onAccepted: {
            editText.focus = false
            albumModel.addAlbumFromName(editText.text)
        }
    }
    ListView {

        id: albumListPage
        anchors.topMargin: 80
        model: albumModel
        spacing: 5
        anchors.fill: parent
        delegate: Rectangle{
            width: parent.width - parent.width/13
            height: 120
            color: Style.buttonBackground

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    albumListPage.currentIndex = index
                    pictureModel.setAlbumId(id)
                    pageStack.push("qrc:/AlbumPage.qml",
                                   { albumName: name, albumRowIndex: index})

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

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
