import QtQuick 2.0
import Sailfish.Silica 1.0

Page {

    property variant placesModel

    function userServices(services)
    {
        var ret = [];
        for (var i = 0; i < services.length; ++i)
        {
            var data = placesModel.service(services[i]);
            var obj = serviceObject(data.type);
            if (obj.serviceDelegate)
            {
                ret.push(data);
            }
        }
        return ret;
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.childrenRect.height

        PullDownMenu {
            MenuItem {
                text: "Add Service"

                onClicked: {
                    var props = {
                        "placesModel": placesModel
                    };
                    pageStack.push(Qt.resolvedUrl("AddServiceDialog.qml"),
                                   props);
                }
            }
        }

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: "Settings"
            }

            SectionHeader {
                text: "Services"
            }

            Repeater {
                id: serviceListView

                model: userServices(placesModel.services)

                delegate: ListItem {
                    id: serviceItem
                    property variant serviceObj: serviceObject(modelData.type);

                    visible: serviceObj.serviceDelegate
                    width: column.width
                    menu: contextMenu

                    function remove()
                    {
                        function closure(placesModel, uid, refreshPanes)
                        {
                            return function()
                            {
                                placesModel.removeService(uid);
                                refreshPanes();
                            }
                        }

                        remorseAction("Deleting", closure(placesModel,
                                                          modelData.uid,
                                                          refreshPanes));
                    }

                    ServiceDelegate {
                        anchors.fill: parent
                        iconSource: serviceObj.icon
                        title: serviceObj.name
                        subtitle: modelData.name
                        highlighted: serviceItem.highlighted
                    }

                    onClicked: {
                        function closure(placesModel, uid, refreshPanes)
                        {
                            return function(serviceName, icon, properties)
                            {
                                placesModel.updateService(uid, properties);
                                refreshPanes();
                            }
                        }

                        var props = placesModel.serviceProperties(modelData.uid);
                        var dlg = pageStack.push(serviceObj.serviceConfigurator,
                                                 { "properties": props });
                        dlg.serviceConfigured.connect(closure(placesModel,
                                                              modelData.uid,
                                                              refreshPanes));
                    }

                    Component {
                        id: contextMenu
                        ContextMenu {
                            MenuItem {
                                text: "Remove"

                                onClicked: {
                                    remove();
                                }
                            }
                        }
                    }
                }
            }

            Label {
                visible: serviceListView.count === 0
                width: parent.width
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: "No services yet.\n" +
                      "Drag down to add a service."
            }

        }

    }

}
