import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtMultimedia 5.0
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4
import "."

PageTheme {


    property string movieName
    property int movieIndex

    MediaPlayer {
        id: player
        autoPlay: true
        source:  ""
        /* playbackState:
          0 - stopped
          1 - playing
          2 - paused
        */
        onPositionChanged: {
            progressbarSlider.value = player.position/player.duration
            //  Converting miliseconds to minutes and seconds.
            var currentTime = player.position
            var currentMin = Math.floor(currentTime/1000/60)
            var currentSec = (currentTime/1000) % 60
            currentSec = currentSec.toFixed(0)

            var totalTime = player.duration
            var totalMin = Math.floor(totalTime/1000/60)
            var totalSec = (totalTime/1000) % 60
            totalSec = totalSec.toFixed(0)

            progressText.text = currentMin + ':'+ currentSec + ' / ' + totalMin + ':' + totalSec

            if(player.position === 0) {
                progressText.text = '0:0 / 0:0'
            }
        }
        onPlaybackStateChanged: {
            if(player.playbackState === 0 || player.playbackState === 2) {
                playImage.source = "qrc:/images/png/play_player.png"
            }
            else
            {
                playImage.source = "qrc:/images/png/pause_player.png"
            }
        }
    }


    toolbarTitle: movieName
    toolbarButtons: ColumnLayout {
        RoundButton{
            id: renameAlbum
//            text: qsTr("RENAME")
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            antialiasing: true
            background: Image {
                source: "qrc:/images/png/rename_movie.png"
                width: 65
                height: 65
                smooth: true
            }
            onClicked: {
                renameMovieDialog.open()
            }
        }
        RoundButton {
            //text: qsTr("DELETE")
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight:  65
            Layout.preferredWidth:   65
            antialiasing: true
            background: Image {
                source: "qrc:/images/png/delete_movie.png"
                width: 65
                height: 65
                smooth: true
            }
            onClicked: {
                movieModel.removeRows(movieIndex, 1)
                stackView.pop()
            }
        }

    }

   Item{

        id: movieListView
        width: parent
        height: parent
        anchors.fill: parent



 Rectangle {

        property int itemIndex: index
        property string itemName: name

        id: showList
        width: movieListView.width
        height: movieListView.height
        color: "transparent"


        Image {
            id: thumbnailImage
            fillMode: Image.PreserveAspectFit
            cache: false
            width: parent.width
            height: parent.height
            source: "image://movies/" + movieIndex + "/full"
        }

        VideoOutput {
            id: videoOutput
            source: player
            anchors.fill: parent
            // fillmode - the video is scaled uniformly to fill, cropping if necessary
            fillMode: VideoOutput.PreserveAspectFit
        }
    }    
}

    Slider {
        id: progressbarSlider
        y: 652
        width: 250
        anchors.horizontalCenterOffset: -50
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50

        onPressedChanged:  player.seek(progressbarSlider.value * player.duration)
    }
    Text {
        id: progressText
        x: 681
        y: 654
        color: "red"
        text: qsTr("0:0 / 0:0")
        anchors.bottom: parent.bottom
        anchors.rightMargin: -95
        anchors.right: progressbarSlider.right

        anchors.bottomMargin: 52
        font.bold: true
        font.pixelSize: 12
    }
    Rectangle {
        id: controlArea
        x: 400
        y: 650
        width: 350
        height: 50
        opacity: 0.6
        color: "white"
        radius: 2
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        RowLayout {
            id: rowPanel
            width: 250
            height: 50
            anchors.centerIn: parent
            RoundButton {
                //text: "Backward"
                id:backward
                Layout.preferredHeight:  50
                Layout.preferredWidth:   50
                display: AbstractButton.IconOnly
                  Layout.alignment: Qt.AlignLeft
                background: Image {
                    source: "qrc:/images/png/backward_player.png"
                    width: 50
                    height: 50
                    smooth: true
                }
                onClicked: {
                    player.seek(player.position - 10000)
                }
            }
            RoundButton {
                id: playPauseButton
                Layout.preferredHeight:  50
                Layout.preferredWidth:   50
                //text: "Play"
                Layout.alignment: Qt.AlignLeft
                background: Image {
                    id:playImage
                    source: "qrc:/images/png/play_player.png"
                    width: 50
                    height: 50
                    smooth: true
                }
                onClicked: {
                    if(player.playbackState === 0 || player.playbackState === 2) {
                        var url = movieProcessor.getUrlFromIndex(movieIndex)
                        player.source = url
                        player.play()
                        thumbnailImage.opacity = 0.0
                    }
                    else  {
                        player.pause()
                    }
                }
            }
            RoundButton {
                id: forward
                Layout.preferredHeight:  50
                Layout.preferredWidth:   50
                Layout.alignment: Qt.AlignLeft
                //text: "Forward"
                background: Image {
                    source: "qrc:/images/png/forward_player.png"
                    width: 50
                    height: 50
                    smooth: true
                }
                onClicked: {
                    player.seek(player.position + 10000)
                }
            }
            RoundButton {
                id: stop
                //text: "Stop"
                Layout.preferredHeight:  50
                Layout.preferredWidth:   50
                Layout.alignment: Qt.AlignLeft
                background: Image {
                    source: "qrc:/images/png/stop_player.png"
                    width: 50
                    height: 50
                    smooth: true
                }
                onClicked: {
                    player.stop()
                    thumbnailImage.opacity = 1.0
                }
            }
            Slider {
                id: audioSlider
                Layout.preferredWidth:   70
                value: 0.5
                onValueChanged: player.volume = audioSlider.value
            }
        }
}
    InputDialog {
        property string hintName : movieName
        id: renameMovieDialog
        title: "Rename a photo"
        label: "Photo name:"
        hint: hintName

        onAccepted: {
            editText.focus = false
            movieModel.rename(movieIndex, editText.text)
            movieName = editText.text
        }
    }
}
