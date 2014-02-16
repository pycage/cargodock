import QtQuick 2.0
import Sailfish.Silica 1.0

Page {

    property alias url: webview.url
    property string redirectionUri

    signal accepted

    // work around Silica bug: don't let webview enable forward navigation
    onForwardNavigationChanged: {
        if (forwardNavigation)
            forwardNavigation = false;
    }

    SilicaWebView {
        id: webview

        anchors.fill: parent
        opacity: loading ? 0.5 : 1

        experimental.userAgent: "Mozilla/5.0 (Maemo; Linux; Jolla; Sailfish; Mobile) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13"

        onUrlChanged: {
            console.log("URL: " + url);
            console.log("Redirection URI: " + redirectionUri);
            if (redirectionUri !== "" &&
                    ("" + url).indexOf(redirectionUri) === 0)
            {
                accepted();
                pageStack.pop();
            }
        }
    }

    BusyIndicator {
        running: webview.loading
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
    }

}
