import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {
    id: dialog

    allowedOrientations: Orientation.Landscape | Orientation.Portrait

    property FileInfo fileInfo

    property variant placesModel
    function toolMenuModel()
    {
        var ret = [];
        var tools = placesModel.tools
        for (var i = 0; i < tools.length;++i){
            var data = {
                "uid": tools[i],
                "name": placesModel.getToolOptions(tools[i]).Name
            }
            ret.push(data);
        }
        return ret;
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            enabled: fileInfo.canOpen ||
                     (fileInfo.capabilities & FolderBase.HasPermissions && toolsMenu.count !==0)
            MenuItem {
                visible: fileInfo.canOpen
                text: qsTr("Open")
                onClicked: {
                    fileInfo.open()
                }
            }
            MenuLabel{
                id: toolsLabel
                text: qsTr("Tools")
                visible: fileInfo.capabilities & FolderBase.HasPermissions && toolsMenu.count !==0
            }
            Repeater{
                id: toolsMenu
                model: toolMenuModel()
                MenuItem{
                    text: modelData.name
                    onClicked: {
                        fileInfo.sourceModel.useTool(modelData.uid,fileInfo.name)
                    }
                }
            }
        }

        Column {
            id: column
            width: parent.width
            height: childrenRect.height

            PageHeader{
                title: fileInfo.type === FolderBase.FolderLink ||
                       fileInfo.type === FolderBase.Folder ?
                           qsTr("Folder"):
                           qsTr("File")
            }

//            DialogHeader {
//                title: fileInfo.canOpen ? qsTr("Open")
//                                        : qsTr("Close")
//                acceptText: fileInfo.canOpen? qsTr("Accept"):""
//                cancelText: fileInfo.canOpen? qsTr("Cancel"):""
//            }

            // preview item
            Loader {
                function previewComponent(s)
                {
                    var idx = s.indexOf('#');
                    if (idx > 0)
                    {
                        return s.substring(0, idx);
                    }
                    else
                    {
                        return s;
                    }
                }

                visible: fileInfo.preview !== ""
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                height: 240
                source: Qt.resolvedUrl(previewComponent(fileInfo.preview) + ".qml")

                onStatusChanged: {
                    if (status === Loader.Ready)
                    {
                        item.fileInfo = fileInfo;
                    }
                }
            }

            TextField {
                width: parent.width
                text: fileInfo.name
                inputMethodHints: Qt.ImhNoAutoUppercase

                EnterKey.text: qsTr("Rename")
                EnterKey.enabled: text !== ""
                EnterKey.onClicked: {
                    fileInfo.rename(text);
                    focus = false;
                }

                onFocusChanged: {
                    if (! focus)
                    {
                        text = fileInfo.name;
                    }
                }
            }

            KeyValue {
                visible: fileInfo.type === FolderBase.FolderLink ||
                         fileInfo.type === FolderBase.FileLink
                key: "→"
                value: fileInfo.linkTarget
            }

            KeyValue {
                key: qsTr("Type")
                value: fileInfo.mimeType
            }

            KeyValue {
                key: qsTr("Path")
                value: fileInfo.path
            }

            KeyValue {
                key: qsTr("Size")
                value: Format.formatFileSize(fileInfo.size)
            }

            KeyValue {
                key: qsTr("Last modified")
                value: Format.formatDate(fileInfo.mtime, Formatter.DurationElapsed)
            }

            KeyValue {
                key: qsTr("Modification time")
                value: Format.formatDate(fileInfo.mtime, Formatter.TimePoint)
            }

            KeyValue {
                visible: fileInfo.owner !== ""
                key: qsTr("Owner")
                value: fileInfo.owner
            }

            KeyValue {
                visible: fileInfo.group !== ""
                key: qsTr("Group")
                value: fileInfo.group
            }

            Column {
                visible: developerMode.enabled &&
                         fileInfo.capabilities & FolderBase.HasPermissions
                width: parent.width

                SectionHeader {
                    text: qsTr("Permissions")
                }

                Repeater {
                    model: [
                        [qsTr("Readable"), FolderBase.ReadOwner],
                        [qsTr("Writable"), FolderBase.WriteOwner],
                        [qsTr("Executable"), FolderBase.ExecOwner]
                    ]

                    KeySwitch {
                        key: modelData[0]
                        checked: fileInfo.permissions & modelData[1];

                        onCheckedChanged: {
                            fileInfo.setPermissions(
                                        checked
                                        ? fileInfo.permissions | modelData[1]
                                        : fileInfo.permissions ^ modelData[1]);
                        }
                    }
                }

                SectionHeader {
                    text: qsTr("Group permissions")
                }

                Repeater {
                    model: [
                        [qsTr("Readable"), FolderBase.ReadGroup],
                        [qsTr("Writable"), FolderBase.WriteGroup],
                        [qsTr("Executable"), FolderBase.ExecGroup]
                    ]

                    KeySwitch {
                        key: modelData[0]
                        checked: fileInfo.permissions & modelData[1];

                        onCheckedChanged: {
                            fileInfo.setPermissions(
                                        checked
                                        ? fileInfo.permissions | modelData[1]
                                        : fileInfo.permissions ^ modelData[1]);
                        }
                    }
                }

                SectionHeader {
                    text: qsTr("World permissions")
                }

                Repeater {
                    model:  [
                        [qsTr("Readable"), FolderBase.ReadOther],
                        [qsTr("Writable"), FolderBase.WriteOther],
                        [qsTr("Executable"), FolderBase.ExecOther]
                    ]

                    KeySwitch {
                        key: modelData[0]
                        checked: fileInfo.permissions & modelData[1];

                        onCheckedChanged: {
                            fileInfo.setPermissions(
                                        checked
                                        ? fileInfo.permissions | modelData[1]
                                        : fileInfo.permissions ^ modelData[1]);
                        }
                    }
                }
            }

        }

        ScrollDecorator { }
    }
}
