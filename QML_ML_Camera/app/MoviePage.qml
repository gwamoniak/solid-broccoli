import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import "."

PageTheme {
    id: moviePage

    property string movieName
    property url movieUrl
    property int movieIndex

    toolbarTitle: movieName

    MediaPlayer {
        id: player
        source: movieUrl
        audioOutput: AudioOutput {
            id: audioOutput
            volume: volumeSlider.value
        }
        videoOutput: videoOutput
    }

    toolbarButtons: ColumnLayout {
        spacing: 5

        RoundButton {
            id: renameMovie
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/png/rename_photo.png"
            icon.width: Style.roundButtonWidth - 15
            icon.height: Style.roundButtonHeight - 15
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonYellow
            }
            onClicked: {
                renameMovieDialog.open()
            }
        }

        RoundButton {
            id: deleteMovie
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: Style.roundButtonHeight
            Layout.preferredWidth: Style.roundButtonWidth
            icon.source: "qrc:/images/png/delete_photo.png"
            icon.width: Style.roundButtonWidth - 15
            icon.height: Style.roundButtonHeight - 15
            background: Rectangle {
                radius: Style.roundButtonRadius
                color: Style.roundButtonRed
            }
            onClicked: {
                player.stop()
                movieModel.removeRows(movieIndex, 1)
                stackView.pop()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        VideoOutput {
            id: videoOutput
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: VideoOutput.PreserveAspectFit
        }

        Slider {
            id: progressSlider
            Layout.fillWidth: true
            from: 0
            to: player.duration
            value: player.position
            onMoved: player.position = value
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            RoundButton {
                text: "⏪"
                font.pointSize: 16
                onClicked: player.position = Math.max(0, player.position - 10000)
            }

            RoundButton {
                id: playPauseButton
                text: player.playbackState === MediaPlayer.PlayingState ? "⏸" : "▶"
                font.pointSize: 16
                onClicked: {
                    if (player.playbackState === MediaPlayer.PlayingState) {
                        player.pause()
                    } else {
                        player.play()
                    }
                }
            }

            RoundButton {
                text: "⏹"
                font.pointSize: 16
                onClicked: player.stop()
            }

            RoundButton {
                text: "⏩"
                font.pointSize: 16
                onClicked: player.position = Math.min(player.duration, player.position + 10000)
            }

            Label {
                text: {
                    function fmt(ms) {
                        var s = Math.floor(ms / 1000)
                        return Math.floor(s / 60) + ":" + (s % 60 < 10 ? "0" : "") + (s % 60)
                    }
                    return fmt(player.position) + " / " + fmt(player.duration)
                }
                color: Style.text
                font.pointSize: 14
            }

            Slider {
                id: volumeSlider
                Layout.preferredWidth: 100
                from: 0
                to: 1
                value: 0.5
            }
        }
    }

    InputDialog {
        id: renameMovieDialog
        title: qsTr("Rename recording")
        label: qsTr("Recording name:")
        hint: movieName

        onAccepted: {
            editText.focus = false
            movieModel.rename(movieIndex, editText.text)
            movieName = editText.text
        }
    }
}
