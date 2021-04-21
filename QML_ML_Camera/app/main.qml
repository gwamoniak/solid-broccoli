import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtQuick.VirtualKeyboard 2.2
import "." // QTBUG-34418, singletons require explicit import to load qmldir file

ApplicationWindow {

    readonly property alias pageStack: stackView

    id: window
    visible: true
    width: 1280
    height: 800
    title: qsTr("UIX")
    color: Style.windowBackground


    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: MainPage {}
    }


}
