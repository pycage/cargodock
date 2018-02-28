import QtQuick 2.0
import Sailfish.Silica 1.0

import "../services"

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
                text: qsTr("Add Service")

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
                title: qsTr("Settings")
            }

            SectionHeader {
                text: qsTr("Security")
            }

            TextSwitch {
                automaticCheck: false

                text: qsTr("Secure password storage")
                description: qsTr("Enable to set a custom passphrase to encrypt your " +
                             "passwords stored in the configuration.\n" +
                             "Your passphrase is not stored and will be asked for " +
                             "once per session.\n" +
                             "If not enabled, passwords are encrypted with a default " +
                             "passphrase.")
                checked: placesModel.useEncryptionPassphrase

                onClicked: {
                    var props;
                    var accepter;
                    var rejecter;

                    if (! checked)
                    {
                        props = {
                            "mode": "new",
                            "passphraseChecker": placesModel
                        };

                        accepter = function(passphrase) {
                            placesModel.changeEncryptionPassphrase(passphrase);
                            placesModel.useEncryptionPassphrase = true;
                            checked = true;
                        };

                        rejecter = function() {
                            checked = false;
                        };
                    }
                    else
                    {
                        props = {
                            "mode": "confirm",
                            "passphraseChecker": placesModel
                        };

                        accepter = function(passphrase) {
                            placesModel.changeEncryptionPassphrase("");
                            placesModel.useEncryptionPassphrase = false;
                            checked = false;
                        };

                        rejecter = function() {
                            checked = true;
                        };
                    }

                    var dlg = pageStack.push(Qt.resolvedUrl("PassphraseDialog.qml"),
                                             props);
                    dlg.passphraseAccepted.connect(accepter);
                    dlg.rejected.connect(rejecter);
                }

            }

            SectionHeader {
                text: qsTr("Services")
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

                        remorseAction(qsTr("Deleting"), closure(placesModel,
                                                          modelData.uid,
                                                          refreshPanes));
                    }

                    function configureService()
                    {
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

                    ServiceDelegate {
                        anchors.fill: parent
                        iconSource: serviceObj.icon
                        title: serviceObj.name
                        subtitle: modelData.name
                        highlighted: serviceItem.highlighted
                    }

                    onClicked: {
                        passphraseGuard.run(serviceObj,
                                            placesModel,
                                            configureService);
                    }

                    Component {
                        id: contextMenu
                        ContextMenu {
                            MenuItem {
                                text: qsTr("Remove")

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
                text: qsTr("No services yet.\n" +
                      "Drag down to add a service.")
            }

        }

        ScrollDecorator { }

    }

}
