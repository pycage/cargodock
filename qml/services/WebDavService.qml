import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "../shared"

ServiceObject {
    id: service

    name: "WebDAV"
    icon: "image://theme/icon-m-region"
    usesEncryption: true
    serviceName: "webdav"
    serviceModel: DavModel { }

    serviceDelegate: ServiceDelegate {
        title: "WebDAV"
        subtitle: "Access a WebDAV share."
        iconSource: service.icon
    }

    serviceConfigurator: Component {
        Dialog {

            allowedOrientations: Orientation.All

            signal serviceConfigured(string serviceName,
                                     string icon,
                                     variant properties)

            property var properties

            property var _securityMethods: ["none", "ssl"]

            onPropertiesChanged: {
                nameEntry.text = properties["name"];
                addressEntry.text = properties["address"];
                pathEntry.text = properties["path"];
                loginEntry.text = properties["login"];
                passwordEntry.text = properties["password:blowfish"];

                var securityMethod = properties["securityMethod"];
                var idx = _securityMethods.indexOf(securityMethod);
                securityCombo.currentIndex = idx;
            }

            canAccept: nameEntry.text !== "" &&
                       addressEntry.text !== "" &&
                       pathEntry.text !== ""

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: column.height

                Column {
                    id: column
                    width: parent.width

                    DialogHeader {
                        title: "Configure WebDAV"
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    TextField {
                        id: nameEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        focus: true
                        label: "Name"
                        placeholderText: "Enter name"
                    }

                    TextField {
                        id: addressEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        inputMethodHints: Qt.ImhNoPredictiveText
                        label: "Server"
                        placeholderText: "Enter server address"
                        text: "webdav.example.com"
                    }

                    TextField {
                        id: pathEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        inputMethodHints: Qt.ImhNoPredictiveText
                        placeholderText: "Enter path"
                        label: "Path"
                        text: "/"
                    }

                    ComboBox {
                        id: securityCombo
                        label: "Secure connection"

                        menu: ContextMenu {
                            MenuItem { text: "Not in use" }
                            MenuItem { text: "SSL" }
                        }
                    }

                    SectionHeader {
                        text: "Authorization"
                    }

                    Label {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.rightMargin: Theme.paddingLarge
                        wrapMode: Text.Wrap
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        text: "Leave empty if the server requires no authorization."
                    }

                    Item {
                        width: 1
                        height: Theme.paddingMedium
                    }

                    TextField {
                        id: loginEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        inputMethodHints: Qt.ImhNoPredictiveText
                        placeholderText: "Enter login name"
                        label: "Login"
                    }

                    PasswordField {
                        id: passwordEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        placeholderText: "Enter password"
                        label: "Password"
                    }
                }

                ScrollDecorator { }
            }

            onAccepted: {
                var props = {
                    "name": nameEntry.text,
                    "address": addressEntry.text,
                    "path": pathEntry.text,
                    "login": loginEntry.text,
                    "password:blowfish": passwordEntry.text,
                    "securityMethod": _securityMethods[securityCombo.currentIndex]
                }

                serviceConfigured(service.serviceName,
                                  service.icon,
                                  props);
            }

        }//Dialog

    }//Component
}
