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
            title: "Add service"
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

            function addService()
            {
                function closure(placesModel, refreshPanes)
                {
                    return function(serviceName, icon, properties)
                    {
                        placesModel.addService(serviceName, icon, properties);
                        refreshPanes();
                    }
                }

                var obj = serviceObject(modelData);
                var dlg = pageStack.replace(obj.serviceConfigurator);
                dlg.serviceConfigured.connect(closure(placesModel, refreshPanes));
            }

            onClicked: {
                passphraseGuard.run(serviceObj,
                                    placesModel,
                                    addService);
            }
        }
    }
}
