#include "dropboxthumbprovider.h"
#include "network.h"

#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTime>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <unistd.h>

namespace
{
const int API(2);
const QString API_CONTENT_DROPBOX_ENDPOINT("https://content.dropboxapi.com");
}

DropboxThumbProvider::DropboxThumbProvider()
    : QObject()
    , QQuickImageProvider(QQuickImageProvider::Image)
{
    connect(this, SIGNAL(loadImage(QString,QString,QSize)),
            this, SLOT(slotLoadImage(QString,QString,QSize)));
}

QImage DropboxThumbProvider::requestImage(const QString& id,
                                          QSize* size,
                                          const QSize& requestedSize)
{
    QImage img = QImage();

    // id consists of access token and full path (1/thumbnails/<root>/<path>)
    // and maybe some trailing garbage to distinguish icon URLs from preview URLs
    int idx = id.indexOf('/');
    if (idx == -1)
    {
        return img;
    }

    const QString accessToken = id.left(idx);
    const QString path = id.mid(idx);


    if (! myLoadingImages.contains(path))
    {
        myLoadingImages << path;
        emit loadImage(accessToken, path, requestedSize);
    }
    else
    {
        return img;
    }

    QTime timer;
    timer.start();
    while (img.isNull() && timer.elapsed() < 30000 &&
           qobject_cast<QNetworkAccessManager*>(Network::accessManager()))
    {
        myMutex.lock();
        if (myImages.contains(path))
        {
            img.loadFromData(myImages[path]);
            *size = img.size();
        }
        myMutex.unlock();
    }

    myLoadingImages.remove(path);
    return img;
}

void DropboxThumbProvider::slotLoadImage(const QString& accessToken,
                                         const QString& path,
                                         const QSize& requestedSize)
{
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/get_thumbnail").arg(API));
    QJsonObject reqData;
    reqData["path"]=path.left(path.lastIndexOf('?'));
    reqData["format"]="png";
    if (requestedSize.width() <= 128 && requestedSize.height() <= 128)
    {
        reqData["size"]="w128h128";
    }
    else
    {
        reqData["size"]="w640h480";
    }
    QJsonDocument doc(reqData);
    qDebug() << "fetching thumbnail:" << path;
    QNetworkRequest req(url);
    req.setRawHeader("Authorization",
                     QString("Bearer %1")
                     .arg(accessToken)
                     .toUtf8());
    req.setRawHeader("Dropbox-API-Arg", doc.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = Network::accessManager()->get(req);
    reply->setProperty("path", path);

    connect(reply, SIGNAL(finished()),
            this, SLOT(slotImageLoaded()));
}

void DropboxThumbProvider::slotImageLoaded()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError)
    {
        myMutex.lock();
        myImages[reply->property("path").toString()] = reply->readAll();
        myMutex.unlock();
    }
    else
    {
        myMutex.lock();
        myImages[reply->property("path").toString()] = "";
        myMutex.unlock();
    }
    reply->deleteLater();
}
