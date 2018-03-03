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

    signal pressAndHold

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
            fileListView.pressAndHold()
        }
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: ! fileListView.model.isValid
        text: qsTr("Not available")
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: ! fileListView.model.isReadable
        text: qsTr("You have no permission for this folder")
    }

    ViewPlaceholder {
        flickable: fileListView
        enabled: fileListView.count === 0 && fileListView.model.isReadable
        text: qsTr("No files")
    }

    VerticalScrollDecorator { }
}
