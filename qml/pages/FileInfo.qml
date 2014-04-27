import QtQuick 2.0
import harbour.cargodock 1.0

QtObject {
    property variant source
    property variant sourceModel

    property bool canOpen: source.capabilities & FolderBase.CanOpen

    property string name: source.name
    property string friendlyName: source.friendlyName
    property string path: source.path
    property string uri: source.uri
    property string linkTarget: source.linkTarget
    property int type: source.type
    property string mimeType: source.mimeType
    property string icon: source.icon
    property string preview: source.preview !== undefined ? source.preview : "PreviewGeneric"
    property int size: source.size
    property date mtime: source.mtime
    property string owner: source.owner
    property string group: source.group
    property int permissions: source.permissions
    property int capabilities: source.capabilities

    function setPermissions(value)
    {
        if (value !== permissions)
        {
            console.log("new permissions: " + value);
            sourceModel.setPermissions(name, value);
        }
    }

    function rename(newName)
    {
        sourceModel.rename(name, newName);
    }

    function readFile()
    {
        return sourceModel.readFile(name);
    }
}
