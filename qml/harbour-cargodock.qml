import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "cover"
import "pages"
import "services"

ApplicationWindow
{    
    id: win

    // the folder page
    property var folderPage

    // dictionary of services
    property var _serviceObjects: ({ })
    property var serviceNames: []

    /* Registers the given service object.
     */
    function registerServiceObject(serviceObject)
    {
        console.log("registering service " + serviceObject.serviceName);
        _serviceObjects[serviceObject.serviceName] = serviceObject;
        serviceNames.push(serviceObject.serviceName);
    }

    /* Returns the service object registered for the given name, or undefined
     * if the service is not registered.
     */
    function serviceObject(serviceName)
    {
        return _serviceObjects[serviceName];
    }

    /* Refreshes the panes.
     */
    function refreshPanes()
    {
        folderPage.refresh();
    }

    initialPage: loaderPageComponent
    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    Component.onCompleted: {
        folderPage = pageStack.pushAttached(folderPageComponent);
        pageStack.navigateForward(PageStackAction.Immediate);
    }

    /******************************************
     * Register the filesystem services here. *
     ******************************************/
    PlacesService { }
    LocalService { }

    // cloud services
    DropboxService { }
    OwnCloudService { }
    TelekomMediencenter { }
    WebDavService { }
    /******************************************/

    QtObject {
        id: sharedState
        property bool actionInProgress
        property string actionName
        property string actionTarget
        property double actionProgress
        property variant currentContentModel
        property bool isSecondPane
    }

    DeveloperMode {
        id: developerMode
    }

    Notification {
        id: notification
    }

    PassphraseGuard {
        id: passphraseGuard
    }

    SslHandler { }

    Component {
        id: loaderPageComponent

        Page {
            id: page
            BusyIndicator {
                running: page.status === PageStatus.Active
                anchors.centerIn: parent
                size: BusyIndicatorSize.Large
            }

            onStatusChanged: {
                if (status === PageStatus.Active && folderPage)
                {
                    pageStack.navigateForward(PageStackAction.Immediate)
                    folderPage.goUp();
                }
            }
        }
    }

    Component {
        id: folderPageComponent

        FolderPageNg { }
    }

}
