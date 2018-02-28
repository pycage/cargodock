import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {

    property string _style: "<style>" +
                            "a:link { color:" + Theme.highlightColor + "} " +
                            "h3 { " +
                            "  color: " + Theme.highlightColor + ";" +
                            "} " +
                            "</style>"

    FileReader {
        id: docReader
        source: Qt.resolvedUrl("../help.html")
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            height: childrenRect.height
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Help")
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                textFormat: Text.RichText
                text: _style + docReader.data

                onLinkActivated: {
                    Qt.openUrlExternally(link);
                }
            }

        }

        ScrollDecorator { }

    }

}
