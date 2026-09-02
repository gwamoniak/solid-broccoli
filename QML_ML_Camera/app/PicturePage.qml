import QtQuick
import solid.broccoli 1.0
import QtQuick.Controls
import "."

NavPage {
    id: picturePage
    property string pictureName
    property int pictureIndex

    pageTitle: pictureName

    trailing: Component {
        Row {
            spacing: 4
            ToolButton {
                icon.source: "qrc:/images/svg/rename.svg"
                icon.color: Theme.accent
                icon.width: 20; icon.height: 20
                background: null
                onClicked: renamePhotoDialog.open()
            }
            ToolButton {
                icon.source: "qrc:/images/svg/delete.svg"
                icon.color: Theme.destructive
                icon.width: 20; icon.height: 20
                background: null
                onClicked: {
                    AppContext.pictureModel.removeRows(pictureIndex, 1)
                    if (owningStack) owningStack.pop()
                }
            }
        }
    }

    ListView {
        id: pictureListView
        model: AppContext.pictureModel
        anchors.fill: parent
        spacing: 0
        orientation: Qt.Horizontal
        snapMode: ListView.SnapOneItem
        currentIndex: pictureIndex

        Component.onCompleted: {
            positionViewAtIndex(currentIndex, ListView.SnapPosition)
        }

        onMovementEnded: {
            const visibleIndex = indexAt(contentX + width / 2, contentY + height / 2)
            if (visibleIndex >= 0)
                currentIndex = visibleIndex
        }

        delegate: Rectangle {
            property int itemIndex: index
            property string itemName: name
            ListView.onIsCurrentItemChanged: {
                if (ListView.isCurrentItem)
                    picturePage.pictureName = itemName
            }

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
        id: renamePhotoDialog
        title: qsTr("Rename photo")
        label: qsTr("Photo name:")
        hint: pictureName
        onAccepted: {
            editText.focus = false
            AppContext.pictureModel.rename(pictureIndex, editText.text)
            pictureName = editText.text
        }
    }
}
