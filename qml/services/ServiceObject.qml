import QtQuick 2.0
import Sailfish.Silica 1.0

/* Element representing a service. Derive new services from this element.
 */
QtObject {
    id: service

    /* The name under which this service will be registered. This is an internal
     * ID and not shown to the user.
     */
    property string serviceName

    /* The name under which the service is shown to the user when instantiated.
     */
    property string name

    /* The icon to be used for instances of this service.
     */
    property string icon: "image://theme/icon-m-folder"

    /* Whether this service uses encryption. Accessing a service with encryption
     * may require the user to enter a pass phrase first.
     */
    property bool usesEncryption: false

    /* The implementation of the service model.
     */
    property Component serviceModel

    /* A list delegate for representing the service in list views.
     * Leave undefined if the service is not to appear in lists.
     */
    property Component serviceDelegate: null

    /* A dialog for configuring the service, if needed. By default, only the
     * name is asked.
     * Must emit the signal serviceConfigured(serviceName,
     *                                        icon,
     *                                        name,
     *                                        properties).
     */
    property Component serviceConfigurator


    /* Creates a new model instance with the given UID.
     */
    function createModel(uid)
    {
        var props = {
            "uid": uid
        };
        return serviceModel.createObject(service, props);
    }

    Component.onCompleted: {
        registerServiceObject(service);
    }

    serviceConfigurator: Component {
        Dialog {
            allowedOrientations: Orientation.All

            signal serviceConfigured(string serviceName,
                                     string icon,
                                     variant properties)

            property alias name: textEntry.text
            property var properties

            canAccept: textEntry.text !== ""

            SilicaFlickable {
                anchors.fill: parent

                Column {
                    width: parent.width

                    DialogHeader {
                        title: "Configure service"
                    }

                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }

                    TextField {
                        id: textEntry

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.rightMargin: Theme.paddingLarge

                        placeholderText: "Name"
                        focus: true
                    }

                }
            }

            onAccepted: {
                serviceConfigured(service.serviceName,
                                  service.icon,
                                  textEntry.text,
                                  { });
            }

        }//Dialog
    }
}
