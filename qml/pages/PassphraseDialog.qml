import QtQuick 2.0
import Sailfish.Silica 1.0
import "../shared"

Dialog {
    id: page

    // "new", "ask", or "confirm"
    property string mode: "ask"
    property variant passphraseChecker

    property bool _passphraseOk: mode === "confirm" || mode === "ask"
                                 ? passphraseChecker.verifyEncryptionPassphrase(entryPassphrase.text)
                                 : mode === "new"
                                   ? entryPassphrase.text === entryVerify.text
                                   : true

    signal passphraseAccepted(string passphrase)

    canAccept: entryPassphrase.text !== "" && _passphraseOk

    Column {
        width: parent.width

        DialogHeader {
            title: qsTr("Accept")
        }

        Item {
            width: 1
            height: Theme.paddingLarge
        }

        Item {
            width: parent.width
            height: childrenRect.height

            Image {
                id: lockIcon
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingLarge
                source: "image://theme/icon-m-device-lock"
            }

            Label {
                anchors.left: lockIcon.right
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: mode === "ask"
                      ? qsTr("Enter your passphrase to unlock the password storage.")
                      : mode === "new"
                        ? qsTr("Set a passphrase to secure the password storage.\n" +
                          "If you forget the passphrase, your stored passwords will " +
                          "become unusable.")
                        : qsTr("Enter current passphrase to confirm.")
            }
        }

        Item {
            width: 1
            height: Theme.paddingLarge
        }

        PasswordField {
            id: entryPassphrase

            anchors.left: parent.left
            anchors.right: parent.right

            placeholderText: qsTr("Enter passphrase")
            label: qsTr("Passphrase")
        }

        PasswordField {
            id: entryVerify
            visible: mode === "new"

            anchors.left: parent.left
            anchors.right: parent.right

            placeholderText: qsTr("Verify passphrase")
            label: qsTr("Passphrase")
        }
    }

    Column {
        visible: mode === "confirm" && parent.height === Screen.height
        width: parent.width
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.paddingLarge
        spacing: Theme.paddingMedium

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.rightMargin: Theme.paddingLarge
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            text: qsTr("If you forgot your passphrase, the stored passwords will " +
                  "become unusable and you will have to enter them again.")
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("I forgot")

            onClicked: {
                remorse.execute(qsTr("Resetting passphrase"),
                                function() {
                                    page.passphraseAccepted("");
                                    page.close();
                                });
            }

            RemorsePopup {
                id: remorse
            }
        }
    }

    onAccepted: {
        passphraseAccepted(entryPassphrase.text);
    }
}
