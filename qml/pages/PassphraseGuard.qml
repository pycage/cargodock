import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: guard

    property var callback

    Connections {
        target: pageStack
        onBusyChanged: {
            if (! pageStack.busy && guard.callback)
            {
                guard.callback();
                guard.callback = undefined;
            }
        }
    }

    function run(serviceObj, placesModel, callback)
    {
        console.log("serviceObj uses encryption " + serviceObj.usesEncryption);
        if (serviceObj.usesEncryption)
        {
            if (placesModel.useEncryptionPassphrase &&
                placesModel.checkEncryptionPassphrase(""))
            {
                var props = {
                    "mode": "ask",
                    "passphraseChecker": placesModel
                };

                var dlg = pageStack.push(Qt.resolvedUrl("PassphraseDialog.qml"),
                                         props);
                dlg.passphraseAccepted.connect(function(passphrase) {
                    placesModel.setEncryptionPassphrase(passphrase);
                    guard.callback = callback;
                });
                return;
            }
        }
        callback();
    }

}
