import QtQuick 2.0
import Sailfish.Silica 1.0

PreviewItem {

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        smooth: true
        visible: source !== ""
        sourceSize.width: width
        sourceSize.height: height
        source: fileInfo.preview
    }

}
