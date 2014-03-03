import QtQuick 2.0
import harbour.cargodock 1.0

ServiceObject {
    name: "WebDAV"
    serviceName: "webdav"
    serviceModel: FolderModel { }

    serviceDelegate: ServiceDelegate {
        title: qsTr("WebDAV")
        subtitle: qsTr("Access a WebDAV share")
    }
}
