import QtQuick 2.0
import Sailfish.Silica 1.0

Item {

    Connections {
        target: sslHandler
        onError: {
            pageStack.push(sslErrorDialog);
        }
    }

    Component {
        id: sslErrorDialog

        Dialog {

            DialogHeader {
                title: "Accept"
            }

            Label {
                anchors.centerIn: parent
                width: parent.width - 2 * Theme.paddingLarge
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignHCenter

                text: "Accept this SSL certificate?"
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
