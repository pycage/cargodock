#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>

#include "developermode.h"
#include "folderbase.h"
#include "foldermodel.h"
#include "placesmodel.h"

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
    qmlRegisterType<FolderModel>("harbour.cargodock", 1, 0, "FolderModel");
    qmlRegisterType<PlacesModel>("harbour.cargodock", 1, 0, "PlacesModel");

    view->setSource(SailfishApp::pathTo("qml/harbour-cargodock.qml"));
    view->show();

    return app->exec();
}

