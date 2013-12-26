import QtQuick 2.0
import Sailfish.Silica 1.0

PreviewItem {

    Image {
        id: iconImage
        anchors.centerIn: parent
        asynchronous: true
        fillMode: Image.Pad
        source: fileInfo.icon !== "" ? fileInfo.icon : fileInfo.uri
    }

}
