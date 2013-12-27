import QtQuick 2.0
import Sailfish.Silica 1.0

Page {

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            height: childrenRect.height
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Cargo Dock"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "A cargo dock is a harbour where cargo crates are being " +
                      "managed and moved around.\n" +
                      "Cargo Dock on your phone helps you manage and move "+
                      "around files and folders in place of cargo."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "It is a two-paned filemanager specially built for " +
                      "Sailfish OS."
            }

            SectionHeader {
                text: "Navigating"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "When you first open Cargo Dock, you see a collection " +
                      "of places on your phone, for instance your music or your " +
                      "camera photos. Tap on a place to look into it."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Places contain files and folders, and folders in turn " +
                      "may contain more files and folders. By tapping on a " +
                      "folder, you can look into it."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "No matter how many folders deep you dived down, you can " +
                      "always dive up by dragging down the pulley menu. There " +
                      "you will find a trail of 'breadcrumbs' to find your " +
                      "way up again."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Hint: When scrolling fast through a long list of files, " +
                      "a Up or Down button will appear, depending on your " +
                      "direction of movement. Touch it to quickly jump to the " +
                      "beginning or end of the list."
            }

            SectionHeader {
                text: "Creating folders"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Anywhere where you have permission to create new " +
                      "folders, you will see the 'New folder ...' option shown " +
                      "above the folder's contents. Tap on it and you will be " +
                      "asked for the name of the new folder to create.\n" +
                      "Accept the dialog to create the folder."
            }

            SectionHeader {
                text: "The Other Half"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Cargo Dock comes with an Other Half as it is a " +
                      "two-paned filemanager. Drag the screen horizontally to " +
                      "the left to go to the right half. Drag the screen to " +
                      "the right to go back to the left half.\n" +
                      "The glow indicators in the top left corner show you on " +
                      "which of the two halves you currently are."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "You can copy files and folders between the two halves."
            }

            SectionHeader {
                text: "Selecting files"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "In order to perform actions on files or folders, you " +
                      "have to select them. Tap and hold on a file or folder " +
                      "to select it."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Notice the drawer that opened on the bottom of the " +
                      "screen. You are now in selection mode and can select " +
                      "or unselect a number of files and folders by tapping " +
                      "on them. Inside the drawer you will see the amount of items " +
                      "selected."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "To cancel your selection and leave selection mode, tap " +
                      "on the close button (X) in the drawer, or, alternatively, " +
                      "tap and hold on any file."
            }

            SectionHeader {
                text: "File actions"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "When you have selected files in selection mode, you can " +
                      "open a pulley menu on the bottom of the drawer."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "In this menu you will find several actions that you " +
                      "can perform on the items you selected."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "'Copy to other side' copies the selected items to " +
                      "the location you have currently open on the other side."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "'Link to other side' links the selected items to " +
                      "the location you have currently open on the other " +
                      "side. This will place symbolic links on the other side " +
                      "to your selected items."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "'Delete' deletes the selected items. A remorse timer of " +
                      "some seconds will appear and give you a chance to " +
                      "revide your decision. Once deleted, files and folders " +
                      "cannot be brought back, so be careful with this action."
            }

            SectionHeader {
                text: "File properties"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Tap on any file, while not in selection mode, to see its " +
                      "properties. You can also rename the file here.\n" +
                      "This view is a dialog with an 'Open' action to " +
                      "open the file in the associated app. Note that not all " +
                      "filetypes have an app associated to open with, though."
            }

            SectionHeader {
                text: "Safety first"
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Cargo Dock takes precautions that you cannot " +
                      "accidentally break your phone by delete files, " +
                      "so you are on the safe side."
            }

            Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.paddingLarge
                anchors.rightMargin: Theme.paddingLarge
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "Still, Cargo Dock is full filemanager. If you need " +
                      "access beyond what's safe, you have to accept the " +
                      "terms and conditions of the Developer Mode on your " +
                      "device and enable it. Only then, Cargo Dock will unlock " +
                      "the dangerous areas."
            }

        }

        ScrollDecorator { }

    }

}
