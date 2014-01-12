#include "dropboxapi.h"
#include "product.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>

#include <locale.h>
#include <time.h>

namespace
{
const int API(1);
const QString DROPBOX_ENDPOINT("https://www.dropbox.com");
const QString API_DROPBOX_ENDPOINT("https://api.dropbox.com");
const QString REDIRECT_URI("https://localhost/oauth2code");

enum
{
    HttpUnchanged = 304,
    HttpTooManyFiles = 406
};

QDateTime fromTimeString(const QString& s)
{
    if (s.isEmpty())
    {
        return QDateTime();
    }

    struct tm tm;
    setlocale(LC_TIME, "C");
    strptime(s.toLatin1().constData(), "%a, %d %b %Y %H:%M:%S %z" , &tm);
    setlocale(LC_TIME, "");
    return QDateTime::fromTime_t(mktime(&tm));
}

QString toTimeString(const QDateTime& d)
{
    time_t t = d.toTime_t();
    char buf[sizeof("Sat, 21 Dec 2013 11:02:51 +0000")];
    setlocale(LC_TIME, "C");
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", gmtime(&t));
    setlocale(LC_TIME, "");
    return QString(buf);
}

}

QMap<QString, QString> DropboxApi::thePathHashes;
QMap<QString, QByteArray> DropboxApi::theHashCache;

DropboxApi::DropboxApi(DropboxRoot root,
                       QObject* parent)
    : QObject(parent)
    , myAppKey(PRODUCT_KEY)
    , myAppSecret(PRODUCT_SECRET)
    , myRoot(root == Dropbox ? "dropbox" : "sandbox")
    , myNetworkAccessManager(new QNetworkAccessManager)
{
    connect(myNetworkAccessManager.data(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotRequestFinished(QNetworkReply*)));
}

void DropboxApi::authorize(const QUrl& uri)
{
    if (uri.isValid())
    {
        // the URI looks like
        //[REDIRECT_URI]#access_token=ABCDEFG&token_type=bearer&uid=12345&state=[STATE]

        const QString u = uri.toString();

        if (! u.startsWith(REDIRECT_URI))
        {
            emit error(InvalidAuthorizationUri);
            return;
        }

        int idx = u.indexOf('#');
        if (idx == -1)
        {
            emit error(InvalidAuthorizationUri);
            return;
        }

        QUrlQuery params(u.mid(idx + 1));

        if (params.queryItemValue("state") != myAuthorizeState)
        {
            emit error(InvalidAuthorizationState);
            return;
        }

        if (params.hasQueryItem("error"))
        {
            qDebug() << "Authorization Error:"
                     << params.queryItemValue("error")
                     << params.queryItemValue("error_description");
            emit error(AuthorizationError);
            return;
        }

        setAccessToken(params.queryItemValue("access_token"),
                 params.queryItemValue("uid"));
        emit authorized();
    }
    else
    {
        myAuthorizeState = QString::number(qrand());

        QUrl url(DROPBOX_ENDPOINT);
        url.setPath("/1/oauth2/authorize");
        QUrlQuery query;
        query.addQueryItem("response_type", "token");
        query.addQueryItem("client_id", myAppKey);
        query.addQueryItem("redirect_uri", REDIRECT_URI);
        query.addQueryItem("state", myAuthorizeState);
        url.setQuery(query);
        qDebug() << "Emitting authorization request:" << url << REDIRECT_URI;
        emit authorizationRequest(url, REDIRECT_URI);
    }
}

void DropboxApi::setAccessToken(const QString& accessToken, const QString& userId)
{
    myAccessToken = accessToken;
    myUserId = userId;
    requestAccountInfo();
}

QNetworkReply* DropboxApi::sendRequest(RequestMethod method,
                                       const QUrl& url,
                                       const QByteArray& payload)
{
    QNetworkRequest req(url);
    req.setRawHeader("Authorization",
                     QString("Bearer %1")
                     .arg(myAccessToken)
                     .toUtf8());

    qDebug() << Q_FUNC_INFO << myAccessToken << url;

    QNetworkReply* reply = 0;
    if (method == GET)
    {
        reply = myNetworkAccessManager->get(req);
    }
    else if (method == POST)
    {
        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
        reply = myNetworkAccessManager->post(req, payload);
    }
    else if (method == PUT)
    {
        reply = myNetworkAccessManager->put(req, payload);
    }
    return reply;
}

QVariantMap DropboxApi::getReplyMap(QObject* sender)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender);
    if (reply)
    {
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "HTTP Code" << code;
        QByteArray data = reply->readAll();
        qDebug() << data;
        if (code == HttpUnchanged)
        {
            qDebug() << "from cache";
            data = theHashCache.value(reply->property("hash").toString(), "");
        }
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        bool ok = (err.error == QJsonParseError::NoError);
        if (ok)
        {
            QVariantMap map = doc.toVariant().toMap();
            if (map.contains("hash"))
            {
                theHashCache[map.value("hash").toString()] = data;
            }
            return map;
        }
    }

    return QVariantMap();
}

