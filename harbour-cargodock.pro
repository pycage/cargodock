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

SOURCES += src/harbour-cargodock.cpp \
    src/foldermodel.cpp \
    src/copyaction.cpp \
    src/folderbase.cpp \
    src/placesmodel.cpp

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
    qml/icon.png

HEADERS += \
    src/foldermodel.h \
    src/folderaction.h \
    src/copyaction.h \
    src/folderbase.h \
    src/placesmodel.h \
    src/developermode.h

