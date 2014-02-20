import QtQuick 2.0
import harbour.cargodock 1.0

QtObject {
    property variant source
    property variant sourceModel

    property bool canOpen: sourceModel.capabilities & FolderBase.CanOpen

    property string name: source.name ? source.name : ""
    property string path: source.path ? source.path : ""
    property string uri: source.uri ? source.uri : ""
    property string linkTarget: source.linkTarget ? source.linkTarget : ""
    property int type: source.type ? source.type : 0
    property string mimeType: source.mimeType ? source.mimeType : "application/x-octet-stream"
    property string icon: source.icon !== undefined ? source.icon : "image://theme/icon-m-other"
    property string preview: source.preview !== undefined ? source.preview : "image://theme/icon-m-other"
    property int size: source.size ? source.size : 0
    property date mtime: source.mtime
    property string owner: source.owner ? source.owner : "?"
    property string group: source.group ? source.group : "?"
    property int permissions: source.permissions ? source.permissions : 0

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
