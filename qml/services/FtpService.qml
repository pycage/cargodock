import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "../shared"

ServiceObject {
    id: service

    name: "FTP Server"
    icon: "image://theme/icon-m-region"
    usesEncryption: true
    serviceName: "ftp"
    serviceModel: FtpModel { }

    serviceDelegate: ServiceDelegate {
        title: "FTP"
        subtitle: "Access a FTP server."
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
                addressEntry.text = properties["address"];
                loginEntry.text = properties["login"];
                passwordEntry.text = properties["password:blowfish"];
            }

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: column.height

                Column {
                    id: column
                    width: parent.width

                    DialogHeader {
                        title: "Configure FTP"
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
                        text: "Leave empty if the server requires no authorization. " +
                              "Please keep in mind that FTP is an insecure protocol " +
                              "and transfers all data, including your login and password " +
                              "in plain text."
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
                    "login": loginEntry.text,
                    "password:blowfish": passwordEntry.text
                };

                serviceConfigured(service.serviceName,
                                  service.icon,
                                  props);
            }

        }//Dialog

    }//Component

}
