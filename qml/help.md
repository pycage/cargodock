A cargo dock is a harbour where cargo crates are being managed and moved around.

**Cargo Dock** on your phone helps you manage and move around files and folders on your phone and the cloud in place of cargo.

It is a two-paned filemanager for your phone and the cloud.

### At a glance: what's new in this version

* new cloud services for WebDAV, Deutsche Telekom Mediencenter, and ownCloud
* handling of HTTP authentication and SSL connections
* secure password storage for cloud service passwords

### Some features

* two-paned UI
* easy folder navigation with trail of breadcrumbs
* create new folders
* perform file-operations (copy, delete, link, rename)
* open files with default handler
* bookmark locations
* preview for images and audio
* integrated cloud and network services:  
  Dropbox
* extended functionality in Developer Mode:  
  see hidden files,  
  navigate the whole filesystem,  
  modify file permissions

### Navigating

When you first open **Cargo Dock**, you will see a collection of places on your phone, for instance your music or your camera photos. Tap on a place to look into it.

Places contain files and folders, and folders in turn may contain more files and folders. By tapping on a folder, you can look into it.

No matter how many folders deep you dived down, you can always dive up by dragging down the pulley menu. There you will find a trail of breadcrumbs to return up again.

**Hint:** When scrolling fast through a long list of files, a 'Up' or 'Down' button will appear, depending on your direction of movement. Touch it to quickly jump to the beginning or end of the list.

### Creating folders

Anywhere where you have permission to create new folders, you will see the 'New Folder ...' option shown above the folder's contents. Tap on it and you will be asked for the name of the new folder to create. Accept the dialog in order to create the folder.

### The other side

**Cargo Dock** comes with an other side as it is a two-paned filemanager. Drag the screen horizontally to the left to move to the right side. Drag the screen to the right to move back to the left side.

The glow indicators in the top left corner show you on which of the two sides you currently are. There is also a perforation shown at the edge to the other half.

You can, for instance, copy files and folders between the two sides.

### Selecting files

In order to perform actions on files or folders, you have to select them. Tap and hold on a file or folder to select it.

Notice the drawer that opened at the bottom of the screen. You are now in selection mode and can select or unselect a number of files and folders by tapping on them. On the drawer, you will see the amount of items selected.

In order to cancel your selection and leave selection mode, tap on the close button (X) of the drawer, or, alternatively, tap and hold on any file.

### File actions

When you have selected files in selection mode, a pulley menu appears on the bottom of the drawer.

In this menu you will find several actions that you can perform on the items selected.

* **Bookmark**  
  ... creates a bookmark to the selected folders. This option is only available if you have selected folders only, and the other side is the 'Places' location.
* **Copy to other side**  
  ... copies the selected items to the location you have currently open on the other side.
* **Link to other side**  
  ... links the selected items to the location you have currently open on the other side. This will place symbolic links on the other side to your selected items. Not all filesystems support symbolic links.
* **Delete**  
  ... deletes the selected items. A remorse timer of some seconds will appear to give you a chance to revide your decision. Once deleted, files and folders cannot be brought back, so be careful with this action.
  
### File properties

Tap on any file, while not in selection mode, to see its properties. You can also rename the file here.

The file properties dialog has an 'Open' action for supported file types that can be opened with another app.

### Managing bookmarks

You can place bookmarks on the 'Places' location. To do so, select the folders you want to bookmark (you can only bookmark folders) and select 'Bookmark' from the actions menu. The other side must have the 'Places' location open.

To remove a bookmark, select it on the 'Places' location, and choose  'Delete' from the actions menu.

### Managing services

Open 'Settings' from the pulley menu on 'Places' and drag down to select 'Add Service'.

Tap on a service and follow the instructions.

The new service will appear in the 'Places' location. You may create more than one instance of a service, for instance, to have access to several Dropbox accounts.

To delete a service instance, tap and hold on the service in 'Settings' and select 'Remove' from the context menu.

### Secure password storage

Cloud services generally require you to enter a password, and **Cargo Dock** will store it on your phone. But worry not, as the passwords are stored encrypted (Blowfish-encrypted to be precise).

By default, you do not have to care about the password encryption as **Cargo Dock** encrypts the passwords with its default passphrase.

However, if you want more security (a malicious hacker could figure out the default passphrase eventually), you can set your own passphrase, that is not stored anywhere (to be precise, **Cargo Dock** stores a cryptographic fingerprint of your passphrase to validate it, but the passphrase cannot be derived from the fingerprint).

Go to 'Settings' and enable 'Secure password storage' to set your own passphrase.

Since your custom passphrase is not stored, you will have to enter it once per session, when needed for the first time.
If you forget this passphrase, the stored passwords will become unusable.

**Hint:** You can always change the passphrase and all stored passwords will be re-encrypted automatically. To change the passphrase, disable 'Secure password storage', enter your current passphrase, and enable it again, this time entering your new passphrase.

**Hint:** If you really forgot your passphrase, you can still disable the 'Secure password storage' by tapping on 'I forgot' when being asked for the current passphrase. **All stored passwords will become unusable at that point, and you will have to enter them again.**

### Safe Mode vs. Developer Mode

**Cargo Dock** takes precautions that you cannot accidentally break your phone by deleting system files, so you are on the safe side.

Still, **Cargo Dock** is a full filemanager. If you need access beyond what's safe, you have to accept the terms and conditions of Developer Mode on your device and enable it. Only then **Cargo Dock** will unlock the dangerous areas.

Developer Mode adds a new section to 'Places', giving you:

* access to the full filesystem
* access to hidden files
* extended view of file permissions with editing

### Root

**Cargo Dock** runs as the default Sailfish user.

If **Cargo Dock** detects that it was started as super user, it will show a reddish background to remind you of your devastating powers.

**Hint:** A way to run **Cargo Dock** as the super user can be found at

[https://openrepos.net/content/schturman/startasroot-cargodock](https://openrepos.net/content/schturman/startasroot-cargodock)

**Use at your own risk!** It is a 3rd party extension not affiliated with **Cargo Dock** in any way.
