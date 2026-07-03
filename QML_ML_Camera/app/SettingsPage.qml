import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import solid.broccoli 1.0
import "."

NavPage {
    pageTitle: qsTr("Settings")
    showLargeTitle: true

    MediaDevices { id: mediaDevices }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        contentHeight: settingsCol.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: settingsCol
            width: Math.min(parent.width, Theme.maxContentWidth)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            // ── CAMERA section ──
            Label {
                text: qsTr("CAMERA")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 50
                radius: Theme.radiusControl
                color: Theme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Label {
                        text: qsTr("Device")
                        font.pointSize: Theme.body
                        color: Theme.label
                    }
                    Item { Layout.fillWidth: true }
                    ComboBox {
                        id: cameraCombo
                        model: mediaDevices.videoInputs
                        textRole: "description"
                        implicitWidth: Math.min(280, settingsCol.width * 0.5)
                        flat: true
                    }
                }
            }

            // ── GENERAL section ──
            Label {
                text: qsTr("GENERAL")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radiusControl
                color: Theme.surface
                implicitHeight: generalCol.implicitHeight

                Column {
                    id: generalCol
                    width: parent.width

                    SettingSwitcher {
                        width: parent.width
                        text: qsTr("Shutter flash")
                        value: AppSettings.shutterFlash
                        onToggled: function(checked) { AppSettings.shutterFlash = checked }
                    }

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    SettingSwitcher {
                        width: parent.width
                        text: qsTr("Mirror preview")
                        value: AppSettings.mirrorPreview
                        onToggled: function(checked) { AppSettings.mirrorPreview = checked }
                    }
                }
            }

            // ── ABOUT section ──
            Label {
                text: qsTr("ABOUT")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 50
                radius: Theme.radiusControl
                color: Theme.surface

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (owningStack)
                            owningStack.push("qrc:/LoggerPage.qml", { owningStack: owningStack })
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Label {
                        text: qsTr("Open Logger")
                        font.pointSize: Theme.body
                        color: Theme.accent
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: ">"
                        font.pointSize: Theme.body
                        color: Theme.secondaryLabel
                    }
                }
            }

            Item { Layout.preferredHeight: 20 }
        }
    }
}
