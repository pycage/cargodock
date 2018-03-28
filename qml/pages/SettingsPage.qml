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

    function toolArray()
    {
        var ret = [];
        var tools = placesModel.tools
        for (var i = 0; i < tools.length;++i){
            var data = {
                "uid": tools[i],
                "options": placesModel.getToolOptions(tools[i])
            }
            ret.push(data);
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
            MenuItem{
                text: qsTr("New Tool")

                onClicked: {
                    var props = {
                        "toolData": null
                    };
                    pageStack.push(toolSettings,props);
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
                text: qsTr("Interface")
            }

            TextSwitch {
                automaticCheck: false
                text: qsTr("Enable alternative UI")
                description: qsTr("This is unfinished ui from new-ui development branch.\n"
                                  +"Restart application after changing.")
                checked: placesModel.useNewUI
                onClicked: {
                    checked = !checked
                    placesModel.useNewUI = checked
                }
            }

            SectionHeader {
                text: qsTr("Tools")
            }

            Label {
                visible: toolsListView.count === 0
                width: parent.width
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("No tools yet.\n" +
                           "Pull down to add a new tool.")
            }

            Repeater {
                id: toolsListView
                model: toolArray()
                delegate: ListItem {
                    function remove()
                    {
                        function closure(placesModel, uid, refreshPanes)
                        {
                            return function()
                            {
                                placesModel.removeTool(uid);
                                refreshPanes();
                            }
                        }

                        remorseAction(qsTr("Deleting"), closure(placesModel,
                                                                modelData.uid,
                                                                refreshPanes));
                    }
                    id: toolItem
                    menu: toolsItemMenu
                    width: column.width
                    Label {
                        id: nameLabel
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.paddingMedium
                        color: highlighted ? Theme.highlightColor : Theme.primaryColor
                        text:modelData.options.Name
                    }

                    Label {
                        anchors.left: nameLabel.left
                        anchors.top: nameLabel.bottom
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                        text: modelData.options.Command
                    }

                    onClicked: {
                        var props = {
                            "toolData":modelData
                        }
                        pageStack.push(toolSettings,props)
                    }

                    Component {
                        id: toolsItemMenu
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
                      "Pull down to add a service.")
            }

        }

        ScrollDecorator { }

    }

    Component {
        id: toolSettings
        Dialog {
            id: tooldialog
            property variant toolData
            property string uid: toolData?toolData.uid:""
            property variant optionsData: toolData?toolData.options:null
            allowedOrientations: Orientation.All
            canAccept: toolName.text!="" && toolCMD.text!=""
            Column{
                id: toolColumn
                width: parent.width
                DialogHeader {}
                TextField{
                    id: toolName
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.rightMargin: Theme.paddingLarge
                    label: qsTr("Name")
                    placeholderText: qsTr("Enter name")
                    text: optionsData?optionsData.Name:""
                }
                TextField{
                    id: toolCMD
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.rightMargin: Theme.paddingLarge
                    label: qsTr("Command line")
                    placeholderText: qsTr("Enter command to use with files")
                    text: optionsData?optionsData.Command:""
                }
                Label{
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.rightMargin: Theme.paddingLarge
                    wrapMode: Label.WordWrap
                    text: qsTr("Command should contain %1 sequence for location placement.")
                }
            }
            onAccepted: {
                var data ={
                    "Name": toolName.text,
                    "Command": toolCMD.text
                }
                if(toolData){
                     //console.debug("Update: " + uid + " " + toolName.text + " " + toolCMD.text)
                    placesModel.updateTool(uid,data)
                }else{
                    //console.debug("New: "+ toolName.text + " " + toolCMD.text)
                    placesModel.addTool(data)
                }
            }
        }
    }

}
