import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property alias key: lblKey.text
    property alias value: lblValue.text

    width: parent.width
    height: Theme.itemSizeSmall * 0.66

    Label {
        id: lblKey
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        color: Theme.primaryColor
    }

    Label {
        id: lblValue
        anchors.left: lblKey.right
        anchors.right: parent.right
        anchors.baseline: lblKey.baseline
        anchors.leftMargin: Theme.paddingLarge
        anchors.rightMargin: Theme.paddingLarge
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeSmall
        truncationMode: TruncationMode.Fade
        color: Theme.secondaryColor
    }

}
