import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

Page {
    id: dialog

    allowedOrientations: Orientation.All

    property FileInfo fileInfo

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            enabled: fileInfo.canOpen

            MenuItem {
                text: "Open"
                onClicked: {
                    fileInfo.open()
                }
            }
        }

        Column {
            id: column
            width: parent.width
            height: childrenRect.height

            PageHeader {
                title: "File information"
            }

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

                EnterKey.text: "Rename"
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
                key: "Type"
                value: fileInfo.mimeType
            }

            KeyValue {
                key: "Path"
                value: fileInfo.path
            }

            KeyValue {
                key: "Size"
                value: Format.formatFileSize(fileInfo.size)
            }

            KeyValue {
                key: "Last modified"
                value: Format.formatDate(fileInfo.mtime, Formatter.DurationElapsed)
            }

            KeyValue {
                key: "Modification time"
                value: Format.formatDate(fileInfo.mtime, Formatter.TimePoint)
            }

            KeyValue {
                visible: fileInfo.owner !== ""
                key: "Owner"
                value: fileInfo.owner
            }

            KeyValue {
                visible: fileInfo.group !== ""
                key: "Group"
                value: fileInfo.group
            }

            Column {
                visible: developerMode.enabled &&
                         fileInfo.capabilities & FolderBase.HasPermissions
                width: parent.width

                SectionHeader {
                    text: "Permissions"
                }

                Repeater {
                    model: [
                        ["Readable", FolderBase.ReadOwner],
                        ["Writable", FolderBase.WriteOwner],
                        ["Executable", FolderBase.ExecOwner]
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
                    text: "Group permissions"
                }

                Repeater {
                    model: [
                        ["Readable", FolderBase.ReadGroup],
                        ["Writable", FolderBase.WriteGroup],
                        ["Executable", FolderBase.ExecGroup]
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
                    text: "World permissions"
                }

                Repeater {
                    model:  [
                        ["Readable", FolderBase.ReadOther],
                        ["Writable", FolderBase.WriteOther],
                        ["Executable", FolderBase.ExecOther]
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