DropboxApi::Metadata DropboxApi::parseMetadata(const QVariantMap& map) const
{
    qDebug() << Q_FUNC_INFO;
    Metadata metadata;
    metadata.path = map.value("path", "").toString();
    metadata.bytes = map.value("bytes", 0).toULongLong();
    metadata.mtime = fromTimeString(map.value("client_mtime", "").toString());
    metadata.isDir = map.value("is_dir", false).toBool();
    metadata.mimeType = map.value("mime_type", "application/octet-stream").toString();
    metadata.icon = map.value("icon", "").toString();
    metadata.rev = map.value("rev", "").toString();

    if (map.value("thumb_exists", false).toBool())
    {
        metadata.thumb = QString("/%1/%2/thumbnails/%3%4")
                .arg(myAccessToken)
                .arg(API)
                .arg(myRoot)
                .arg(metadata.path);
    }

    if (map.contains("hash"))
    {
        qDebug() << "hash value" << map.value("hash").toString();
        thePathHashes.insert(metadata.path, map.value("hash").toString());
    }

    QVariantList contents = map.value("contents").toList();
    foreach (const QVariant child, contents)
    {
        metadata.contents << parseMetadata(child.toMap());
    }

    return metadata;
}

void DropboxApi::requestAccountInfo()
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/account/info")
                .arg(API));
    QNetworkReply* reply = sendRequest(GET, url);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotAccountInfoReceived()));
}

void DropboxApi::requestMetadata(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/metadata/%2%3")
                .arg(API)
                .arg(myRoot)
                .arg(path));
    if (thePathHashes.contains(path))
    {
        qDebug() << "path is hashed" << thePathHashes[path];
        QUrlQuery query;
        query.addQueryItem("hash", thePathHashes[path]);
        url.setQuery(query);
    }
    QNetworkReply* reply = sendRequest(GET, url);
    if (thePathHashes.contains(path))
    {
        reply->setProperty("hash", thePathHashes[path]);
    }
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotMetadataReceived()));
}

void DropboxApi::createFolder(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/fileops/create_folder")
                .arg(API));
    QUrlQuery query;
    query.addQueryItem("root", myRoot);
    query.addQueryItem("path", path);
    QNetworkReply* reply = sendRequest(POST, url, query.toString().toUtf8());
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFolderCreated()));
}

void DropboxApi::moveFile(const QString& oldPath, const QString& newPath)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/fileops/move")
                .arg(API));
    QUrlQuery query;
    query.addQueryItem("root", myRoot);
    query.addQueryItem("from_path", oldPath);
    query.addQueryItem("to_path", newPath);
    QNetworkReply* reply = sendRequest(POST, url, query.toString().toUtf8());
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFileMoved()));

}

void DropboxApi::deleteFile(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/fileops/delete")
                .arg(API));
    QUrlQuery query;
    query.addQueryItem("root", myRoot);
    query.addQueryItem("path", path);
    QNetworkReply* reply = sendRequest(POST, url, query.toString().toUtf8());
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFileDeleted()));
}

void DropboxApi::slotRequestFinished(QNetworkReply* reply)
{
    reply->deleteLater();
}

void DropboxApi::slotAccountInfoReceived()
{
    QVariantMap jsonMap = getReplyMap(sender());

    AccountInfo info;
    info.displayName = jsonMap.value("display_name", "").toString();
    info.uid = jsonMap.value("uid", "").toString();
    info.country = jsonMap.value("country", "").toString();

    QVariantMap quotaMap = jsonMap.value("quota_info").toMap();
    info.quoteNormal = quotaMap.value("normal", 0).toULongLong();
    info.quota = quotaMap.value("quota", 0).toULongLong();
    info.quotaShared = quotaMap.value("shared", 0).toULongLong();

    emit accountInfoReceived(info);
}

void DropboxApi::slotMetadataReceived()
{
    QVariantMap jsonMap = getReplyMap(sender());
    Metadata metadata = parseMetadata(jsonMap);
    emit metadataReceived(metadata);
}

void DropboxApi::slotFolderCreated()
{
    QVariantMap jsonMap = getReplyMap(sender());
    if (jsonMap.contains("path"))
    {
        emit folderCreated(jsonMap.value("path").toString());
    }
    else
    {
        emit folderCreated(QString());
    }
}

void DropboxApi::slotFileMoved()
{
    QVariantMap jsonMap = getReplyMap(sender());
    if (jsonMap.contains("path"))
    {
        emit fileMoved(jsonMap.value("path").toString());
    }
    else
    {
        emit fileMoved(QString());
    }
}

void DropboxApi::slotFileDeleted()
{
    QVariantMap jsonMap = getReplyMap(sender());
    if (jsonMap.contains("path"))
    {
        emit fileDeleted(jsonMap.value("path").toString());
    }
    else
    {
        emit fileDeleted(QString());
    }
}
