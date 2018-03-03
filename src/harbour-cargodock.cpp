#include <sailfishapp.h>
#include <QGuiApplication>
#include <QLatin1String>
#include <QQmlContext>
#include <QQuickView>
#include <QSharedPointer>
#include <QtQml>

#include "network.h"
#include "sslhandler.h"

#include "developermode.h"
#include "filereader.h"
#include "folderbase.h"

#include "davmodel.h"
#include "dropboxmodel.h"
#include "foldermodel.h"
#include "ftpmodel.h"
#include "placesmodel.h"

#include "dropboxthumbprovider.h"

namespace
{
const char* URI("harbour.cargodock");
}
bool DeveloperMode::inDebug=false;
void silentOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        break;
    case QtInfoMsg:
        break;
    case QtWarningMsg:
        break;
    case QtCriticalMsg:
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", msg.toLocal8Bit().constData(), context.file, context.line, context.function);
        abort();
    }
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

    if(!app->arguments().contains("-debug"))
        qInstallMessageHandler(&silentOutput);
    else
        DeveloperMode::inDebug=true;

    QSharedPointer<QQuickView> view(SailfishApp::createView());

    Network::setAccessManager(view->engine()->networkAccessManager());

    SslHandler sslHandler(*Network::accessManager());
    view->engine()->rootContext()->setContextProperty("sslHandler",
                                                      &sslHandler);

    //@uri harbour.cargodock
    qmlRegisterType<DeveloperMode>(URI, 1, 0, "DeveloperMode");
    qmlRegisterType<FileReader>(URI, 1, 0, "FileReader");
    qmlRegisterUncreatableType<FolderBase>(URI, 1, 0, "FolderBase", "abstract");

    qmlRegisterType<DavModel>(URI, 1, 0, "DavModel");
    qmlRegisterType<DropboxModel>(URI, 1, 0, "DropboxModel");
    qmlRegisterType<FolderModel>(URI, 1, 0, "FolderModel");
    qmlRegisterType<FtpModel>(URI, 1, 0, "FtpModel");
    qmlRegisterType<PlacesModel>(URI, 1, 0, "PlacesModel");

    DropboxThumbProvider* dropBoxThumbProvider = new DropboxThumbProvider;
    view->engine()->addImageProvider("dropbox", dropBoxThumbProvider);
    QSettings settings("harbour-cargodock", "CargoDock");
    settings.beginGroup("uiconfig");
    view->setSource(SailfishApp::pathTo(
                        settings.value("useNewUI").toBool()?
                        "qml/NewUi.qml":
                        "qml/ClassicUi.qml"
                        ));
    view->show();

    return app->exec();
}

