import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "cover"
import "pages"
import "services"

ApplicationWindow
{    
    id: win

    // array of the folder panes
    property var _panes: []

    // dictionary of services
    property var _serviceObjects: ({ })
    property var serviceNames: []

    property var path1: []
    property var path2: []

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

    /* Registers the given page as a content pane.
     */
    function registerContentPane(page)
    {
        _panes.push(page);
    }

    function updateDestinationContentModels()
    {
        if (_panes.length === 2)
        {
            _panes[0].destinationModel = _panes[1].sourceModel;
            _panes[1].destinationModel = _panes[0].sourceModel;
        }
    }


    /* Refreshes the panes.
     */
    function refreshPanes()
    {
        console.log("refreshing panes");
        for (var i = 0; i < _panes.length; ++i)
        {
            _panes[i].sourceModel.refresh();
        }
    }

    initialPage: folderPageNg
    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    Component.onCompleted: {


        /*
        var props = {
            "isSecondPane": true
        };

        pageStack.pushExtra(folderPage, props);
        updateDestinationContentModels();
        */
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

    Rectangle {
        width: parent.width
        height: childrenRect.height + 2 * Theme.paddingLarge
        anchors.bottom: parent.bottom

        gradient: Gradient {
            GradientStop { position: 0; color: "transparent" }
            GradientStop { position: 1; color: "black" }
        }

        Behavior on opacity {
            NumberAnimation { duration: 1500; easing.type: Easing.OutCurve }
        }

        Label {
            y: Theme.paddingLarge
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeExtraLarge
            color: Theme.highlightColor
            text: "Pull down for help"
        }

        Timer {
            running: true
            repeat: false
            interval: 1500

            onTriggered: {
                parent.opacity = 0;
            }
        }
    }

    Component {
        id: loaderPage

        Page {
            id: page
            BusyIndicator {
                running: page.status === PageStatus.Active
                anchors.centerIn: parent
                size: BusyIndicatorSize.Large
            }

            onStatusChanged: {
                if (status === PageStatus.Active)
                {
                    if (path1.length === 0)
                    {
                        var model = serviceObject("places").createModel("places");
                        var props = {
                            "sourceModel": model
                        };
                        var page = pageStack.push(folderPage, props);
                    }
                    else
                    {

                    }
                }
            }
        }
    }

    Component {
        id: folderPageNg

        FolderPageNg { }
    }

    /*
    Component {
        id: folderPage

        FolderPage {
            id: page

            Component.onCompleted: {
                registerContentPane(page);
            }

            onModelChanged: {
                updateDestinationContentModels();
            }

            onCopyCommand: {
                sharedState.actionName = "copying";
                sharedState.actionInProgress = true;
                sourceModel.copySelected(destModel);
            }

            onDeleteCommand: {
                sharedState.actionName = "deleting";
                sharedState.actionInProgress = true;
                sourceModel.deleteItems(items);
            }

            onLinkCommand: {
                sharedState.actionName = "linking";
                sharedState.actionInProgress = true;
                sourceModel.linkSelected(destModel);
            }

            onError: {
                notification.show(details);
            }

            onFinished: {
                sharedState.actionInProgress = false;
                refreshPanes();
            }
        }
    }
    */
}
