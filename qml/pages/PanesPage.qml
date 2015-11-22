import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property ListModel trails
    property var currentTrailModel

    signal opened(var model)
    signal newPane
    signal removePane(int idx)

    SilicaGridView {
        id: gridView
        anchors.fill: parent

        cellWidth: width
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

        header: PageHeader {
            title: qsTr("Panes")
        }

        delegate: BackgroundItem {
            width: gridView.cellWidth
            height: gridView.cellHeight

            highlighted: page.currentTrailModel === trailModel

            Image {
                id: icon
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: Theme.paddingSmall
                source: trailModel.fsIcon
            }

            Label {
                anchors.left: icon.right
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.rightMargin: Theme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                color: parent.pressed ? Theme.highlightColor : Theme.primaryColor
                text: trailModel.fsModel.path
            }

            onClicked: {
                page.opened(trailModel);
            }

            IconButton {
                visible: gridView.count > 1
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                icon.source: "image://theme/icon-m-close"

                onClicked: {
                    page.removePane(index);
                }
            }

        }

        ViewPlaceholder {
            enabled: gridView.count < 2
            text: "Pull down to add a new pane"
        }
    }

}
