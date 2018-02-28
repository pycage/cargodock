import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property bool peek
    property alias placeholderText: entry.placeholderText
    property alias label: entry.label
    property alias text: entry.text

    height: childrenRect.height

    TextField {
        id: entry

        anchors.left: parent.left
        anchors.right: peekLabel.left
        anchors.rightMargin: Theme.paddingSmall

        inputMethodHints: Qt.ImhNoPredictiveText
        echoMode: peek ? TextInput.Normal : TextInput.Password
        placeholderText: qsTr("Enter password")
        label: qsTr("Password")
    }

    Label {
        id: peekLabel
        anchors.right: parent.right
        anchors.rightMargin: Theme.paddingLarge
        text: "Abc"
        font.pixelSize: Theme.fontSizeMedium
        font.bold: true
        color: peek ? Theme.highlightColor : Theme.secondaryColor

        MouseArea {
            anchors.fill: parent

            onClicked: {
                peek = ! peek
            }
        }
    }
}

