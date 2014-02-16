import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

ServiceObject {

    icon: Qt.resolvedUrl("../dropbox.png")
    name: "Dropbox"
    serviceName: "dropbox"

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
        title: name
        subtitle: "Login to a Dropbox account"
    }
}
