import QtQuick 2.0

QtObject {
    property variant source

    property string name: source ? source.name : ""
    property string path: source ? source.path : ""
    property string uri: source ? source.uri : ""
    property string linkTarget: source ? source.linkTarget : ""
    property int type: source ? source.type : 0
    property string mimeType: source ? source.mimeType : "application/x-octet-stream"
    property string icon: source ? source.icon : "image://theme/icon-m-other"
    property int size: source ? source.size : 0
    property date mtime: source ? source.mtime : Date()
    property string owner
    property string group
    property int permissions
}
