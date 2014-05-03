import QtQuick 2.0
import Sailfish.Silica 1.0

Item {

    Connections {
        target: sslHandler
        onError: {
            var props = {
                "message": message,
                "details": details
            };

            pageStack.push(sslErrorDialog, props);
        }
    }

    Component {
        id: sslErrorDialog

        Dialog {

            property string message
            property string details

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: column.height

                Column {
                    id: column
                    width: parent.width

                    DialogHeader {
                        title: "Accept certificate"
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width - 2 * Theme.paddingLarge
                        font.pixelSize: Theme.fontSizeMedium
                        color: Theme.secondaryColor
                        wrapMode: Text.Wrap

                        text: message
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    SectionHeader {
                        text: "Certificate"
                    }

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width - 2 * Theme.paddingLarge
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.secondaryColor
                        wrapMode: Text.Wrap
                        textFormat: Text.RichText

                        text: details
                    }
                }

                ScrollDecorator { }
            }


            onAccepted: {
                sslHandler.accept();
            }

            onRejected: {
                sslHandler.decline();
            }

        }
    }
}
