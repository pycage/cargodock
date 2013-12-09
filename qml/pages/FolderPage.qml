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
    signal linkCommand(variant sourceModel)

    function currentContentModel()
    {
        return folderModel;
    }

    FolderModel {
        id: folderModel
        path: "/home/nemo"
    }

    ListModel {
        id: bookMarksModel

        ListElement {
            name: "Documents"
            type: FolderModel.Folder
            path: "/home/nemo/Documents"
        }

        ListElement {
            name: "Downloads"
            type: FolderModel.Folder
            path: "/home/nemo/Downloads"
        }

        ListElement {
            name: "Music"
            type: FolderModel.Folder
            path: "/home/nemo/Music"
        }

        ListElement {
            name: "Videos"
            type: FolderModel.Folder
            path: "/home/nemo/Videos"
        }

        ListElement {
            name: "Pictures"
            type: FolderModel.Folder
            path: "/home/nemo/Pictures"
        }

        ListElement {
            name: "Camera"
            type: FolderModel.Folder
            path: "/home/nemo/Pictures/Camera"
        }
    }

    Rectangle {
        visible: _selectionMode
        anchors.fill: parent
        color: Theme.highlightColor
        opacity: 0.3
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
            }

            MenuItem {
                text: "Settings"
            }
        }

        delegate: ListItem {
            Rectangle {
                id: placeIcon
                width: height
                height: parent.height
                color: type === FolderModel.Folder ? "red" : "blue"
            }

            Label {
                anchors.left: placeIcon.right
                text: name
            }

            onClicked: {
                contentlist.model.path = path;
                bookmarksList.visible = false;
            }
        }
    }

    SilicaListView {
        id: contentlist

        visible: ! bookmarksList.visible
        anchors.fill: parent

        // improve performance when switching pages
        model: page.status === PageStatus.Active ? folderModel : null

        header: PageHeader {
            title: folderModel.name
        }

        PullDownMenu {
            MenuItem {
                visible: _selectionMode
                text: "Delete"
            }
            MenuItem {
                visible: _selectionMode
                text: "Copy to other side"

                onClicked: {
                    page.copyCommand(folderModel);
                }
            }
            MenuItem {
                visible: _selectionMode
                text: "Link to other side"

                onClicked: {
                    page.linkCommand(folderModel);
                }
            }

            MenuItem {
                visible: ! _selectionMode
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

        delegate: ListItem {
            showMenuOnPressAndHold: false

            Rectangle {
                anchors.fill: parent
                color: selected ? Theme.highlightColor : "transparent"
                opacity: 0.5
            }

            Rectangle {
                id: icon
                width: height
                height: parent.height
                color: type === FolderModel.Folder ? "red" : "blue"
            }

            Label {
                anchors.left: icon.right
                text: name
            }

            onClicked: {
                if (! page._selectionMode)
                {
                    folderModel.open(name);
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
                    for (var i = 0; i < contentlist.count; ++i)
                    {
                        folderModel.setSelected(i, false);
                    }
                }
            }
        }

        ViewPlaceholder {
            enabled: ! folderModel.isReadable
            text: "You have no permission for this folder."
        }
    }

}

