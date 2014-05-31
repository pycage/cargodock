import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    property bool confirm
    property variant passphraseChecker
    property bool _passphraseOk: confirm ? passphraseChecker.checkEncryptionPassphrase(entryPassphrase.text)
                                         : true

    signal passphraseAccepted(string passphrase)

    canAccept: entryPassphrase.text !== "" && _passphraseOk

    Column {
        width: parent.width

        DialogHeader {
            title: "Accept"
        }

        Label {
            visible: ! confirm
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.rightMargin: Theme.paddingLarge
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            text: "Passwords are saved encrypted with a pass phrase. This " +
                  "pass phrase is not saved anywhere."
        }

        Label {
            visible: confirm
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.rightMargin: Theme.paddingLarge
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            text: "Enter current pass phrase to confirm."
        }

        PasswordField {
            id: entryPassphrase

            anchors.left: parent.left
            anchors.right: parent.right

            placeholderText: "Enter passphrase"
            label: "Passphrase"
        }
    }

    onAccepted: {
        passphraseAccepted(entryPassphrase.text)
    }
}
