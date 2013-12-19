import QtQuick 2.0
import Sailfish.Silica 1.0
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
        pageStack.pushExtra(folderPage, { "secondPane": true });
    }

    QtObject {
        id: sharedState
        property bool actionInProgress
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
                    sharedState.actionInProgress = true;
                    sourceModel.copySelected(destModel);
                }
            }

            onDeleteCommand: {
                sharedState.actionInProgress = true;
                sourceModel.deleteSelected();
            }

            onLinkCommand: {
                var destModel = destinationContentModel(sourceModel);
                if (destModel)
                {
                    sharedState.actionInProgress = true;
                    sourceModel.linkSelected(destModel);
                }
            }

            onError: {
                sharedState.actionInProgress = false;
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
