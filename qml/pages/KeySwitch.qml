import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    property alias key: lblKey.text
    property alias checked: switchValue.checked

    width: parent.width
    height: Theme.itemSizeSmall * 0.66

    Label {
        id: lblKey
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
    }

    Switch {
        id: switchValue
        anchors.right: parent.right
        anchors.rightMargin: Theme.paddingLarge
        anchors.verticalCenter: lblKey.verticalCenter
        enabled: parent.enabled
    }

    onClicked: {
        switchValue.checked = ! switchValue.checked;
    }
}
