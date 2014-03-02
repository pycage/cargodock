#include "dropboxthumbprovider.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTime>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

#include <unistd.h>

namespace
{
const QString API_CONTENT_DROPBOX_ENDPOINT("https://api-content.dropbox.com");
}

DropboxThumbProvider::DropboxThumbProvider(QNetworkAccessManager* nam)
    : QObject()
    , QQuickImageProvider(QQuickImageProvider::Image)
    , myNetworkAccessManager(nam)
{
    connect(myNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotImageLoaded(QNetworkReply*)));
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
    while (img.isNull() && timer.elapsed() < 30000
           && qobject_cast<QNetworkAccessManager*>(myNetworkAccessManager))
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
    url.setPath(path.left(path.lastIndexOf('?')));
    QUrlQuery query;
    query.addQueryItem("format", "png");
    if (requestedSize.width() <= 128 && requestedSize.height() <= 128)
    {
        query.addQueryItem("size", "m" /* 128x128 */);
    }
    else
    {
        query.addQueryItem("size", "l" /* 640x480 */);
    }
    url.setQuery(query);
    qDebug() << "fetching thumbnail:" << url;
    QNetworkRequest req(url);
    req.setRawHeader("Authorization",
                     QString("Bearer %1")
                     .arg(accessToken)
                     .toUtf8());
    QNetworkReply* reply = myNetworkAccessManager->get(req);
    reply->setProperty("path", path);
}

void DropboxThumbProvider::slotImageLoaded(QNetworkReply* reply)
{
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
