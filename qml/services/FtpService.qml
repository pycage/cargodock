import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "../shared"

ServiceObject {
    id: service

    name: qsTr("FTP Server")
    icon: "image://theme/icon-m-region"
    usesEncryption: true
    serviceName: "ftp"
    serviceModel: FtpModel { }

    serviceDelegate: ServiceDelegate {
        title: "FTP"
        subtitle: qsTr("Access a FTP server.")
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
                        title: qsTr("Configure FTP")
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
                        label: qsTr("Name")
                        placeholderText: qsTr("Enter name")
                    }

                    TextField {
                        id: addressEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        inputMethodHints: Qt.ImhNoPredictiveText
                        label: qsTr("Server")
                        placeholderText: qsTr("Enter server address")
                        text: "ftp.example.com"
                    }

                    SectionHeader {
                        text: qsTr("Authorization")
                    }

                    Label {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.rightMargin: Theme.paddingLarge
                        wrapMode: Text.Wrap
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        text: qsTr("Leave empty if the server requires no authorization. " +
                              "Please keep in mind that FTP is an insecure protocol " +
                              "and transfers all data, including your login and password " +
                              "in plain text.")
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
                        placeholderText: qsTr("Enter login name")
                        label: qsTr("Login")
                    }

                    PasswordField {
                        id: passwordEntry

                        anchors.left: parent.left
                        anchors.right: parent.right

                        placeholderText: qsTr("Enter password")
                        label: qsTr("Password")
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
