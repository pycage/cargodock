import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property ListModel trailModels: ListModel { }

    function refresh()
    {
        fileView.model.refresh();
    }

    Component {
        id: trailModelComponent

        TrailModel {
            onFsModelChanged: fileView.model = fsModel

            onProgress: {
                console.log("Progress: " + name + " " + amount + "%");
                toolBox.progress.name = name;
                toolBox.progress.amount = amount;
            }

            onFinished: {
                toolBox.mode = 0;
                toolBox.opened = false;
                fileView.model.refresh();
                console.log("Finished");
            }

            onError: {
                notification.show(details);
                console.log("Error: " + details);
            }
        }
    }

    function addPane()
    {
        var trailModel = trailModelComponent.createObject(page);
        trailModels.append({"trailModel": trailModel});
        if (trailModels.count === 1)
        {
            fileView.trailModel = trailModel;
        }
    }

    function goUp()
    {
        fileView.trailModel.pop(1);
    }

    Component.onCompleted: {
        addPane();
    }

    onStatusChanged: {
        if (status === PageStatus.Active)
        {
            pageStack.pushAttached(panesPageComponent);
        }
    }

    RemorsePopup {
        id: remorse
    }

    FileListView {
        id: fileView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: toolBox.top

        selectionMode: toolBox.opened && toolBox.mode === 0
        clip: true

        onPressAndHold: {
            toolBox.opened = ! toolBox.opened;
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("About")

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"));
                }
            }

            MenuItem {
                text: qsTr("Settings")

                onClicked: {
                    var props = {
                        "placesModel": fileView.trailModel.rootModel
                    }

                    pageStack.push(Qt.resolvedUrl("SettingsPage.qml"), props);
                }
            }

            MenuItem {
                visible: toolBox.opened && toolBox.mode === 0
                text: qsTr("Select none")
                onClicked: {
                    fileView.unselectAll();
                }
            }

            MenuItem {
                visible: toolBox.opened && toolBox.mode === 0
                text: qsTr("Select all")
                onClicked: {
                    fileView.selectAll();
                }
            }
        }
    }

    ToolBox {
        id: toolBox

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.bottom
        anchors.topMargin: opened ? -height : -Theme.itemSizeSmall

        fileView: fileView
        remorse: remorse

        Behavior on anchors.topMargin {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }
    }

    Component {
        id: panesPageComponent

        PanesPage {
            trails: page.trailModels
            currentTrailModel: fileView.trailModel

            onOpened: {
                fileView.trailModel = model;
                pageStack.navigateBack();
            }

            onNewPane: {
                page.addPane();
            }

            onRemovePane: {
                page.trailModels.remove(idx);
                var newIndex = Math.min(page.trailModels.count - 1, idx);
                var m = page.trailModels.get(newIndex).trailModel;
                console.log("new model is " + m.fsModel.name);
                fileView.trailModel = m;
            }
        }
    }
}
