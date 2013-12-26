import QtQuick 2.0
import Sailfish.Silica 1.0

PreviewItem {

    TextArea {
        anchors.fill: parent
        readOnly: true
        wrapMode: Text.NoWrap
        font.pixelSize: Theme.fontSizeSmall
        text: fileInfo.readFile()
    }

}
