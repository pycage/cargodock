import QtQuick 2.0
import QtWebKit 3.0
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.Landscape | Orientation.Portrait

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
        opacity: 0

        //experimental.userAgent: "Mozilla/5.0 (Maemo; Linux; Jolla; Sailfish; Mobile) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13"

        onLoadingChanged: {
            switch (loadRequest.status)
            {
            case WebView.LoadSucceededStatus:
                opacity = 1
                break
            case WebView.LoadFailedStatus:
                opacity = 0
                break
            default:
                opacity = 0
                break
            }
        }

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
