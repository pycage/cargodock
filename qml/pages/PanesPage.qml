import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property ListModel trails

    signal opened(var model)
    signal newPane
    signal removePane(int idx)

    SilicaGridView {
        id: gridView
        anchors.fill: parent

        cellWidth: width / 3
        cellHeight: Theme.itemSizeLarge

        model: page.trails

        PullDownMenu {
            MenuItem {
                text: "New pane"

                onClicked: {
                    page.newPane();
                }
            }
        }

        delegate: Item {
            width: gridView.cellWidth
            height: gridView.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: Theme.paddingSmall
                radius: 5
                color: Qt.darker(Theme.primaryColor, 3)

                Image {
                    id: icon
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: Theme.paddingSmall
                    source: trailModel.fsIcon
                }

                Label {
                    anchors.left: icon.right
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: Theme.paddingSmall
                    elide: Text.ElideRight
                    text: trailModel.fsModel.name
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        page.opened(trailModel);
                    }
                }
            }

            IconButton {
                visible: gridView.count > 1
                anchors.top: parent.top
                anchors.right: parent.right
                icon.source: "image://theme/icon-m-close"

                onClicked: {
                    page.removePane(index);
                }
            }

        }
    }

}
