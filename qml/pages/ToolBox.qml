import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Item {
    id: toolBox

    // whether the tool box is opened
    property bool opened

    // current mode
    // 0: Selection Mode
    // 1: Copy Mode
    // 2: Progress Mode
    property int mode: 0

    // the current file view on which to operate
    property FileListView fileView

    // the remorse popup to use
    property RemorsePopup remorse

    property var progress: QtObject {
        property string name
        property string amount
    }

    // the source model selected for actions
    property var _sourceModel: null
    // array of paths in the source model
    property var _selection: []

    /* Copies the selected items to the destination.
     */
    function copyTo(destModel)
    {
        console.log("Copying to " + destModel.path + ", " + _selection);
        _sourceModel.copyItems(destModel, _selection);
    }

    /* Copies the selected items to the destination.
     */
    function linkTo(destModel)
    {
        _sourceModel.linkItems(destModel, _selection);
    }

    /* Deletes the given items in the model.
     */
    function deleteItems(model, items)
    {
        model.deleteItems(items);
    }

    height: Theme.itemSizeSmall + Theme.itemSizeLarge


    BackgroundItem {
        id: closeSection
        width: parent.width
        height: Theme.itemSizeSmall

        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0; color: Theme.highlightColor }
                GradientStop { position: 0.1; color: "transparent" }
                GradientStop { position: 1; color: "transparent" }
            }
        }

        Image {
            id: btnToggleOpen
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            source: toolBox.opened ? "image://theme/icon-m-down"
                                        : "image://theme/icon-m-up"
        }

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: Theme.paddingSmall
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignRight
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.highlightColor
            text: mode === 0 ? qsTr("%1 selected").arg(fileView.selectionSize)
                             : mode === 1 ? qsTr("Copy %1 items").arg(toolBox._selection.length)
                                          : toolBox.progress.name
        }

        onClicked: {
            toolBox.opened = ! toolBox.opened;
        }
    }

    Item {
        id: selectionSection

        visible: toolBox.mode === 0

        width: parent.width
        height: Theme.itemSizeLarge
        anchors.top: closeSection.bottom

        Row {
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.paddingMedium

            IconButton {
                id: btnNewFolder
                enabled: fileView.model.isWritable
                icon.source: "image://theme/icon-m-folder"
                onClicked: {
                    var dlg = pageStack.push(Qt.resolvedUrl("NewFolderDialog.qml"));

                    function closure(model, dlg)
                    {
                        return function()
                        {
                            model.newFolder(dlg.name);
                        }
                    }

                    dlg.accepted.connect(closure(fileView.model, dlg));
                }
            }

            IconButton {
                id: btnBookmark
                enabled: fileView.model.capabilities & FolderBase.CanBookmark
                icon.source: "image://theme/icon-m-favorite"
                onClicked: {
                    var placesModel = fileView.trailModel.rootModel;
                    toolBox._sourceModel = fileView.model;
                    toolBox._selection = fileView.model.selection;
                    toolBox.linkTo(placesModel);
                    toolBox._sourceModel = null;
                    toolBox._selection = [];
                }
            }

            IconButton {
                id: btnClipboard
                enabled: fileView.selectionSize > 0 &&
                         (fileView.model.capabilities & FolderBase.CanCopy ||
                          fileView.model.capabilities & FolderBase.CanLink)
                icon.source: "image://theme/icon-m-clipboard"
                onClicked: {
                    toolBox._sourceModel = fileView.model;
                    toolBox._selection = fileView.model.selection;
                    toolBox.mode = 1;
                }
            }

        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.paddingMedium

            IconButton {
                id: btnDelete
                enabled: fileView.model.capabilities & FolderBase.CanDelete
                icon.source: "image://theme/icon-m-delete"
                onClicked: {
                    console.log("Deleting items: " + fileView.model.selection);

                    function closure(toolBox, model, items)
                    {
                        return function()
                        {
                            toolBox.deleteItems(model, items);
                        }
                    }

                    var msg = qsTr("Delete %1 items").arg(fileView.model.selected);
                    remorse.execute(msg,
                                    closure(toolBox,
                                            fileView.model,
                                            fileView.model.selection.slice()));
                }
            }
        }

    }

    Item {
        id: copySection

        visible: toolBox.mode === 1

        width: parent.width
        height: Theme.itemSizeLarge
        anchors.top: closeSection.bottom

        Row {
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.paddingMedium

            IconButton {
                id: btnCopyHere
                enabled: _sourceModel &&
                         _sourceModel.capabilities & FolderBase.CanCopy &&
                         fileView.model.capabilities & FolderBase.AcceptCopy
                icon.source: "image://theme/icon-m-clipboard"
                onClicked: {
                    toolBox.copyTo(fileView.model);
                    toolBox._sourceModel = null;
                    toolBox._selection = [];
                    toolBox.mode = 2;
                }
            }

            IconButton {
                id: btnLinkHere
                enabled: _sourceModel &&
                         _sourceModel.capabilities & FolderBase.CanLink &&
                         fileView.model.capabilities & FolderBase.AcceptLink
                icon.source: "image://theme/icon-m-link"
                onClicked: {
                    toolBox.linkTo(fileView.model);
                    toolBox._sourceModel = null;
                    toolBox._selection = [];
                }
            }
        }


        Row {
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.paddingMedium

            IconButton {
                id: btnCopyCancel
                icon.source: "image://theme/icon-m-clear"
                onClicked: {
                    toolBox._sourceModel = null;
                    toolBox._selection = [];
                    toolBox.mode = 0;
                }
            }
        }
    }

    Item {
        id: progressSection

        visible: toolBox.mode === 2

        width: parent.width
        height: Theme.itemSizeLarge
        anchors.top: closeSection.bottom

        ProgressBar {
            id: progressBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: Theme.paddingSmall
            anchors.rightMargin: Theme.paddingSmall

            minimumValue: 0
            maximumValue: 1000
            value: Math.floor(toolBox.progress.amount * 1000)
            label: value + "%"
        }
    }
}

