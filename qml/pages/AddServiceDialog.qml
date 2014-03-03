import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property variant placesModel

    SilicaListView {
        id: listview

        anchors.fill: parent

        model: serviceNames

        header: PageHeader {
            title: qsTr("Add service")
        }

        delegate: ListItem {
            property variant serviceObj: serviceObject(modelData)

            visible: serviceObj.serviceDelegate
            width: listview.width

            onHighlightedChanged: {
                if (loader.item)
                {
                    loader.item.highlighted = highlighted;
                }
            }

            Loader {
                id: loader
                anchors.fill: parent
                sourceComponent: serviceObj.serviceDelegate
            }

            onClicked: {
                function closure(placesModel, refreshPanes)
                {
                    return function(serviceName, icon, name, properties)
                    {
                        placesModel.addService(serviceName, icon, name, properties);
                        refreshPanes();
                    }
                }

                var obj = serviceObject(modelData);
                var dlg = pageStack.replace(obj.serviceConfigurator);
                dlg.serviceConfigured.connect(closure(placesModel, refreshPanes));
            }
        }
    }
}
