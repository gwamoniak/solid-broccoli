import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.2
import "." // QTBUG-34418, singletons require explicit import to load qmldir file

ApplicationWindow {

    readonly property alias pageStack: stackView

    function goHome() {
        console.log("Navigation: goHome requested. Current depth:", stackView.depth)
        stackView.clear(StackView.Immediate)
        stackView.push("qrc:/MainPage.qml", {}, StackView.Immediate)
    }

    id: window
    visible: true
    width: 1280
    height: 800
    title: qsTr("UIX")
    color: Style.windowBackground


    StackView {
        id: stackView
        anchors.fill: parent
        clip: true
        initialItem: MainPage {}
        pushEnter: null
        pushExit: null
        popEnter: null
        popExit: null
        replaceEnter: null
        replaceExit: null

        onCurrentItemChanged: {
            console.log("Navigation: current page changed. Depth:", depth, "item:", currentItem)
        }
    }


}
