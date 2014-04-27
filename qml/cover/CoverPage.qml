import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {

    property variant coverModel: sharedState.currentContentModel

    Rectangle {
        id: dot
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Theme.paddingMedium
        width: Theme.paddingMedium / 2
        height: width
        radius: width / 2
        color: Theme.primaryColor
        opacity: sharedState.isSecondPane ? 0.2 : 1
    }

    Rectangle {
        anchors.top: dot.top
        anchors.left: dot.right
        anchors.leftMargin: width / 2
        width: Theme.paddingMedium / 2
        height: width
        radius: width / 2
        color: Theme.primaryColor
        opacity: sharedState.isSecondPane ? 1 : 0.2
    }

    Image {
        id: folderIcon
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Theme.paddingLarge
        height: Theme.fontSizeMedium
        width: height
        source: "image://theme/icon-m-folder"
    }

    Label {
        id: titleLabel
        anchors.baseline: folderIcon.bottom
        anchors.left: folderIcon.right
        anchors.right: parent.right
        anchors.topMargin: Theme.paddingLarge
        anchors.leftMargin: Theme.paddingSmall
        anchors.rightMargin: Theme.paddingLarge
        truncationMode: TruncationMode.Fade
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.primaryColor
        text: coverModel.name
    }

    ListView {
        id: listView
        anchors.top: titleLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: Theme.paddingSmall
        anchors.leftMargin: Theme.paddingLarge
        anchors.rightMargin: Theme.paddingLarge
        anchors.bottomMargin: Theme.paddingLarge
        model: coverModel

        delegate: ListItem {
            height: Theme.fontSizeSmall
            Label {
                width: parent.width
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: friendlyName !== "" ? friendlyName : name
            }
        }
    }

    CoverActionList {
        enabled: ! sharedState.isSecondPane

        CoverAction {
            iconSource: "image://theme/icon-cover-next"

            onTriggered: {
                pageStack.navigateForward(PageStackAction.Immediate);
            }
        }
    }

    CoverActionList {
        enabled: sharedState.isSecondPane

        CoverAction {
            iconSource: "image://theme/icon-cover-previous"

            onTriggered: {
                pageStack.navigateBack(PageStackAction.Immediate);
            }
        }
    }
}


