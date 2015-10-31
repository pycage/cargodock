import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property ListModel trailModels: ListModel { }

    Component {
        id: trailModelComponent

        TrailModel {
            onFsModelChanged: fileView.model = fsModel
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

        clip: true

        Connections {
            ignoreUnknownSignals: true
            target: fileView.model
            onError: {
                notification.show(details);
            }
        }

        PullDownMenu {
            MenuItem {
                text: "About"

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"));
                }
            }

            MenuItem {
                text: "Settings"

                onClicked: {
                    var props = {
                        "placesModel": fileView.trailModel.rootModel
                    }

                    pageStack.push(Qt.resolvedUrl("SettingsPage.qml"), props);
                }
            }
        }
    }

    Item {
        id: toolBox

        property var sourceModel: null
        property var selection: []

        function copyTo(destModel)
        {
            console.log("Copying to " + destModel.path + ", " + selection);
            sourceModel.copyItems(destModel, selection);
        }

        function deleteItems(model, items)
        {
            model.deleteItems(items);
        }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.bottom
        anchors.topMargin: fileView.selectionMode ? -height : 0
        height: Theme.itemSizeLarge

        Behavior on anchors.topMargin {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }

        Row {
            anchors.fill: parent
            spacing: Theme.paddingSmall

            Button {
                visible: toolBox.sourceModel == null
                text: "A"
                width: height
                onClicked: {
                    fileView.selectAll();
                }
            }

            Button {
                visible: toolBox.sourceModel == null
                text: "N"
                width: height
                onClicked: {
                    fileView.unselectAll();
                }
            }

            Label {
                text: qsTr("%1 selected").arg(fileView.selectionSize)
            }

            Button {
                visible: toolBox.sourceModel == null
                text: "X"
                width: height
                onClicked: {
                    fileView.selectAll();
                }
            }

            Button {
                visible: toolBox.sourceModel == null
                text: "C"
                width: height
                onClicked: {
                    toolBox.sourceModel = fileView.model;
                    toolBox.selection = fileView.model.selection;
                }
            }

            Button {
                visible: toolBox.sourceModel != null
                text: "V"
                width: height
                onClicked: {
                    toolBox.copyTo(fileView.model);
                    toolBox.sourceModel = null;
                    toolBox.selection = [];
                }
            }

            Button {
                text: "Del"
                width: height

                onClicked: {
                    console.log("Deleting items: " + fileView.model.selection);

                    function closure(toolBox, model, items)
                    {
                        return function()
                        {
                            toolBox.deleteItems(model, items);
                        }
                    }

                    var msg = qsTr("Deleting %1 items").arg(fileView.model.selected);
                    remorse.execute(msg,
                                    closure(toolBox,
                                            fileView.model,
                                            fileView.model.selection.slice()));
                }
            }
        }
    }

    Component {
        id: panesPageComponent

        PanesPage {
            trails: page.trailModels

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
