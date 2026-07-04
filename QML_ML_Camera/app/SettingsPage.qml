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

            // ── SPECTROMETER section ──
            Label {
                text: qsTr("SPECTROMETER")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radiusControl
                color: Theme.surface
                implicitHeight: spectroCol.implicitHeight

                Column {
                    id: spectroCol
                    width: parent.width

                    // Device picker: tap cycles through the available devices.
                    Item {
                        width: parent.width
                        height: 50

                        MouseArea {
                            anchors.fill: parent
                            onClicked: SpectrometerService.selectNextDevice()
                        }

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
                            Label {
                                text: SpectrometerService.availableDevices.length > 0
                                      ? SpectrometerService.availableDevices[SpectrometerService.currentDeviceIndex]
                                      : qsTr("None")
                                font.pointSize: Theme.subhead
                                color: Theme.secondaryLabel
                                elide: Text.ElideRight
                                Layout.maximumWidth: parent.width * 0.6
                            }
                            Label {
                                text: ">"
                                font.pointSize: Theme.body
                                color: Theme.secondaryLabel
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Connect / Disconnect
                    Item {
                        width: parent.width
                        height: 50

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Connection")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                id: connectButton
                                text: SpectrometerService.connected ? qsTr("Disconnect") : qsTr("Connect")
                                onClicked: SpectrometerService.connected
                                           ? SpectrometerService.disconnectDevice()
                                           : SpectrometerService.connectDevice()
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: SpectrometerService.connected
                                           ? Theme.fill
                                           : (connectButton.down ? Theme.accentPressed : Theme.accent)
                                }
                                contentItem: Text {
                                    text: connectButton.text
                                    font.pointSize: Theme.subhead
                                    color: SpectrometerService.connected ? Theme.label : Theme.textOverAccent
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 12
                                    rightPadding: 12
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Integration time
                    Item {
                        width: parent.width
                        height: 58

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Label {
                                text: qsTr("Integration")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 5
                                to: 1000
                                stepSize: 5
                                value: SpectrometerService.integrationTimeMs
                                onMoved: SpectrometerService.integrationTimeMs = Math.round(value)
                            }
                            Label {
                                text: SpectrometerService.integrationTimeMs + " ms"
                                font.family: Theme.readoutFontName
                                font.pointSize: Theme.subhead
                                color: Theme.label
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: 64
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Averaging
                    Item {
                        width: parent.width
                        height: 50

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Averaging")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Item { Layout.fillWidth: true }
                            Row {
                                spacing: 4
                                Repeater {
                                    model: [1, 2, 4, 8]
                                    Button {
                                        id: avgButton
                                        required property int modelData
                                        width: 40
                                        height: 32
                                        onClicked: SpectrometerService.averaging = avgButton.modelData
                                        background: Rectangle {
                                            radius: Theme.radiusControl
                                            color: SpectrometerService.averaging === avgButton.modelData
                                                   ? Theme.accent : Theme.fill
                                        }
                                        contentItem: Text {
                                            text: avgButton.modelData
                                            font.pointSize: Theme.subhead
                                            font.family: Theme.readoutFontName
                                            color: SpectrometerService.averaging === avgButton.modelData
                                                   ? Theme.textOverAccent : Theme.label
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

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

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    SettingSwitcher {
                        width: parent.width
                        text: qsTr("Spectrum overlay on video")
                        value: AppSettings.spectrumOverlay
                        onToggled: function(checked) { AppSettings.spectrumOverlay = checked }
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
