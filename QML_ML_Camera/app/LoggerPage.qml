import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.2
import "."

PageTheme {
    id: pageTheme

    property url logsPath :  appPath + "/logs"

    toolbarTitle: "Logger"
    toolbarButtons: ColumnLayout {
        RoundButton{
            id: openLOG
            //text: qsTr("open LOG")
            //font.pointSize: 12
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            //antialiasing: true
            background: Image {
                source: "qrc:/images/png/open_log.png"
                width:  65
                height: 65
            }

            onClicked: {
                dialog.folder =  logsPath
                console.log("app logs path: ",logsPath )
                dialog.open()
            }
        }}


    TableView {
        id: tableView
        anchors.topMargin: 35
        anchors.rightMargin: 105
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 15

        TableViewColumn {
            role: "time"
            title: "Time"
             width: 160
        }
        TableViewColumn {
            role: "type"
            title: "Type"
             width: 150
        }
        TableViewColumn {
            role: "message"
            title: "Message"
            width: 800
        }

        model: loggerModel


        // Setting lines in TableView to intercept mouse left click
        rowDelegate: Rectangle {
            anchors.fill: parent
            color: styleData.selected ? 'skyblue' : (styleData.alternate ? 'whitesmoke' : 'white');
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton | Qt.LeftButton
                onClicked: {
                    tableView.selection.clear()
                    tableView.selection.select(styleData.row)
                    tableView.currentRow = styleData.row
                    tableView.focus = true

                    switch(mouse.button) {
                    case Qt.RightButton:
                        contextMenu.popup() // Call the context menu
                        break
                    default:
                        break
                    }
                }
            }
        }
    }

    // The context menu offers deleting a row from the database
    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Remove")
            onTriggered: {
                /* Call the dialog box that will clarify the intention to remove the row from the database
                 * */
                dialogDelete.open()
            }
        }
    }

    // Dialog of confirmation the removal line from the database
    MessageDialog {
        id: dialogDelete
        title: qsTr("Remove record")
        text: qsTr("Confirm the deletion of log entries")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        // If the answer ...
        onAccepted: {
            /* ... remove the line by id, which is taken from the data model
             * on the line number in the presentation
             * */
            //7database.removeRecord(myModel.getId(tableView.currentRow))
            //myModel.updateModel();
        }
    }

    FileDialog {
        id: dialog
        title: "Open file"
        folder: ""
        onAccepted: {
            var loggerUrl = dialog.fileUrl.toString()
            loggerUrl =loggerUrl.replace(/^(file:\/{3})/,"") // cut out file:\\
            loggerModel.readCSV(loggerUrl)
            dialog.close()
        }
    }
}





/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:3;anchors_height:421;anchors_y:54}
}
 ##^##*/
