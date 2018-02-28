import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "../shared"

ServiceObject {
    id: service

    name: "Telekom® Mediencenter"
    icon: "image://theme/icon-m-folder"
    usesEncryption: true
    serviceName: "telekom-mediencenter"
    serviceModel: DavModel { }

    property string _address: "webdav.mediencenter.t-online.de"
    property string _path: "/"
    property string _securityMethod: "ssl"

    serviceDelegate: ServiceDelegate {
        title: service.name
        subtitle: qsTr("Access Deutsche Telekom® Mediencenter.")
        iconSource: service.icon
    }

    serviceConfigurator: Component {
        Dialog {

            signal serviceConfigured(string serviceName,
                                     string icon,
                                     variant properties)

            property var properties

            onPropertiesChanged: {
                nameEntry.text = properties["name"];
                loginEntry.text = properties["login"];
                passwordEntry.text = properties["password:blowfish"];
            }

            canAccept: nameEntry.text !== "" &&
                       loginEntry.text !== "" &&
                       passwordEntry.text !== ""

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: column.height

                Column {
                    id: column
                    width: parent.width

                    DialogHeader {
                        title: "Mediencenter einrichten"
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    Label {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.rightMargin: Theme.paddingLarge
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        wrapMode: Text.Wrap

                        text: "Diese Zugriffs-Software steht in keinerlei Zusammenhang zur " +
                              "Deutschen Telekom®, sondern nutzt lediglich den " +
                              "WebDAV-Zugang der Telekom Cloud.\n" +
                              "Die Benutzung setzt ein T-Online®-E-Mail-Konto, welches " +
                              "zur Verwendung des Mediencenters berechtigt, voraus.\n" +
                              "Die Benutzung erfolgt auf eigene Gefahr. "+
                              "Der Autor dieser Software haftet nicht für " +
                              "Störungen, Missbrauch oder Datenverlust.\n" +
                              "Bei Akzeptieren dieses Dialoges erklären Sie sich " +
                              "mit den Nutzungsbedingungen einverstanden."
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    TextField {
                        id: nameEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        label: "Verbindungsname"
                        placeholderText: "Verbindungsname eingeben"
                    }

                    TextField {
                        id: loginEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        inputMethodHints: Qt.ImhNoPredictiveText
                        placeholderText: "T-Online-Adresse eingeben"
                        label: "T-Online-Adresse"
                    }

                    PasswordField {
                        id: passwordEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        placeholderText: "Passwort eingeben"
                        label: "Passwort"
                    }
                }

                ScrollDecorator { }
            }

            onAccepted: {
                var props = {
                    "name": nameEntry.text,
                    "address": service._address,
                    "path": service._path,
                    "login": loginEntry.text,
                    "password:blowfish": passwordEntry.text,
                    "securityMethod": service._securityMethod
                }

                serviceConfigured(service.serviceName,
                                  service.icon,
                                  props);
            }

        }//Dialog

    }//Component
}
