import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

SilicaListView {
    id: fileListView

    property TrailModel trailModel

    // true if the view is in selection mode
    property bool selectionMode

    // amount of selected items
    property int selectionSize: model ? model.selected : 0

    //model: trailModel.fsModel

    function selectAll()
    {
        model.selectAll();
    }

    function unselectAll()
    {
        model.unselectAll();
    }

    onTrailModelChanged: {
        model = trailModel.fsModel;
    }

    onModelChanged: {
        console.log("model changed");
    }

    header: Column {
        width: fileListView.width

        Trail {
            width: fileListView.width
            height: Theme.itemSizeLarge

            model: trailModel
        }

        ListItem {
            id: newFolderItem
            visible: fileListView.model.isValid &&
                     fileListView.model.isWritable &&
                     ! fileListView.model.loading
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: newFolderIcon
                x: Theme.paddingMedium
                width: height
                height: parent.height
                fillMode: Image.Pad
                source: "image://theme/icon-m-add"
            }

            Label {
                anchors.left: newFolderIcon.right
                anchors.leftMargin: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                text: "New folder ..."
            }

            onClicked: {
                var dlg = pageStack.push(Qt.resolvedUrl("NewFolderDialog.qml"));

                function closure(model, dlg)
                {
                    return function()
                    {
                        model.newFolder(dlg.name);
                    }
                }

                dlg.accepted.connect(closure(fileListView.model, dlg));
            }
        }

        Separator {
            visible: newFolderItem.visible
            width: parent.width
            horizontalAlignment: Qt.AlignHCenter
            color: Theme.highlightColor
        }
    }

    section.property: "section"
    section.delegate: SectionHeader {
        text: section
    }

    delegate: FileDelegate {
        fileInfo: FileInfo {
            source: model
            sourceModel: fileListView.model
        }

        selected: selectionMode && model.selected
        height: Theme.itemSizeSmall
        opacity: (selectionMode && ! model.selectable) ? 0.3 : 1

        onClicked: {
            if (! selectionMode)
            {
                if (model.modelTarget)
                {
                    // open new model
                    var obj = serviceObject(model.modelTarget);
                    if (obj)
                    {
                        var fsModel = obj.createModel(model.linkTarget);
                        trailModel.push(fsModel,
                                        model.friendlyName,
                                        fileInfo.icon);
                    }
                }
                else if (model.type === FolderBase.File ||
                         model.type === FolderBase.FileLink)
                {
                    // open file
                    var props = {
                        "fileInfo": fileInfo
                    };
                    pageStack.push(Qt.resolvedUrl("FileInfoDialog.qml"),
                                   props);
                }
                else
                {
                    // open folder
                    trailModel.push(fileListView.model,
                                    model.name,
                                    fileInfo.icon);
                    fileListView.model.open(model.name);
                }
            }
            else if (model.selectable)
            {
                // selection mode
                fileListView.model.setSelected(index, ! model.selected);
            }
        }

        onPressAndHold: {
            if (! selectionMode)
            {
                if (model.selectable)
                {
                    fileListView.model.setSelected(index, true);
                }
                selectionMode = true;
            }
            else
            {
                selectionMode = false;
                fileListView.model.unselectAll();
            }
        }
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: ! fileListView.model.isValid
        text: "Not available"
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: ! fileListView.model.isReadable
        text: "You have no permission for this folder"
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: fileListView.count === 0 && fileListView.model.isReadable
        text: "No files"
    }

    VerticalScrollDecorator { }
}
