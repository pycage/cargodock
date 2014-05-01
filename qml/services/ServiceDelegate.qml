import QtQuick 2.0
import Sailfish.Silica 1.0

/* Template for service delegates.
 */
Item {
    property string title
    property string subtitle
    property string iconSource

    property bool highlighted

    Image {
        id: iconImage
        width: 86
        height: 86
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingMedium
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.Pad
        source: iconSource !== "" ? iconSource : "image://theme/icon-m-folder"
    }

    Label {
        id: nameLabel
        anchors.left: iconImage.right
        anchors.leftMargin: Theme.paddingMedium
        color: highlighted ? Theme.highlightColor : Theme.primaryColor
        text: title
    }

    Label {
        anchors.left: nameLabel.left
        anchors.top: nameLabel.bottom
        font.pixelSize: Theme.fontSizeExtraSmall
        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        text: subtitle
    }
}
