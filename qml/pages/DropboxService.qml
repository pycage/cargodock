import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

ServiceObject {
    id: service

    icon: Qt.resolvedUrl("../dropbox.png")
    serviceName: "dropbox"
    name: "Dropbox"

    serviceModel: DropboxModel {
        id: dropboxModel

        onAuthorizationRequired: {
            var props = {
                "url": url,
                "redirectionUri": redirectionUri
            }

            console.log("URL: " + url);
            console.log("Redirection URI: " + redirectionUri);

            notification.show("Authorization required.");
            var dlg = pageStack.push(Qt.resolvedUrl("OAuthDialog.qml"), props);

            function f(model, dlg)
            {
                return function()
                {
                    model.authorize(dlg.url);
                }
            }

            dlg.accepted.connect(f(dropboxModel, dlg));
        }
    }

    serviceDelegate: ServiceDelegate {
        iconSource: icon
        title: qsTr("Dropbox")
        subtitle: qsTr("Login to a Dropbox account.")
    }

    serviceConfigurator: Component {
        Dialog {
            signal serviceConfigured(string serviceName,
                                     string icon,
                                     string name,
                                     variant properties)

            property alias name: textEntry.text

            canAccept: textEntry.text !== ""

            SilicaFlickable {
                anchors.fill: parent

                Column {
                    width: parent.width

                    DialogHeader {
                        title: qsTr("Configure Dropbox")
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

                        placeholderText: qsTr("Name")
                        focus: true
                    }

                    Item {
                        width: parent.width
                        height: childrenRect.height

                        Image {
                            id: dropboxIcon
                            x: Theme.paddingLarge
                            source: Qt.resolvedUrl("../dropbox.png")
                        }

                        Label {
                            anchors.left: dropboxIcon.right
                            anchors.right: parent.right
                            anchors.leftMargin: Theme.paddingLarge
                            anchors.rightMargin: Theme.paddingLarge
                            wrapMode: Text.Wrap
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.secondaryColor
                            text: qsTr("Enter the name under which you want to ") +
                                  qsTr("access your Dropbox folder in ") +
                                  qsTr("Cargo Dock.\n") +
                                  qsTr("You may add several Dropbox services ") +
                                  qsTr("connected to different Dropbox accounts. ") +
                                  qsTr("The name will help you distinguish them.")
                        }
                    }
                }
            }

            onAccepted: {
                serviceConfigured(service.serviceName,
                                  service.icon,
                                  textEntry.text,
                                  { });
            }
        }//Dialog

    }//Component
}
