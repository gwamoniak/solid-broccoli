import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import "."

PageTheme {
    id: pageTheme

    toolbarTitle: "Logger"

    Component.onCompleted: {
        loggerModel.readLatestLog()
    }

    toolbarButtons: ColumnLayout {
        spacing: 5

        RoundButton {
            id: openLOG
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            antialiasing: true
            icon.source: "qrc:/images/png/open_log.png"
            icon.width: Style.roundButtonWidth - 12
            icon.height: Style.roundButtonHeight - 12
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonGreen
            }

            onClicked: {
                dialog.currentFolder = logsPath
                dialog.open()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        anchors.rightMargin: Style.roundButtonWidth + 24
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Style.buttonBackground

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 12

                Label { text: "Time"; color: Style.text; font.bold: true; Layout.preferredWidth: 180 }
                Label { text: "Type"; color: Style.text; font.bold: true; Layout.preferredWidth: 180 }
                Label { text: "Message"; color: Style.text; font.bold: true; Layout.fillWidth: true }
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
                color: index % 2 === 0 ? "#f5f5f5" : "#e7e7e7"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { text: time; color: "#202020"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 180 }
                    Text { text: type; color: "#202020"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.preferredWidth: 180 }
                    Text { text: message; color: "#202020"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter; Layout.fillWidth: true }
                }
            }
        }
    }

    FileDialog {
        id: dialog
        title: "Open log file"
        currentFolder: logsPath
        nameFilters: ["CSV files (*.csv)", "All files (*)"]

        onAccepted: {
            var loggerUrl = dialog.selectedFile.toString()
            loggerUrl = loggerUrl.replace(/^(file:\/{3})/, "")
            loggerModel.readCSV(loggerUrl)
            dialog.close()
        }
    }
}
