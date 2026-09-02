pragma Singleton
import QtQuick 2.0

// Back-compatibility shim: existing pages reference Style.xxx; these now
// delegate to Theme tokens so the look updates from one place. New code
// should reference Theme directly. This file can be deleted once every
// consumer is migrated.
QtObject {
    property color text:             Theme.label
    property color pictureText:      Theme.accent
    property color windowBackground: Theme.groupedBackground
    property color toolBackground:   Theme.surface
    property color pageBackground:   Theme.groupedBackground
    property color buttonBackground: Theme.accent
    property color itemHighlight:    Theme.accent
    property color iconColor:        Theme.label
    property color dangerColor:      Theme.destructive

    property int iconSize: 24
    property string fontName: Theme.fontName
    property int fontSize: Theme.subhead

    // Legacy round-button metrics — retained for pages not yet migrated.
    property int roundButtonWidth:  44
    property int roundButtonHeight: 44
    property int roundButtonRadius: 22
    property color roundButtonRed:    Theme.destructive
    property color roundButtonYellow: Theme.accent
    property color roundButtonGreen:  Theme.accent
    property color roundButtonHome:   Theme.accent
}
