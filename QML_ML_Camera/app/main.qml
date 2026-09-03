import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: qsTr("SolidBroccoli")
    color: Theme.groupedBackground

    // Legacy compat: some pages still reference window.pageStack / goHome().
    readonly property alias pageStack: photosStack
    function goHome() {
        tabBar.currentIndex = 0
        liveStack.pop(null)
    }

    StackLayout {
        id: tabContent
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: tabBarContainer.top
        currentIndex: tabBar.currentIndex

        StackView {
            id: liveStack
            initialItem: LivePage { owningStack: liveStack }
        }
        StackView {
            id: sessionsStack
            initialItem: SessionsPage { owningStack: sessionsStack }
        }
        StackView {
            id: cameraStack
            initialItem: CameraPage { owningStack: cameraStack }
        }
        StackView {
            id: photosStack
            initialItem: AlbumListPage { owningStack: photosStack }
        }
        StackView {
            id: videosStack
            initialItem: MovieAlbumPage { owningStack: videosStack }
        }
        StackView {
            id: settingsStack
            initialItem: SettingsPage { owningStack: settingsStack }
        }
    }

    Rectangle {
        id: tabBarContainer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Theme.tabBarHeight
        color: Theme.surface

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: Theme.hairline
            color: Theme.separator
        }

        TabBar {
            id: tabBar
            anchors.fill: parent
            anchors.topMargin: Theme.hairline
            background: null

            Repeater {
                model: [
                    { icon: "qrc:/images/svg/spectrum.svg",  label: qsTr("Live") },
                    { icon: "qrc:/images/svg/sessions.svg",  label: qsTr("Sessions") },
                    { icon: "qrc:/images/svg/camera.svg",    label: qsTr("Camera") },
                    { icon: "qrc:/images/svg/gallery.svg",   label: qsTr("Photos") },
                    { icon: "qrc:/images/svg/movie.svg",     label: qsTr("Videos") },
                    { icon: "qrc:/images/svg/settings.svg",  label: qsTr("Settings") }
                ]

                TabButton {
                    id: tabButton
                    width: tabBarContainer.width / 6
                    height: tabBarContainer.height - Theme.hairline
                    background: null
                    icon.source: modelData.icon
                    icon.width: 22
                    icon.height: 22
                    icon.color: tabButton.checked ? Theme.accent : Theme.secondaryLabel
                    text: modelData.label
                    display: AbstractButton.TextUnderIcon
                    spacing: 2
                    font.pointSize: Theme.caption
                    palette.windowText: tabButton.checked ? Theme.accent : Theme.secondaryLabel
                    palette.buttonText: tabButton.checked ? Theme.accent : Theme.secondaryLabel
                }
            }
        }
    }

    ToastOverlay { }
}
