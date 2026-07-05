import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import "."

NavPage {
    pageTitle: qsTr("Logger")

    Component.onCompleted: {
        loggerModel.readLatestLog()
    }

    trailing: Component {
        ToolButton {
            icon.source: "qrc:/images/svg/log.svg"
            icon.color: Theme.accent
            icon.width: 20; icon.height: 20
            background: null
            onClicked: {
                dialog.currentFolder = logsPath
                dialog.open()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            radius: Theme.radiusControl
            color: Theme.fill

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 12
                Label { text: qsTr("Time");    color: Theme.label; font.weight: Font.Medium; Layout.preferredWidth: 180 }
                Label { text: qsTr("Type");    color: Theme.label; font.weight: Font.Medium; Layout.preferredWidth: 180 }
                Label { text: qsTr("Message"); color: Theme.label; font.weight: Font.Medium; Layout.fillWidth: true }
            }
        }

        ListView {
            id: logList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: loggerModel

            delegate: Rectangle {
                width: logList.width
                height: 34
                color: index % 2 === 0 ? Theme.surface : Theme.groupedBackground

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12
                    Text { text: time;    color: Theme.label; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 180 }
                    Text { text: type;    color: Theme.secondaryLabel; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 180 }
                    Text { text: message; color: Theme.label; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.fillWidth: true }
                }
            }
        }
    }

    FileDialog {
        id: dialog
        title: qsTr("Open log file")
        currentFolder: logsPath
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: {
            var loggerUrl = dialog.selectedFile.toString()
            loggerUrl = loggerUrl.replace(/^(file:\/{3})/, "")
            loggerModel.readCSV(loggerUrl)
        }
    }
}
