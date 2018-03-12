import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

PreviewItem {

    TextArea {
        anchors.fill: parent
        readOnly: true
        wrapMode: Text.NoWrap
        font.pixelSize: Theme.fontSizeSmall
        text: fileInfo.readFile()
        onPressAndHold: {
             pageStack.push(fullPageComponent);
        }
    }

    Component {
        id: fullPageComponent
        Page {
            id: page
            property string currentText: fileInfo.readFile()
            allowedOrientations: Orientation.All
            SilicaFlickable {
                id: fl
                anchors.fill: parent
                contentHeight: editor.height
                TextArea{
                    id: editor
                    text: page.currentText
                    autoScrollEnabled: true;
                    font {
                        family: Theme.fontFamily;
                        pixelSize: Theme.fontSizeMedium
                    }
                    width: Math.max(page.width * 2, implicitWidth +1)
                    anchors.fill: parent
                    wrapMode: Text.NoWrap
                    readOnly: (fileInfo.capabilities & FolderBase.CanDelete)
                    background: null
                }
                contentWidth: page.width + editor.width // for long lines
                ScrollDecorator {}
            }
            onStatusChanged: {
                if(status===PageStatus.Deactivating&&editor.text != currentText){
                    fileInfo.writeFile(editor.text)
                }
            }
        }
    }

}
