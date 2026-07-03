import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import "."

NavPage {
    id: moviePage

    property string movieName
    property url movieUrl
    property int movieIndex

    pageTitle: movieName

    trailing: Component {
        Row {
            spacing: 4
            ToolButton {
                icon.source: "qrc:/images/svg/rename.svg"
                icon.color: Theme.accent
                icon.width: 20; icon.height: 20
                background: null
                onClicked: renameMovieDialog.open()
            }
            ToolButton {
                icon.source: "qrc:/images/svg/delete.svg"
                icon.color: Theme.destructive
                icon.width: 20; icon.height: 20
                background: null
                onClicked: {
                    player.stop()
                    movieModel.removeRows(movieIndex, 1)
                    if (owningStack) owningStack.pop()
                }
            }
        }
    }

    MediaPlayer {
        id: player
        source: movieUrl
        audioOutput: AudioOutput { volume: volumeSlider.value }
        videoOutput: videoOutput
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
            spacing: 16

            ToolButton {
                text: "⏪"; font.pointSize: 16
                background: null
                onClicked: player.position = Math.max(0, player.position - 10000)
            }
            ToolButton {
                text: player.playbackState === MediaPlayer.PlayingState ? "⏸" : "▶"
                font.pointSize: 16; background: null
                onClicked: {
                    if (player.playbackState === MediaPlayer.PlayingState)
                        player.pause()
                    else
                        player.play()
                }
            }
            ToolButton {
                text: "⏹"; font.pointSize: 16; background: null
                onClicked: player.stop()
            }
            ToolButton {
                text: "⏩"; font.pointSize: 16; background: null
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
                color: Theme.label
                font.pointSize: Theme.footnote
            }

            Slider {
                id: volumeSlider
                Layout.preferredWidth: 100
                from: 0; to: 1; value: 0.5
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
