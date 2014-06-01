# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = harbour-cargodock

CONFIG += sailfishapp
QT += network xml

exists(../productkey.h) {
  DEFINES += HAVE_DROPBOX_PRODUCT_KEY
}

exists(../encryptionpassphrase.h) {
  DEFINES += HAVE_ENCRYPTION_PASSPHRASE
}

SOURCES += src/harbour-cargodock.cpp \
    src/foldermodel.cpp \
    src/copyaction.cpp \
    src/folderbase.cpp \
    src/placesmodel.cpp \
    src/dropboxmodel.cpp \
    src/dropboxapi/dropboxapi.cpp \
    src/dropboxthumbprovider.cpp \
    src/dropboxapi/dropboxfile.cpp \
    src/localfile.cpp \
    src/davmodel.cpp \
    src/network.cpp \
    src/davapi/davfile.cpp \
    src/davapi/davapi.cpp \
    src/pipedevice.cpp \
    src/sslhandler.cpp \
    src/authenticator.cpp \
    src/blowfish.cpp

OTHER_FILES += qml/harbour-cargodock.qml \
    qml/cover/CoverPage.qml \
    rpm/harbour-cargodock.spec \
    rpm/harbour-cargodock.yaml \
    harbour-cargodock.desktop \
    qml/pages/FolderPage.qml \
    qml/pages/FileDelegate.qml \
    qml/pages/FileInfoDialog.qml \
    qml/pages/NewFolderDialog.qml \
    qml/pages/HelpPage.qml \
    qml/pages/KeyValue.qml \
    qml/pages/FileInfo.qml \
    qml/pages/KeySwitch.qml \
    qml/pages/Notification.qml \
    qml/pages/FancyScroller.qml \
    qml/pages/license.js \
    qml/pages/LicensePage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/PreviewItem.qml \
    qml/pages/PreviewFile.qml \
    qml/pages/PreviewImage.qml \
    qml/pages/PreviewText.qml \
    qml/pages/PreviewAudio.qml \
    qml/icon.png \
    src/qtdropbox/LICENCE \
    src/qtdropbox/AUTHORS.md \
    qml/pages/SettingsPage.qml \
    qml/pages/OAuthDialog.qml \
    qml/dropbox.png \
    qml/pages/AddServiceDialog.qml \
    qml/services/ServiceObject.qml \
    qml/services/DropboxService.qml \
    qml/services/LocalService.qml \
    qml/services/PlacesService.qml \
    qml/services/ServiceDelegate.qml \
    qml/services/WebDavService.qml \
    qml/works-with-dropbox.png \
    qml/help.html \
    qml/help.md \
    qml/pages/PreviewGeneric.qml \
    qml/services/TelekomMediencenter.qml \
    qml/pages/SslHandler.qml \
    qml/pages/PassphraseDialog.qml \
    qml/shared/PasswordField.qml

HEADERS += \
    src/foldermodel.h \
    src/folderaction.h \
    src/copyaction.h \
    src/folderbase.h \
    src/placesmodel.h \
    src/developermode.h \
    src/dropboxmodel.h \
    src/dropboxapi/dropboxapi.h \
    src/dropboxthumbprovider.h \
    src/dropboxapi/dropboxfile.h \
    src/localfile.h \
    src/dropboxapi/productkey.h \
    src/filereader.h \
    src/davmodel.h \
    src/network.h \
    src/davapi/davfile.h \
    src/davapi/davapi.h \
    src/pipedevice.h \
    src/sslhandler.h \
    src/authenticator.h \
    src/blowfish.h

