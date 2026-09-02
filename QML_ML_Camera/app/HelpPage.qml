import QtQuick
import QtQuick.Controls
import solid.broccoli 1.0
import "."

NavPage {
    pageTitle: qsTr("Help")

    ScrollView {
        anchors.fill: parent
        anchors.margins: Theme.screenMargin
        clip: true

        TextArea {
            text: HelpContent.markdown
            textFormat: TextEdit.MarkdownText
            readOnly: true
            wrapMode: TextEdit.Wrap
            color: Theme.label
            selectionColor: Theme.accent
            selectedTextColor: Theme.textOverAccent
            font.family: Theme.fontName
            font.pointSize: Theme.body
            background: null
        }
    }
}
