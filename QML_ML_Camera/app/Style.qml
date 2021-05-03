pragma Singleton
import QtQuick 2.0

QtObject {
    property color text: "#fffafa" //"#3daee9"
    property color pictureText: "#1e90ff"

    property color windowBackground:  "#bd93f9"
    property color toolBackground:    "#C0BC87"
    property color pageBackground:    "#6272a4" //"#fcfcfc"
    property color buttonBackground:  "#1e90ff" //#50fa7b



    property color itemHighlight:     "#3daee9"

    property string fontName : "sans-serif";

    //property var specialPaths : [{ "label" : qsTr ("Root"), "uri" : "file:///" }];

    property int fontSize : 16;

    // round buttons
    property int roundButtonWidth:  65
    property int roundButtonHeight: 65
    property int roundButtonRadius: 32
    property color roundButtonRed: "#7F0000"
    property color roundButtonYellow: "#FFCA35"
    property color roundButtonGreen: "#95A751"
    property color roundButtonHome: "#216E9A"
}
