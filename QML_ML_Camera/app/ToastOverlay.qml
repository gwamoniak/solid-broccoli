import QtQuick
import QtQuick.Controls
import solid.broccoli 1.0
import "."

Item {
    id: root
    anchors.fill: parent
    visible: toast.opacity > 0
    z: 1000

    Rectangle {
        id: toast
        anchors.top: parent.top
        anchors.topMargin: Theme.screenMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width - 2 * Theme.screenMargin,
                        messageLabel.implicitWidth + 32)
        implicitHeight: messageLabel.implicitHeight + 24
        radius: Theme.radiusControl
        color: AppNotifier.level === AppNotifier.Error
               ? Theme.destructive : Theme.surfaceElevated
        border.width: AppNotifier.level === AppNotifier.Warning ? Theme.hairline : 0
        border.color: Theme.accent
        opacity: 0

        Label {
            id: messageLabel
            anchors.fill: parent
            anchors.margins: 12
            text: AppNotifier.message
            color: AppNotifier.level === AppNotifier.Warning
                   ? Theme.accent : Theme.label
            font.pointSize: Theme.subhead
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Behavior on opacity { NumberAnimation { duration: 140 } }
    }

    Connections {
        target: AppNotifier
        function onNotificationChanged() {
            toast.opacity = 1
            hideTimer.restart()
        }
    }

    Timer {
        id: hideTimer
        interval: 4000
        onTriggered: toast.opacity = 0
    }
}
