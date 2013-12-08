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

SOURCES += src/harbour-cargodock.cpp

OTHER_FILES += qml/harbour-cargodock.qml \
    qml/cover/CoverPage.qml \
    qml/pages/SecondPage.qml \
    rpm/harbour-cargodock.spec \
    rpm/harbour-cargodock.yaml \
    harbour-cargodock.desktop \
    qml/pages/FolderPage.qml

