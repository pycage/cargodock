#include "dropboxapi.h"
#include "../network.h"
#ifdef HAVE_DROPBOX_PRODUCT_KEY
#include "../../../productkey.h"
#else
#include "productkey.h"
#endif

#include <QJsonDocument>
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
const QString API_CONTENT_DROPBOX_ENDPOINT("https://api-content.dropbox.com");
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
    qDebug() << "time" << s << "->" << QDateTime::fromTime_t(timegm(&tm));
    return QDateTime::fromTime_t(timegm(&tm));
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
{

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
                                       const QByteArray& payload,
                                       const QVariantMap& headers)
{
    QNetworkRequest req(url);
    req.setRawHeader("Authorization",
                     QString("Bearer %1")
                     .arg(myAccessToken)
                     .toUtf8());

    foreach (const QString& key, headers.keys())
    {
        req.setRawHeader(key.toUtf8(), headers.value(key).toByteArray());
    }
    qDebug() << Q_FUNC_INFO << myAccessToken << url;

    QNetworkReply* reply = 0;
    QNetworkAccessManager* nam = Network::accessManager();
    if (nam)
    {
        if (method == GET)
        {
            reply = nam->get(req);
        }
        else if (method == POST)
        {
            req.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/x-www-form-urlencoded");
            reply = nam->post(req, payload);
        }
        else if (method == PUT)
        {
            reply = nam->put(req, payload);
        }
    }

    if (reply)
    {
        connect(reply, SIGNAL(finished()),
                this, SLOT(slotRequestFinished()));
    }

    return reply;
}

QVariantMap DropboxApi::getReplyMap(QObject* sender)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender);
    if (reply)
    {
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "HTTP Code" << reply->url() << code;
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

int DropboxApi::getReplyStatus(QObject* sender) const
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender);
    if (reply)
    {
        return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                .toInt();
    }
    else
    {
        return 0;
    }
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

void DropboxApi::createUpload(const QString& identifier)
{
    qDebug() << Q_FUNC_INFO << identifier;
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/chunked_upload")
                .arg(API));
    QUrlQuery query;
    query.addQueryItem("offset", "0");
    url.setQuery(query);
    QNetworkReply* reply = sendRequest(PUT, url, QByteArray());
    reply->setProperty("identifier", identifier);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotUploadCreated()));
}

void DropboxApi::upload(const QString& uploadId,
                        qint64 offset,
                        const QByteArray& chunk)
{
    qDebug() << uploadId << offset << chunk.size();
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/chunked_upload")
                .arg(API));
    QUrlQuery query;
    query.addQueryItem("upload_id", uploadId);
    query.addQueryItem("offset", QString::number(offset));
    url.setQuery(query);
    QNetworkReply* reply = sendRequest(PUT, url, chunk);
    reply->setProperty("identifier", uploadId);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotUploadedChunk()));
}

void DropboxApi::commitUpload(const QString& uploadId, const QString& path)
{
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/commit_chunked_upload/%2/%3")
                .arg(API)
                .arg(myRoot)
                .arg(path));
    QUrlQuery query;
    query.addQueryItem("upload_id", uploadId);
    QNetworkReply* reply = sendRequest(POST, url, query.toString().toUtf8());
    reply->setProperty("identifier", uploadId);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotUploadCommitted()));
}

void DropboxApi::download(const QString& path,
                          qint64 rangeBegin,
                          qint64 rangeEnd)
{
    qDebug() << Q_FUNC_INFO << path << rangeBegin << rangeEnd;
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/%2/%3")
                .arg(API)
                .arg(myRoot)
                .arg(path));

    QVariantMap headers;
    QString range("bytes=%1-%2");
    range = rangeBegin >= 0 ? range.arg(rangeBegin) : range.arg("");
    range = rangeEnd >= 0 ? range.arg(rangeEnd) : range.arg("");
    headers.insert("Range", range.toUtf8());

    QNetworkReply* reply = sendRequest(GET, url, QByteArray(), headers);
    reply->setProperty("identifier", path);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotDownloaded()));
}

void DropboxApi::slotRequestFinished()
{
    sender()->deleteLater();
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

void DropboxApi::slotUploadCreated()
{
    QString identifier = sender()->property("identifier").toString();
    qDebug() << Q_FUNC_INFO << identifier;
    QVariantMap jsonMap = getReplyMap(sender());
    emit uploadCreated(identifier, jsonMap.value("upload_id", "").toString());
}

void DropboxApi::slotUploadedChunk()
{
    bool ok = true;
    int status = getReplyStatus(sender());
    if (status == 400)
    {
        // offset mismatch
        ok = false;
    }
    else if (status == 404)
    {
        // unknown or expired upload_id
        ok = false;
    }

    QString identifier = sender()->property("identifier").toString();
    emit uploaded(identifier, ok);
}

void DropboxApi::slotUploadCommitted()
{
    bool ok = true;
    int status = getReplyStatus(sender());
    if (status == 400)
    {
        // unknown or expired upload_id
        ok = false;
    }

    QString identifier = sender()->property("identifier").toString();
    emit uploadCommitted(identifier, ok);
}

void DropboxApi::slotDownloaded()
{
    QString identifier = sender()->property("identifier").toString();
    int status = getReplyStatus(sender());
    qDebug() << Q_FUNC_INFO << identifier << status;
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && status >= 200 && status < 400)
    {
        QByteArray metadata = reply->rawHeader("x-dropbox-metadata");
        QVariantMap map;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(metadata, &err);
        bool ok = (err.error == QJsonParseError::NoError);
        if (ok)
        {
            map = doc.toVariant().toMap();
        }

        emit downloaded(identifier, reply->readAll(),
                        map.value("bytes", 0).toLongLong(),
                        true);
    }
    else
    {
        emit downloaded(identifier, QByteArray(), 0, false);
    }
}
