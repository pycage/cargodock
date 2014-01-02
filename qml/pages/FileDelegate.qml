import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.cargodock 1.0

/* Delegate item representing a file or directory.
 */
ListItem {

    property FileInfo fileInfo
    property bool selected

    showMenuOnPressAndHold: false

    highlighted: down || menuOpen || selected

    Image {
        id: iconImage
        x: Theme.paddingMedium
        width: height
        height: Math.max(parent.height, 10)
        asynchronous: true
        sourceSize.width: fileInfo.icon !== "" ? undefined : width
        sourceSize.height: fileInfo.icon !== "" ? undefined : height
        fillMode: fileInfo.icon !== ""
                  ? Image.Pad
                  : Image.PreserveAspectCrop
        smooth: fileInfo.icon !== "" ? false : true
        source: fileInfo.icon !== "" ? fileInfo.icon : fileInfo.uri
        clip: true
    }

    Label {
        id: nameLabel
        anchors.left: iconImage.right
        anchors.right: parent.right
        anchors.leftMargin: Theme.paddingMedium
        anchors.rightMargin: Theme.paddingMedium
        truncationMode: TruncationMode.Fade
        font.pixelSize: Theme.fontSizeMedium
        color: (fileInfo.type === FolderModel.FolderLink || fileInfo.type === FolderModel.FileLink) ? Theme.highlightColor
                                                                                  : Theme.primaryColor
        text: fileInfo.name
    }

    Label {
        anchors.top: nameLabel.bottom
        anchors.left: nameLabel.left
        anchors.right: nameLabel.right
        truncationMode: TruncationMode.Fade
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.secondaryColor
        text: (fileInfo.type === FolderModel.FolderLink || fileInfo.type === FolderModel.FileLink) ? "→ " + fileInfo.linkTarget
                                : Format.formatDate(fileInfo.mtime, Formatter.DurationElapsed) +
                                  (fileInfo.type === FolderModel.File ? " (" + Format.formatFileSize(fileInfo.size) + ")"
                                                             : "")

    }
}
