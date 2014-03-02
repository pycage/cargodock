import QtQuick 2.0
import Sailfish.Silica 1.0

PreviewItem {

    Image {
        id: previewImage
        anchors.centerIn: parent
        fillMode: Image.Pad
        asynchronous: true
        smooth: true
        visible: source !== ""
        sourceSize.width: width
        sourceSize.height: height
        source: fileInfo.icon

        BusyIndicator {
            running: parent.status === Image.Loading
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
        }
    }
}
