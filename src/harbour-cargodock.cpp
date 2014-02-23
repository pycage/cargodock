#include <sailfishapp.h>
#include <QGuiApplication>
#include <QLatin1String>
#include <QQmlContext>
#include <QQuickView>
#include <QSharedPointer>
#include <QtQml>

#include "developermode.h"
#include "filereader.h"
#include "folderbase.h"

#include "dropboxmodel.h"
#include "placesmodel.h"
#include "foldermodel.h"

#include "dropboxthumbprovider.h"

namespace
{
const char* URI("harbour.cargodock");
}

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

    QSharedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QSharedPointer<QQuickView> view(SailfishApp::createView());

    //@uri harbour.cargodock
    qmlRegisterType<DeveloperMode>(URI, 1, 0, "DeveloperMode");
    qmlRegisterType<FileReader>(URI, 1, 0, "FileReader");
    qmlRegisterUncreatableType<FolderBase>(URI, 1, 0, "FolderBase", "abstract");

    qmlRegisterType<DropboxModel>(URI, 1, 0, "DropboxModel");
    qmlRegisterType<FolderModel>(URI, 1, 0, "FolderModel");
    qmlRegisterType<PlacesModel>(URI, 1, 0, "PlacesModel");

    DropboxThumbProvider* dropBoxThumbProvider = new DropboxThumbProvider(view->engine()->networkAccessManager());
    view->engine()->addImageProvider("dropbox", dropBoxThumbProvider);

    view->setSource(SailfishApp::pathTo("qml/harbour-cargodock.qml"));
    view->show();

    return app->exec();
}

