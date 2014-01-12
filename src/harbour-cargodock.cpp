#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>

#include "developermode.h"
#include "folderbase.h"

#include "dropboxmodel.h"
#include "placesmodel.h"
#include "foldermodel.h"

#include "dropboxthumbprovider.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/template.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    qmlRegisterType<DeveloperMode>("harbour.cargodock", 1, 0, "DeveloperMode");
    qmlRegisterUncreatableType<FolderBase>("harbour.cargodock", 1, 0, "FolderBase", "abstract");

    qmlRegisterType<DropboxModel>("harbour.cargodock", 1, 0, "DropboxModel");
    qmlRegisterType<FolderModel>("harbour.cargodock", 1, 0, "FolderModel");
    qmlRegisterType<PlacesModel>("harbour.cargodock", 1, 0, "PlacesModel");

    QScopedPointer<DropboxThumbProvider> dropBoxThumbProvider(new DropboxThumbProvider);
    view->engine()->addImageProvider("dropbox", dropBoxThumbProvider.data());

    view->setSource(SailfishApp::pathTo("qml/harbour-cargodock.qml"));
    view->show();

    return app->exec();
}

