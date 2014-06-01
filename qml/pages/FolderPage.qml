import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {
    id: page

    allowedOrientations: Orientation.Landscape | Orientation.Portrait

    property bool isSecondPane

    property var _modelStack: []
    property var _modelIconStack: []
    property string _currentModelIcon

    property variant sourceModel
    property variant destinationModel

    property bool _selectionMode: false

    property int _capabilities: sourceModel ? sourceModel.capabilities : 0
    property int _destinationCapabilities: destinationModel ? destinationModel.capabilities : 0

    signal modelChanged

    signal copyCommand(variant sourceModel, variant destModel)
    signal deleteCommand(variant sourceModel, variant items)
    signal linkCommand(variant sourceModel, variant destModel)
    signal finished()
    signal error(string details)

    function pushModel(typeName, uid, icon)
    {
        if (serviceObject(typeName).usesEncryption)
        {
            if (_modelStack[0].useEncryptionPassphrase &&
                _modelStack[0].checkEncryptionPassphrase(""))
            {
                var props = {
                    "mode": "ask",
                    "passphraseChecker": _modelStack[0]
                };

                var dlg = pageStack.push(Qt.resolvedUrl("PassphraseDialog.qml"),
                                         props);
                dlg.passphraseAccepted.connect(function(passphrase) {
                    console.log("setting passphrase " + passphrase);
                    _modelStack[0].setEncryptionPassphrase(passphrase);
                    pushModel(typeName, uid, icon);
                });
                return;
            }
        }

        var props = {
            "uid": uid
        };
        var model;
        console.log("create model " + typeName + " " + uid);
        model = serviceObject(typeName).createModel(uid);
        model.progress.connect(function(n, p) { sharedState.actionProgress = p; sharedState.actionTarget = n; });
        model.finished.connect(page.finished);
        model.error.connect(page.error);

        _modelIconStack.push(icon);
        _currentModelIcon = _modelIconStack[_modelIconStack.length - 1];
        _modelStack.push(model);
        sourceModel = model;
        sharedState.currentContentModel = sourceModel;
        page.modelChanged();
    }

    function popModels(model)
    {
        while (_modelStack.length > 0 && _modelStack[_modelStack.length - 1] !== model)
        {
            _modelStack.pop();
            _modelIconStack.pop();
            _currentModelIcon = _modelIconStack[_modelIconStack.length - 1];
        }
        sourceModel = _modelStack[_modelStack.length - 1];
        sharedState.currentContentModel = sourceModel;

        // special treatment for the places model
        if (_modelStack.length === 1)
        {
            sourceModel.refresh();
        }
        page.modelChanged();
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

    function availableActions(sourceCapabilities, destCapabilities)
    {
        var actions = [];
        if (copyAction.enabled) actions.push(copyAction);
        if (bookmarkAction.enabled) actions.push(bookmarkAction);
        if (linkAction.enabled) actions.push(linkAction);
        if (deleteAction.enabled) actions.push(deleteAction);

        return actions;
    }

    onStatusChanged: {
        if (status === PageStatus.Active)
        {
            sharedState.currentContentModel = sourceModel;
            sharedState.isSecondPane = isSecondPane;
        }
    }

    Component.onCompleted: {
        pushModel("places", "places", "");
    }

    RemorsePopup {
        id: remorse
    }

    QtObject {
        id: copyAction
        property string name: "Copy to other side"
        property bool enabled: sourceModel.capabilities & FolderBase.CanCopy &&
                               destinationModel.capabilities & FolderBase.AcceptCopy

        function action()
        {
            page.copyCommand(sourceModel, destinationModel);
            _selectionMode = false;
        }
    }

    QtObject {
        id: bookmarkAction
        property string name: "Bookmark"
        property bool enabled: sourceModel.capabilities & FolderBase.CanBookmark &&
                               destinationModel.capabilities & FolderBase.AcceptBookmark

        function action()
        {
            page.linkCommand(sourceModel, destinationModel);
            _selectionMode = false;
        }
    }

    QtObject {
        id: linkAction
        property string name: "Link to other side"
        property bool enabled: sourceModel.capabilities & FolderBase.CanLink &&
                               destinationModel.capabilities & FolderBase.AcceptLink

        function action()
        {
            page.linkCommand(sourceModel, destinationModel);
            _selectionMode = false;
        }
    }

    QtObject {
        id: deleteAction
        property string name: "Delete"
        property bool enabled: sourceModel.capabilities & FolderBase.CanDelete

        function action()
        {
            var text = qsTr("Deleting %1 items").arg(sourceModel.selected);

            function closure(model, items)
            {
                return function() {
                    console.log("deleting " + items.length + " items " + items);
                    page.deleteCommand(model, items);
                }
            }

            remorse.execute(text,
                            closure(sourceModel,
                                    sourceModel.selection.slice()));
            _selectionMode = false;
        }
    }

    Rectangle {
        visible: developerMode.isRoot
        anchors.fill: parent
        color: Qt.rgba(1, 0, 0, 0.4)

        Label {
            anchors.centerIn: parent
            rotation: -66
            color: Qt.rgba(0.3, 0, 0, 0.2)
            font.pixelSize: Theme.fontSizeExtraLarge * 4
            font.weight: Font.Bold
            text: "ROOT"
        }
    }

    Column {
        x: isSecondPane ? 0
                        : parent.width - childrenRect.width
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.paddingMedium

        Repeater {
            model: Math.ceil(page.height / (Theme.paddingMedium + 10))

            Image {
                source: isSecondPane ? Qt.resolvedUrl("../left-indicator.png")
                                     : Qt.resolvedUrl("../right-indicator.png")
            }
        }
    }

    Drawer {
        anchors.fill: parent
        dock: Dock.Bottom
        open: _selectionMode || sharedState.actionInProgress
        backgroundSize: drawerView.contentHeight

        background: SilicaFlickable {
            id: drawerView
            anchors.fill: parent
            contentHeight: _selectionMode ? 340 : Theme.itemSizeSmall
            clip: true

            PushUpMenu {
                visible: actionMenuRepeater.count > 0 &&
                         ! sharedState.actionInProgress

                Repeater {
                    id: actionMenuRepeater
                    model: availableActions(sourceModel.capabilities,
                                            destinationModel.capabilities)

                    MenuItem {
                        text: modelData.name
                        onClicked: {
                            modelData.action();
                        }
                    }
                }
            }

            Item {
                visible: _selectionMode
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
                        sourceModel.unselectAll();
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
                    text: qsTr("%1 selected").arg(sourceModel ? sourceModel.selected : 0)
                }

                Label {
                    anchors.top: selectedLabel.bottom
                    visible: selectedLabel.visible
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: (sourceModel && sourceModel.selected > 0) ? qsTr("Pull up for actions")
                                                                    : qsTr("Select some items")
                }
            }

            Item {
                visible: ! sharedState.actionInProgress && _selectionMode
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
                            sourceModel.selectAll();
                        }
                    }

                    Button {
                        text: "None"

                        onClicked: {
                            sourceModel.unselectAll();
                        }
                    }
                }
            }

            Label {
                visible: sharedState.actionInProgress
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: progressBar.top
                anchors.topMargin: -2
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: sharedState.actionName
            }

            ProgressBar {
                id: progressBar
                visible: sharedState.actionInProgress
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: Theme.paddingSmall
                anchors.rightMargin: Theme.paddingSmall
                anchors.bottomMargin: 2

                minimumValue: 0
                maximumValue: 1000
                value: Math.floor(sharedState.actionProgress * 1000)
                label: sharedState.actionTarget
            }
        }

        SilicaListView {
            id: contentlist

            anchors.fill: parent
            model: sourceModel.isValid ? sourceModel : null

            header: Column {
                width: contentlist.width

                Item {
                    width: parent.width
                    height: Theme.itemSizeLarge

                    PageHeader {
                        width: parent.width - headerIcon.width - Theme.paddingLarge
                        title: sourceModel ? sourceModel.name : ""
                    }

                    Image {
                        id: headerIcon
                        width: height
                        height: Theme.fontSizeLarge
                        anchors.right: parent.right
                        anchors.rightMargin: Theme.paddingLarge
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                        source: _currentModelIcon !== "" ? _currentModelIcon
                                                         : "image://theme/icon-m-folder"
                    }
                }

                ListItem {
                    visible: sourceModel ? (sourceModel.isValid &&
                                            sourceModel.isWritable &&
                                            ! sourceModel.loading)
                                         : false
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

                        dlg.accepted.connect(closure(sourceModel, dlg));
                    }
                }

                Separator {
                    visible: sourceModel ? sourceModel.isWritable : false
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    color: Theme.highlightColor
                }

            }

            PullDownMenu {
                id: menu

                MenuItem {
                    visible: breadcrumbRepeater.count === 0
                    text: "About"

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("AboutPage.qml"));
                    }
                }

                MenuItem {
                    visible: breadcrumbRepeater.count === 0
                    text: "Settings"

                    onClicked: {
                        var props = {
                            "placesModel": _modelStack[0]
                        }

                        pageStack.push(Qt.resolvedUrl("SettingsPage.qml"), props);
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
                    model: sourceModel ? collectBreadcrumbs(sourceModel.breadcrumbs) : null

                    MenuItem {
                        id: pathMenuItem

                        property bool _chosen

                        text: modelData.name

                        Connections {
                            target: menu
                            onActiveChanged: {
                                // run the menu item action only once the menu
                                // has closed to make it smooth
                                if (! menu.active && pathMenuItem._chosen)
                                {
                                    pathMenuItem._chosen = false;

                                    if (modelData.model !== sourceModel)
                                    {
                                        popModels(modelData.model);
                                    }

                                    console.log("up " + modelData.level);
                                    _selectionMode = false;
                                    modelData.model.cdUp(modelData.level);
                                }
                            }
                        }

                        onClicked: { _chosen = true; }
                    }
                }
            }

            section.property: "section"
            section.delegate: SectionHeader {
                text: section
            }


            delegate: FileDelegate {

                fileInfo: FileInfo {
                    source: model
                    sourceModel: page.sourceModel
                }

                selected: page._selectionMode && model.selected
                height: Theme.itemSizeSmall
                opacity: (_selectionMode && ! selectable) ? 0.3 : 1

                onClicked: {
                    if (! page._selectionMode)
                    {
                        if (model.modelTarget)
                        {
                            pushModel(model.modelTarget, model.linkTarget,
                                      fileInfo.icon);
                        }
                        else if (model.type === FolderBase.File || model.type === FolderBase.FileLink)
                        {
                            var props = {
                                "fileInfo": fileInfo
                            }
                            var dlg = pageStack.push(Qt.resolvedUrl("FileInfoDialog.qml"),
                                                     props);

                            if (fileInfo.canOpen)
                            {
                                function closure(model, name)
                                {
                                    return function()
                                    {
                                        model.open(name);
                                    }
                                }
                                dlg.accepted.connect(closure(sourceModel, model.name));
                            }
                        } else {
                            sourceModel.open(model.name);
                        }
                    }
                    else if (! sharedState.actionInProgress && selectable)
                    {
                        sourceModel.setSelected(index, ! selected);
                    }
                }

                onPressAndHold: {
                    if (! page._selectionMode)
                    {
                        if (selectable)
                        {
                            sourceModel.setSelected(index, true);
                        }
                        page._selectionMode = true;
                    }
                    else
                    {
                        page._selectionMode = false;
                        sourceModel.unselectAll();
                    }
                }
            }

            ViewPlaceholder {
                enabled: sourceModel ? ! sourceModel.isValid : true
                text: "Not available"
            }

            ViewPlaceholder {
                enabled: sourceModel ? ! sourceModel.isReadable : false
                text: "You have no permission for this folder"
            }

            ViewPlaceholder {
                enabled: sourceModel ? (sourceModel.count === 0 && sourceModel.isReadable) : false
                text: "No files"
            }

            VerticalScrollDecorator { id: scrollDecorator }

        }//SilicaListView

        FancyScroller {
            visible: contentlist.quickScrollEnabled !== true && ! _selectionMode
            flickable: contentlist
        }
    }//Drawer

    BusyIndicator {
        running: sourceModel ? sourceModel.loading : true
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
    }
}

