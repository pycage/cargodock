import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    property FileInfo fileInfo

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            height: childrenRect.height

            DialogHeader {
                title: "Open"
            }

            Image {
                width: parent.width
                height: 240
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                visible: source !== ""
                source: fileInfo.mimeType.substring(0, 6) === "image/" ? fileInfo.uri
                                                                       : ""
            }

            TextField {
                width: parent.width
                text: fileInfo.name
            }

            KeyValue {
                key: "Type"
                value: fileInfo.mimeType
            }

            KeyValue {
                key: "Path"
                value: fileInfo.path
            }

            KeyValue {
                key: "Size"
                value: Format.formatFileSize(fileInfo.size)
            }

            KeyValue {
                key: "Last modified"
                value: Format.formatDate(fileInfo.mtime, Formatter.TimePoint)
            }

            KeyValue {
                key: "Owner"
                value: "nemo"
            }

            ListItem {
                height: Theme.itemSizeSmall

                Label {
                    id: lblWritable
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    text: "Writable"
                }

                Switch {
                    id: switchWritable
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    anchors.verticalCenter: lblWritable.verticalCenter
                }
            }

        }

        ScrollDecorator { }
    }
}
