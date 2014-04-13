import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

ServiceObject {
    id: service

    name: "WebDAV"
    icon: "image://theme/icon-m-region"
    serviceName: "webdav"
    serviceModel: DavModel { }

    serviceDelegate: ServiceDelegate {
        title: "WebDAV"
        subtitle: "Access a WebDAV share."
    }

    serviceConfigurator: Component {
        Dialog {
            signal serviceConfigured(string serviceName,
                                     string icon,
                                     string name,
                                     variant properties)

            canAccept: urlEntry.text !== ""

            SilicaFlickable {
                anchors.fill: parent

                Column {
                    width: parent.width

                    DialogHeader {
                        title: "Configure WebDAV"
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    TextField {
                        id: urlEntry

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.rightMargin: Theme.paddingLarge

                        placeholderText: "Address"
                        focus: true
                    }
                }
            }

            onAccepted: {
                serviceConfigured(service.serviceName,
                                  "image://theme/icon-m-region",
                                  urlEntry.text,
                                  { "url": urlEntry.text });
            }

        }//Dialog

    }//Component
}
