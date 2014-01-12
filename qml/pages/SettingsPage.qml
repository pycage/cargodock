import QtQuick 2.0
import Sailfish.Silica 1.0

Page {

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.childrenRect.height

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: "Settings"
            }

            Row {
                width: parent.width

                Switch {

                }

                Label {
                    text: "Use Dropbox"
                }
            }

            TextInput {

            }
        }

    }

}
