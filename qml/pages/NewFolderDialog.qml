import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    allowedOrientations: Orientation.Landscape | Orientation.Portrait

    property alias name: textEntry.text

    canAccept: textEntry.text !== ""

    SilicaFlickable {
        anchors.fill: parent

        Column {
            width: parent.width

            DialogHeader {
                title: "Create folder"
            }

            Item {
                width: 1
                height: Theme.paddingLarge
            }

            TextField {
                id: textEntry

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge

                inputMethodHints: Qt.ImhNoAutoUppercase
                placeholderText: "Name"
                focus: true
            }

            Item {
                width: 1
                height: Theme.paddingLarge * 3
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "image://theme/icon-m-folder"
            }
        }
    }
}
