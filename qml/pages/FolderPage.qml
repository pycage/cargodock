import QtQuick 2.0
import Sailfish.Silica 1.0
import org.pycage.cargodock 1.0

Page {
    id: page

    property variant _modelStack: []

    property variant _breadcrumbs: []

    property bool _selectionMode: false

    signal copyCommand(variant sourceModel)
    signal deleteCommand(variant sourceModel, variant items)
    signal linkCommand(variant sourceModel)
    signal finished()
    signal error(string details)

    function currentContentModel()
    {
        return _modelStack[_modelStack.length - 1];
    }

    function pushModel(name, path)
    {
        var props = {
            "path": path
        };
        var model;
        if (name === "places")
        {
            model = placesModelComponent.createObject(page, props);
        }
        else
        {
            model = folderModelComponent.createObject(page, props);
        }

        var stack = _modelStack;
        stack.push(model);
        _modelStack = stack;
        contentlist.model = model;
    }

    function popModels(model)
    {
        var stack = _modelStack;
        while (stack.length > 0 && stack[stack.length - 1] !== model)
        {
            stack.pop();
        }
        _modelStack = stack;
        contentlist.model = _modelStack[_modelStack.length - 1];
    }

    function collectBreadcrumbs(currentBreadcrumbs)
    {
        var crumbs = [];
        for (var i = 0; i < _modelStack.length; ++i)
        {
            var model = _modelStack[i];
            for (var j = 0; j < model.breadcrumbs.length; ++j)
            {
                var item = {
                    "name": model.breadcrumbs[j],
                    "model": model,
                    "level": model.breadcrumbs.length - 1 -j
                };
                crumbs.push(item);
            }
        }

        // we're not interested in the last crumbs (current folder)
        if (crumbs.length > 0) {
            crumbs.pop();
        }

        return crumbs;
    }

    Component {
        id: folderModelComponent
        FolderModel {

            onFinished: {
                page.finished();
            }
            onError: {
                page.error(details);
            }
        }
    }

    Component {
        id: placesModelComponent
        PlacesModel {

            onFinished: {
                page.finished();
            }
            onError: {
                page.error(details);
            }
        }
    }

    Component.onCompleted: {
        pushModel("places", "Places");
    }

    RemorsePopup {
        id: remorse
    }

    Drawer {
        anchors.fill: parent
        dock: Dock.Bottom
        open: _selectionMode
        backgroundSize: 340

        background: SilicaFlickable {
            id: drawerView
            anchors.fill: parent
            contentHeight: 340
            clip: true

            PushUpMenu {
                enabled: ! sharedState.actionInProgress

                MenuItem {
                    enabled: contentlist.model.selected > 0
                    text: "Copy to other side"

                    onClicked: {
                        page.copyCommand(contentlist.model);
                    }
                }
                MenuItem {
                    enabled: contentlist.model.selected > 0
                    text: "Link to other side"

                    onClicked: {
                        page.linkCommand(contentlist.model);
                    }
                }
                MenuItem {
                    enabled: contentlist.model.selected > 0
                    text: "Delete"

                    onClicked: {
                        var text = qsTr("Deleting %1 items")
                        .arg(contentlist.model.selected);

                        function closure(model, items)
                        {
                            return function() {
                                console.log("deleting " + items.length + " items " + items);
                                page.deleteCommand(model, items);
                            }
                        }

                        remorse.execute(text,
                                        closure(contentlist.model,
                                                contentlist.model.selection.slice()));
                    }
                }
            }

            Item {
                width: parent.width
                height: parent.height - Theme.itemSizeSmall

                Separator {
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    color: Theme.highlightColor
                }

                IconButton {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    icon.source: "image://theme/icon-m-close"

                    onClicked: {
                        contentlist.model.unselectAll();
                        _selectionMode = false;
                    }
                }

                Label {
                    id: selectedLabel
                    visible: ! sharedState.actionInProgress
                    anchors.centerIn: parent
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: Theme.fontSizeExtraLarge
                    text: qsTr("%1 selected").arg(contentlist.model.selected)
                }

                Label {
                    anchors.top: selectedLabel.bottom
                    visible: selectedLabel.visible
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: qsTr("Pull up for actions")
                }

                BusyIndicator {
                    id: busyIndicator
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    running: sharedState.actionInProgress
                    size: BusyIndicatorSize.Large

                }

                Label {
                    visible: busyIndicator.running
                    anchors.centerIn: busyIndicator
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: sharedState.actionName
                }

            }

            Item {
                visible: ! sharedState.actionInProgress
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.itemSizeSmall

                Rectangle {
                    anchors.fill: parent
                    color: Theme.highlightColor
                    opacity: 0.1
                }

                Row {
                    anchors.centerIn: parent

                    Button {
                        text: "All"

                        onClicked: {
                            contentlist.model.selectAll();
                        }
                    }

                    Button {
                        text: "None"

                        onClicked: {
                            contentlist.model.unselectAll();
                        }
                    }
                }
            }
        }

        SilicaListView {
            id: contentlist

            anchors.fill: parent

            header: Column {
                width: contentlist.width

                PageHeader {
                    title: contentlist.model.name
                }

                ListItem {
                    visible: contentlist.model.isWritable
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        id: newFolderIcon
                        x: Theme.paddingMedium
                        width: height
                        height: parent.height
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

                        dlg.accepted.connect(closure(contentlist.model, dlg));
                    }
                }

                Separator {
                    visible: contentlist.model.isWritable
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    color: Theme.highlightColor
                }

            }

            PullDownMenu {

                MenuItem {
                    visible: breadcrumbRepeater.count === 0
                    text: "About"

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("AboutPage.qml"));
                    }
                }

                MenuItem {
                    visible: breadcrumbRepeater.count === 0
                    text: "Help"

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("HelpPage.qml"));
                    }
                }

                // Menu of breadcrumbs
                Repeater {
                    id: breadcrumbRepeater
                    model: collectBreadcrumbs(contentlist.model.breadcrumbs)

                    MenuItem {
                        text: modelData.name

                        onClicked: {
                            if (modelData.model !== contentlist.model)
                            {
                                popModels(modelData.model);
                            }

                            console.log("up " + modelData.level);
                            _selectionMode = false;
                            modelData.model.cdUp(modelData.level);
                        }
                    }
                }
            }

            delegate: FileDelegate {

                fileInfo: FileInfo {
                    source: model
                    sourceModel: contentlist.model
                }

                selected: model.selected
                height: Theme.itemSizeSmall

                onClicked: {
                    if (! page._selectionMode)
                    {
                        if (model.modelTarget)
                        {
                            pushModel(model.modelTarget, model.linkTarget);
                        }
                        else if (model.type === FolderBase.File || model.type === FolderBase.FileLink)
                        {
                            var props = {
                                "fileInfo": fileInfo
                            }
                            var dlg = pageStack.push(Qt.resolvedUrl("FileInfoDialog.qml"),
                                                     props);

                            function closure(model, name)
                            {
                                return function()
                                {
                                    model.open(name);
                                }
                            }

                            dlg.accepted.connect(closure(contentlist.model, model.name));
                        } else {
                            contentlist.model.open(model.name);
                        }
                    }
                    else
                    {
                        contentlist.model.setSelected(index, ! selected);
                    }
                }

                onPressAndHold: {
                    if (! page._selectionMode)
                    {
                        contentlist.model.setSelected(index, true);
                        page._selectionMode = true;
                    }
                    else
                    {
                        page._selectionMode = false;
                        contentlist.model.unselectAll();
                    }
                }
            }

            ViewPlaceholder {
                enabled: ! contentlist.model.isReadable
                text: "You have no permission for this folder"
            }

            ViewPlaceholder {
                enabled: contentlist.model.count === 0 && contentlist.model.isReadable
                text: "No files"
            }

            ScrollDecorator { }

        }//SilicaListView

        FancyScroller {
            flickable: contentlist
        }
    }//Drawer
}

