import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtMultimedia
import solid.broccoli 1.0
import "."

NavPage {
    pageTitle: qsTr("Settings")
    showLargeTitle: true

    MediaDevices { id: mediaDevices }

    FileDialog {
        id: modelFileDialog
        title: qsTr("Import detection model")
        nameFilters: [qsTr("ONNX models (*.onnx)")]
        onAccepted: AppSettings.importDetectionModel(selectedFile)
    }

    FileDialog {
        id: ggufFileDialog
        title: qsTr("Choose AI model")
        nameFilters: [qsTr("GGUF models (*.gguf)")]
        onAccepted: AppSettings.setAiModelFromUrl(selectedFile)
    }

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

                    // BLE bridge scan: appends discovered devices to the picker.
                    Item {
                        width: parent.width
                        height: 50

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Bluetooth bridges")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                id: refreshButton
                                text: qsTr("Refresh")
                                onClicked: SpectrometerService.refreshDevices()
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: refreshButton.down ? Theme.accentPressed : Theme.fill
                                }
                                contentItem: Text {
                                    text: refreshButton.text
                                    font.pointSize: Theme.subhead
                                    color: Theme.label
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

            // ── GEIGER section ──
            Label {
                text: qsTr("GEIGER")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radiusControl
                color: Theme.surface
                implicitHeight: geigerCol.implicitHeight

                Column {
                    id: geigerCol
                    width: parent.width

                    // Device picker: tap cycles through the available devices.
                    Item {
                        width: parent.width
                        height: 50

                        MouseArea {
                            anchors.fill: parent
                            onClicked: GeigerService.selectNextDevice()
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
                                text: GeigerService.availableDevices.length > 0
                                      ? GeigerService.availableDevices[GeigerService.currentDeviceIndex]
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
                                id: geigerConnectButton
                                text: GeigerService.connected ? qsTr("Disconnect") : qsTr("Connect")
                                onClicked: GeigerService.connected
                                           ? GeigerService.disconnectDevice()
                                           : GeigerService.connectDevice()
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: GeigerService.connected
                                           ? Theme.fill
                                           : (geigerConnectButton.down ? Theme.accentPressed : Theme.accent)
                                }
                                contentItem: Text {
                                    text: geigerConnectButton.text
                                    font.pointSize: Theme.subhead
                                    color: GeigerService.connected ? Theme.label : Theme.textOverAccent
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

                    // Tube conversion factor (µSv/h per CPM)
                    Item {
                        width: parent.width
                        height: 58

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Label {
                                text: qsTr("Tube factor")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.001
                                to: 0.02
                                stepSize: 0.0001
                                value: GeigerService.tubeFactor
                                onMoved: GeigerService.tubeFactor = value
                            }
                            Label {
                                text: GeigerService.tubeFactor.toFixed(4)
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

                    // Alert threshold (µSv/h)
                    Item {
                        width: parent.width
                        height: 58

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Label {
                                text: qsTr("Alert level")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.1
                                to: 5.0
                                stepSize: 0.05
                                value: GeigerService.alertThreshold
                                onMoved: GeigerService.alertThreshold = value
                            }
                            Label {
                                text: GeigerService.alertThreshold.toFixed(2) + " µSv/h"
                                font.family: Theme.readoutFontName
                                font.pointSize: Theme.subhead
                                color: Theme.label
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: 88
                            }
                        }
                    }
                }
            }

            // ── VISION section (object detection; needs the ONNX plugin) ──
            Label {
                text: qsTr("VISION")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radiusControl
                color: Theme.surface
                implicitHeight: visionCol.implicitHeight

                Column {
                    id: visionCol
                    width: parent.width

                    // Unavailable note (plugin not built / ONNX Runtime absent)
                    Item {
                        width: parent.width
                        height: 50
                        visible: !visionAvailable

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Detection plugin not available — install ONNX Runtime and rebuild")
                                font.pointSize: Theme.subhead
                                color: Theme.tertiaryLabel
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }
                    }

                    SettingSwitcher {
                        width: parent.width
                        visible: visionAvailable
                        text: qsTr("Object detection")
                        value: AppSettings.objectDetection
                        onSwitched: function(checked) { AppSettings.objectDetection = checked }
                    }

                    Rectangle {
                        width: parent.width - 28
                        height: Theme.hairline
                        color: Theme.separator
                        visible: visionAvailable
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Model row: shows the imported model, opens the models folder.
                    Item {
                        width: parent.width
                        height: 50
                        visible: visionAvailable

                        MouseArea {
                            anchors.fill: parent
                            onClicked: AppSettings.revealModelsDir()
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Model")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: AppSettings.detectionModelName.length > 0
                                      ? AppSettings.detectionModelName
                                      : qsTr("None — import a YOLO .onnx")
                                font.pointSize: Theme.subhead
                                color: AppSettings.detectionModelName.length > 0
                                       ? Theme.secondaryLabel : Theme.tertiaryLabel
                                elide: Text.ElideMiddle
                                Layout.maximumWidth: parent.width * 0.5
                            }
                            Button {
                                id: importModelButton
                                text: qsTr("Import…")
                                onClicked: modelFileDialog.open()
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: importModelButton.down ? Theme.accentPressed : Theme.fill
                                }
                                contentItem: Text {
                                    text: importModelButton.text
                                    font.pointSize: Theme.subhead
                                    color: Theme.label
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
                        visible: visionAvailable
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Detection stride: run the model every Nth preview frame.
                    Item {
                        width: parent.width
                        height: 58
                        visible: visionAvailable

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Label {
                                text: qsTr("Detect every")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 1
                                to: 10
                                stepSize: 1
                                value: AppSettings.detectionStride
                                onMoved: AppSettings.detectionStride = Math.round(value)
                            }
                            Label {
                                text: AppSettings.detectionStride
                                      + (AppSettings.detectionStride === 1
                                         ? qsTr(" frame") : qsTr(" frames"))
                                font.family: Theme.readoutFontName
                                font.pointSize: Theme.subhead
                                color: Theme.label
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: 88
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
                        onSwitched: function(checked) { AppSettings.shutterFlash = checked }
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
                        onSwitched: function(checked) { AppSettings.mirrorPreview = checked }
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
                        onSwitched: function(checked) { AppSettings.spectrumOverlay = checked }
                    }
                }
            }

            // ── AI section (report generator; needs the ai-core build) ──
            Label {
                text: qsTr("AI")
                font.pointSize: Theme.caption
                color: Theme.secondaryLabel
                Layout.leftMargin: 12
                visible: aiAvailable
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radiusControl
                color: Theme.surface
                implicitHeight: aiCol.implicitHeight
                visible: aiAvailable

                Column {
                    id: aiCol
                    width: parent.width

                    // Model row: the GGUF is referenced in place (gigabytes).
                    Item {
                        width: parent.width
                        height: 50

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14

                            Label {
                                text: qsTr("Model")
                                font.pointSize: Theme.body
                                color: Theme.label
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: AppSettings.aiModelName.length > 0
                                      ? AppSettings.aiModelName
                                      : qsTr("None — choose a GGUF")
                                font.pointSize: Theme.subhead
                                color: AppSettings.aiModelName.length > 0
                                       ? Theme.secondaryLabel : Theme.tertiaryLabel
                                elide: Text.ElideMiddle
                                Layout.maximumWidth: parent.width * 0.5
                            }
                            Button {
                                id: chooseGgufButton
                                text: qsTr("Choose…")
                                onClicked: ggufFileDialog.open()
                                background: Rectangle {
                                    radius: Theme.radiusControl
                                    color: chooseGgufButton.down ? Theme.accentPressed : Theme.fill
                                }
                                contentItem: Text {
                                    text: chooseGgufButton.text
                                    font.pointSize: Theme.subhead
                                    color: Theme.label
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

                    // Guidance footnote
                    Item {
                        width: parent.width
                        height: 54

                        Label {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            text: qsTr("Recommended: Gemma 3 4B instruct QAT Q4_0 (~3 GB RAM); "
                                       + "Gemma 3 1B on smaller devices. Model use is subject "
                                       + "to the Gemma Terms of Use.")
                            font.pointSize: Theme.caption
                            color: Theme.tertiaryLabel
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                        }
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
