#include "dropboxthumbprovider.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

#include <unistd.h>

namespace
{
const QString API_CONTENT_DROPBOX_ENDPOINT("https://api-content.dropbox.com");
}

DropboxThumbProvider::DropboxThumbProvider()
    : QObject()
    , QQuickImageProvider(QQuickImageProvider::Image)
    , myNetworkAccessManager(new QNetworkAccessManager)
{
    connect(myNetworkAccessManager.data(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotImageLoaded(QNetworkReply*)));
    connect(this, SIGNAL(loadImage(QString,QString)),
            this, SLOT(slotLoadImage(QString,QString)));
}

QImage DropboxThumbProvider::requestImage(const QString& id,
                                          QSize* size,
                                          const QSize& requestedSize)
{
    QImage img = QImage();

    // id consists of access token and full path (1/thumbnails/<root>/<path>)
    int idx = id.indexOf('/');
    if (idx == -1)
    {
        return img;
    }

    const QString accessToken = id.left(idx);
    const QString path = id.mid(idx);

    emit loadImage(accessToken, path);

    while (img.isNull())
    {
        myMutex.lock();
        if (myImages.contains(path))
        {
            img.loadFromData(myImages[path]);
            *size = img.size();
        }
        myMutex.unlock();
    }

    return img;
}

void DropboxThumbProvider::slotLoadImage(const QString& accessToken,
                                         const QString& path)
{
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(path);
    QUrlQuery query;
    query.addQueryItem("format", "png");
    query.addQueryItem("size", "m" /* 128x128 */);
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
    reply->deleteLater();
}
