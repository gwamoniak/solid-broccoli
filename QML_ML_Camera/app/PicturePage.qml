import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import "."

PageTheme {


    property string pictureName
    property int pictureIndex

    toolbarTitle: pictureName
    toolbarButtons: ColumnLayout {
        RoundButton{
            id: renameAlbum
//            text: qsTr("RENAME")
//            font.pointSize: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            antialiasing: true
            background: Image {
                source: "qrc:/images/png/rename_photo.png"
                width: 65
                height: 65
                smooth: true
            }
            onClicked: {
                renamePhotoDialog.open()
            }
        }
        RoundButton {
            //text: qsTr("DELETE")
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            antialiasing: true
            background: Image {
                source: "qrc:/images/png/delete_photo.png"
                width: 65
                height: 65
                smooth: true
            }
            onClicked: {
                pictureModel.removeRows(pictureIndex, 1)
                stackView.pop()
            }
        }

    }

    ListView{

        id: pictureListView
        model: pictureModel
        anchors.fill: parent
        spacing: 5
        orientation: Qt.Horizontal
        snapMode: ListView.SnapOneItem
        currentIndex: pictureIndex


    Component.onCompleted: {
        positionViewAtIndex(currentIndex,ListView.SnapPosition)


    }
    onMovementEnded: {
        currentIndex = itemAt(contentX, contentY).itemIndex
    }

    onCurrentItemChanged: {
        toolbarTitle = currentItem.itemName
        pictureName  = currentItem.itemName
    }


    delegate: Rectangle {
        property int itemIndex: index
        property string itemName: name

        // delegate is instanciated first so listview has no dimension,
        // causing positionViewAtIndex to fail...
        width: ListView.view.width === 0 ? parent.width : ListView.view.width
        height: pictureListView.height
        color: "transparent"

        Image {
            fillMode: Image.PreserveAspectFit
            cache: false
            width: parent.width
            height: parent.height
            source: "image://pictures/" + index + "/full"

        }
    }

}

    InputDialog {

        property string hintName : pictureName

        id: renamePhotoDialog
        title: "Rename a photo"
        label: "Photo name:"
        hint: hintName

        onAccepted: {
            editText.focus = false
            pictureModel.rename(pictureIndex, editText.text)
            pictureName = editText.text
        }
    }
}
