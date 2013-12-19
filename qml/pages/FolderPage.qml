import QtQuick 2.0
import Sailfish.Silica 1.0
import org.pycage.cargodock 1.0

Page {
    id: page

    property bool secondPane: false
    property alias path: folderModel.path
    property variant _breadcrumbs: folderModel.breadcrumbs

    property bool _selectionMode: false

    signal copyCommand(variant sourceModel)
    signal deleteCommand(variant sourceModel)
    signal linkCommand(variant sourceModel)
    signal finished()
    signal error(string details)

    function currentContentModel()
    {
        return folderModel;
    }

    /*
    onStatusChanged: {
        if (status === PageStatus.Active)
        {
            contentList.contentY = contentList.position;
        }
    }
    */

    FolderModel {
        id: folderModel
        path: "/home/nemo"

        onFinished: {
            page.finished();
        }

        onError: {
            page.error(details);
        }
    }

    ListModel {
        id: bookMarksModel

        ListElement {
            name: "Documents"
            type: FolderModel.Folder
            path: "/home/nemo/Documents"
            depth: 3
            icon: "image://theme/icon-m-document"
        }

        ListElement {
            name: "Downloads"
            type: FolderModel.Folder
            path: "/home/nemo/Downloads"
            depth: 3
            icon: "image://theme/icon-m-download"
        }

        ListElement {
            name: "Music"
            type: FolderModel.Folder
            path: "/home/nemo/Music"
            depth: 3
            icon: "image://theme/icon-m-music"
        }

        ListElement {
            name: "Videos"
            type: FolderModel.Folder
            path: "/home/nemo/Videos"
            depth: 3
            icon: "image://theme/icon-m-media"
        }

        ListElement {
            name: "Pictures"
            type: FolderModel.Folder
            path: "/home/nemo/Pictures"
            depth: 3
            icon: "image://theme/icon-m-image"
        }

        ListElement {
            name: "Camera"
            type: FolderModel.Folder
            path: "/home/nemo/Pictures/Camera"
            depth: 4
            icon: "image://theme/icon-m-camera"
        }

        ListElement {
            name: "Home"
            type: FolderModel.Folder
            path: "/home/nemo"
            depth: 2
            icon: "image://theme/icon-m-home"
        }

        ListElement {
            name: "System"
            type: FolderModel.Folder
            path: "/"
            depth: 0
            icon: "image://theme/icon-m-device"
        }
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
                    enabled: folderModel.selected > 0
                    text: "Copy to other side"

                    onClicked: {
                        page.copyCommand(folderModel);
                    }
                }
                MenuItem {
                    enabled: folderModel.selected > 0
                    text: "Link to other side"

                    onClicked: {
                        page.linkCommand(folderModel);
                    }
                }
                MenuItem {
                    enabled: folderModel.selected > 0
                    text: "Delete"

                    onClicked: {
                        var text = qsTr("Deleting %1 items")
                                   .arg(folderModel.selected);
                        remorse.execute(text, function() {
                            page.deleteCommand(folderModel);
                        });
                    }
                }
            }

            Column {
                width: parent.width

                Separator {
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    color: Theme.highlightColor
                }

                IconButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    icon.source: "image://theme/icon-m-close"

                    onClicked: {
                        folderModel.unselectAll();
                        _selectionMode = false;
                    }
                }

                Label {
                    visible: ! sharedState.actionInProgress
                    width: drawerView.width
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("%1 selected").arg(folderModel.selected)
                }

                BusyIndicator {
                    running: sharedState.actionInProgress
                    size: BusyIndicatorSize.Large
                }
            }
        }

        SilicaListView {
            id: bookmarksList

            visible: true
            anchors.fill: parent
            model: bookMarksModel

            header: PageHeader {
                title: "Places"
            }

            PullDownMenu {
                MenuItem {
                    text: "About"
                }

                MenuItem {
                    text: "Help"

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("HelpPage.qml"));
                    }
                }
            }

            delegate: FileDelegate {
                fileInfo: FileInfo {
                    name: model.name
                    type: model.type
                    icon: model.icon
                    uri: model.path
                }

                onClicked: {
                    contentlist.model.minDepth = depth;
                    contentlist.model.path = path;
                    bookmarksList.visible = false;
                }
            }
        }

        SilicaListView {
            id: contentlist

            property int position: contentY

            visible: ! bookmarksList.visible
            anchors.fill: parent

            // improve performance when switching pages
            model: folderModel //page.status === PageStatus.Active ? folderModel : null

            header: Column {
                width: contentlist.width

                PageHeader {
                    title: folderModel.name
                }

                ListItem {
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
                        dlg.accepted.connect(function() {
                            folderModel.newFolder(name);
                        });
                    }
                }

                Separator {
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    color: Theme.highlightColor
                }

            }

            PullDownMenu {
                MenuItem {
                    text: "Places"

                    onClicked: {
                        bookmarksList.visible = true;
                    }
                }

                Repeater {
                    model: _selectionMode ? 0 : _breadcrumbs.length

                    MenuItem {
                        text: _breadcrumbs[index] + "/"

                        onClicked: {
                            folderModel.cdUp(_breadcrumbs.length - index);
                        }
                    }
                }
            }

            delegate: FileDelegate {

                fileInfo: FileInfo {
                    source: model
                }

                selected: model.selected
                height: Theme.itemSizeSmall

                onClicked: {
                    if (! page._selectionMode)
                    {
                        if (model.type === FolderModel.File || model.type === FolderModel.FileLink)
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

                            dlg.accepted.connect(closure(folderModel, model.name));
                        } else {
                            folderModel.open(model.name);
                        }
                    }
                    else
                    {
                        folderModel.setSelected(index, ! selected);
                    }
                }

                onPressAndHold: {
                    if (! page._selectionMode)
                    {
                        folderModel.setSelected(index, true);
                        page._selectionMode = true;
                    }
                    else
                    {
                        page._selectionMode = false;
                        folderModel.unselectAll();
                    }
                }
            }

            ViewPlaceholder {
                enabled: ! folderModel.isReadable
                text: "You have no permission for this folder"
            }

            ViewPlaceholder {
                enabled: folderModel.count === 0 && folderModel.isReadable
                text: "No files"
            }

            ScrollDecorator { }
        }
    }
}

