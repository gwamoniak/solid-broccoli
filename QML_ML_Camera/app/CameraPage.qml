import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtMultimedia 5.8
import Qt.labs.settings 1.0
import solid.broccoli 1.0
import "."

PageTheme {

    property string albumName
    property int albumRowIndex

    CameraProcessor{
        id: cameraProcessor
    }


    ColumnLayout
    {
        anchors.fill: parent
        GroupBox
        {
            id: videoGroupbox
            title: qsTr("Video Preview")
            label: Label{
                color: Style.pictureText
                text: videoGroupbox.title
                font.pointSize: 18
                anchors.horizontalCenter: parent.horizontalCenter

            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.topMargin: 10

            VideoOutput
            {
                id: videoOutput
                anchors.fill: parent
                Camera
                {
                    id: camera

                    Component.onCompleted:
                    {
                        cameraProcessor.setCamera(camera)
                    }

                    videoRecorder.audioBitRate: 48000
                    videoRecorder.mediaContainer: "mp4"
                    videoRecorder.frameRate: 25
                    videoRecorder.outputLocation: "\app\recodings"

                }

                source: camera
                fillMode: Qt.KeepAspectRatio
                autoOrientation: true


            }
        }
    }
    toolbarButtons: ColumnLayout {

    RoundButton
    {
        id: savePicture

        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        Layout.preferredHeight:  65
        Layout.preferredWidth:   65
        background: Image {
            source: "qrc:/images/png/save_photo.png"
            width: 65
            height: 65
            smooth: true
        }


        onClicked: {
            cameraProcessor.captureImage()
//            var url = cameraProcessor.getURL()
//            console.log("albumm url", url)
        }

    }
    RoundButton
    {
        id: recordMovie
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        Layout.preferredHeight:  65
        Layout.preferredWidth:   65
        background: Image {
            source: "qrc:/images/png/record.png"
            width: 65
            height: 65
            smooth: true
        }


        onClicked: {
            console.log("Record")
            //cameraProcessor.recordMovie()
        }

    }
    RoundButton
    {
        id: stopRecording
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        Layout.preferredHeight:  65
        Layout.preferredWidth:   65
        background: Image {
            source: "qrc:/images/png/stop_recording.png"
            width: 65
            height: 65
            smooth: true
        }


        onClicked: {
            console.log("stop recording")
            cameraProcessor.stopRecording()
        }

    }
    }
}
