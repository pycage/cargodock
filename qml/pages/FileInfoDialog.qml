import QtQuick 2.0
import Sailfish.Silica 1.0
import org.pycage.cargodock 1.0

Dialog {
    property FileInfo fileInfo

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            height: childrenRect.height

            DialogHeader {
                title: "Open"
            }

            Image {
                width: parent.width
                height: 240
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                visible: source !== ""
                source: fileInfo.mimeType.substring(0, 6) === "image/" ? fileInfo.uri
                                                                       : ""
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
                key: "Owner"
                value: fileInfo.owner
            }

            KeyValue {
                key: "Group"
                value: fileInfo.group
            }

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
                model: [
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

        ScrollDecorator { }
    }
}
