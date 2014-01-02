import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0
import "cover"
import "pages"

ApplicationWindow
{    
    // array of the folder panes
    property variant _panes: []

    /* Registers the given page as a content pane.
     */
    function registerContentPane(page)
    {
        var p = _panes;
        p.push(page);
        _panes = p;
    }

    /* Returns the destination model for the given source model, i.e. the
     * current model of the other pane.
     * Returns null in case there is no destination model, which e.g. is the
     * case if the other pane is showing the places list.
     */
    function destinationContentModel(sourceModel)
    {
        var destModel = null;
        for (var i = 0; i < _panes.length; ++i)
        {
            var model = _panes[i].currentContentModel();
            if (model !== sourceModel)
            {
                destModel = model;
                break;
            }
        }
        return destModel;
    }

    initialPage: folderPage
    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    Component.onCompleted: {
        var props = {
            "isSecondPane": true
        };

        pageStack.pushExtra(folderPage, props);
    }

    QtObject {
        id: sharedState
        property bool actionInProgress
        property string actionName
        property variant currentContentModel
        property bool isSecondPane
    }

    DeveloperMode {
        id: developerMode
    }

    Notification {
        id: notification
    }

    Label {
        width: parent.width
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.paddingLarge
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        font.pixelSize: Theme.fontSizeExtraLarge
        color: Theme.secondaryColor
        text: "Pull down for help"

        Behavior on opacity {
            NumberAnimation { duration: 1500; easing.type: Easing.OutCurve }
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
        id: folderPage

        FolderPage {
            id: page

            Component.onCompleted: {
                registerContentPane(page);
            }

            onCopyCommand: {
                var destModel = destinationContentModel(sourceModel);
                if (destModel)
                {
                    sharedState.actionName = "copying";
                    sharedState.actionInProgress = true;
                    sourceModel.copySelected(destModel);
                }
            }

            onDeleteCommand: {
                sharedState.actionName = "deleting";
                sharedState.actionInProgress = true;
                sourceModel.deleteItems(items);
            }

            onLinkCommand: {
                var destModel = destinationContentModel(sourceModel);
                if (destModel)
                {
                    sharedState.actionName = "linking";
                    sharedState.actionInProgress = true;
                    sourceModel.linkSelected(destModel);
                }
            }

            onError: {
                sharedState.actionInProgress = false;
                notification.show(details);
            }

            onFinished: {
                sharedState.actionInProgress = false;

                console.log("Finished action. Refreshing panes.");

                for (var i = 0; i < _panes.length; ++i)
                {
                    _panes[i].currentContentModel().refresh();
                }
            }
        }
    }
}
