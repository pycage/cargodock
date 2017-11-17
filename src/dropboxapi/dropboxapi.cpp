#include "dropboxapi.h"
#include "../network.h"
#ifdef HAVE_DROPBOX_PRODUCT_KEY
#include "../../../productkey.h"
#else
#include "productkey.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>

#include <locale.h>
#include <time.h>

namespace
{
const int API(2);
const QString DROPBOX_ENDPOINT("https://www.dropbox.com");
const QString API_DROPBOX_ENDPOINT("https://api.dropboxapi.com");
const QString API_CONTENT_DROPBOX_ENDPOINT("https://content.dropboxapi.com");
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
    strptime(s.toLatin1().constData(), "%FT%TZ" , &tm);
    setlocale(LC_TIME, "");
    qDebug() << "time" << s << "->" << QDateTime::fromTime_t(timegm(&tm));
    return QDateTime::fromTime_t(timegm(&tm));
}

QString toTimeString(const QDateTime& d)
{
    time_t t = d.toTime_t();
    char buf[sizeof("2015-05-12T15:50:38Z")];
    setlocale(LC_TIME, "C");
    strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&t));
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
        url.setPath("/oauth2/authorize");
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

QNetworkReply* DropboxApi::sendRequest(APIType method,
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
    qDebug() << Q_FUNC_INFO << myAccessToken << url << " " << payload << " " << headers;

    QNetworkReply* reply = 0;
    QNetworkAccessManager* nam = Network::accessManager();
    if (nam)
    {
        switch(method){
        case RPCRequest:
            req.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/json");
            break;
        case RPCUpload:
            req.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/octet-stream");
            break;
        case RPCDownload:
            break;
        }
        reply = nam->post(req, payload);
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
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "HTTP Code" << reply->url() << code;
        return code;
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
    metadata.path = map.value("path_display", "/").toString();
    metadata.bytes = map.value("size", 0).toULongLong();
    metadata.mtime = fromTimeString(map.value("client_modified", "").toString());
    metadata.isDir = map.value(".tag", "folder").toString()=="folder";
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
        //thePathHashes.insert(metadata.path, map.value("hash").toString());
    }

    QVariantList contents = map.value("entries").toList();
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
    url.setPath(QString("/%1/users/get_current_account")
                .arg(API));
    QNetworkReply* reply = sendRequest(RPCRequest, url,"null");
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotAccountInfoReceived()));
}

void DropboxApi::requestMetadata(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/get_metadata").arg(API));
 //               .arg(myRoot)
 //               .arg(path));
    if (thePathHashes.contains(path))
    {
        qDebug() << "path is hashed" << thePathHashes[path];
        //QUrlQuery query;
        //query.addQueryItem("hash", thePathHashes[path]);
        //url.setQuery(query);
    }
    if(path=="/"){
        requestListFolder(path);
        return;
    }
    QJsonObject reqData;
    reqData["path"]=path;
    reqData["include_media_info"]=true;
    QJsonDocument doc(reqData);
    QNetworkReply* reply = sendRequest(RPCRequest, url,doc.toJson(QJsonDocument::Compact));
    if (thePathHashes.contains(path))
    {
        //reply->setProperty("hash", thePathHashes[path]);
    }
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotMetadataReceived()));
}

void DropboxApi::requestListFolder(const QString &path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/list_folder").arg(API));
    QJsonObject reqData;
    reqData["path"]=(path=="/"?"":path);
    reqData["include_media_info"]=true;
    QJsonDocument doc(reqData);
    QNetworkReply* reply = sendRequest(RPCRequest, url, doc.toJson(QJsonDocument::Compact));
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotMetadataReceived()));
}

void DropboxApi::createFolder(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/create_folder_v2")
                .arg(API));
    QJsonObject reqData;
    reqData["path"]=(path=="/"?"":path);
    QJsonDocument doc(reqData);
    QNetworkReply* reply = sendRequest(RPCRequest, url, doc.toJson(QJsonDocument::Compact));
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFolderCreated()));
}

void DropboxApi::moveFile(const QString& oldPath, const QString& newPath)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/move_v2")
                .arg(API));
    QJsonObject reqData;
    reqData["from_path"]=(oldPath=="/"?"":oldPath);
    reqData["to_path"]=(newPath=="/"?"":newPath);
    QJsonDocument doc(reqData);
    QNetworkReply* reply = sendRequest(RPCRequest, url, doc.toJson(QJsonDocument::Compact));
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFileMoved()));

}

void DropboxApi::deleteFile(const QString& path)
{
    QUrl url;
    url.setUrl(API_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/delete_v2")
                .arg(API));
    QJsonObject reqData;
    reqData["path"]=(path=="/"?"":path);
    QJsonDocument doc(reqData);
    QNetworkReply* reply = sendRequest(RPCRequest, url, doc.toJson(QJsonDocument::Compact));
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotFileDeleted()));
}

void DropboxApi::createUpload(const QString& identifier)
{
    qDebug() << Q_FUNC_INFO << identifier;
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/upload_session/start")
                .arg(API));
    QVariantMap headers;
    QJsonObject reqData;
    reqData["close"]=false;
    QJsonDocument doc(reqData);
    headers.insert("Dropbox-API-Arg", doc.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = sendRequest(RPCUpload, url, QByteArray(), headers);
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
    url.setPath(QString("/%1/files/upload_session/append_v2")
                .arg(API));
    QVariantMap headers;
    QJsonObject reqData;
    QJsonObject cursor;
    cursor["session_id"]=uploadId;
    cursor["offset"]=offset;
    reqData["cursor"]=cursor;
    reqData["close"]=false;
    QJsonDocument doc(reqData);
    headers.insert("Dropbox-API-Arg", doc.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = sendRequest(RPCUpload, url, chunk, headers);
    reply->setProperty("identifier", uploadId);
    connect(reply, SIGNAL(finished()),
            this, SLOT(slotUploadedChunk()));
}

void DropboxApi::commitUpload(const QString& uploadId, const QString& path, qint64 offset)
{
    QUrl url;
    url.setUrl(API_CONTENT_DROPBOX_ENDPOINT, QUrl::StrictMode);
    url.setPath(QString("/%1/files/upload_session/finish")
                .arg(API));
    QVariantMap headers;
    QJsonObject reqData;
    QJsonObject cursor;
    cursor["session_id"]=uploadId;
    cursor["offset"]=offset;
    reqData["cursor"]=cursor;
    QJsonObject commit;
    commit["path"]=path;
    reqData["commit"]=commit;
    QJsonDocument doc(reqData);
    headers.insert("Dropbox-API-Arg", doc.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = sendRequest(RPCUpload, url, QByteArray(), headers);
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
    url.setPath(QString("/%1/files/download").arg(API));

    QVariantMap headers;
    QString range("bytes=%1-%2");
    range = rangeBegin >= 0 ? range.arg(rangeBegin) : range.arg("");
    range = rangeEnd >= 0 ? range.arg(rangeEnd) : range.arg("");
    headers.insert("Range", range.toUtf8());
    QJsonObject reqData;
    reqData["path"]=(path=="/"?"":path);
    QJsonDocument doc(reqData);
    headers.insert("Dropbox-API-Arg", doc.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = sendRequest(RPCDownload, url, QByteArray(), headers);
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
    QVariantMap nameMap = jsonMap.value("name").toMap();

    AccountInfo info;
    info.displayName = nameMap.value("display_name", "").toString();
    info.id = jsonMap.value("account_id", "").toString();
    info.country = jsonMap.value("country", "").toString();

//    QVariantMap quotaMap = jsonMap.value("quota_info").toMap();
//    info.quoteNormal = quotaMap.value("normal", 0).toULongLong();
//    info.quota = quotaMap.value("quota", 0).toULongLong();
//    info.quotaShared = quotaMap.value("shared", 0).toULongLong();

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
    if (jsonMap.contains("metadata"))
    {
        Metadata metadata = parseMetadata(jsonMap.value("metadata").toMap());
        emit folderCreated(metadata.path);
    }
    else
    {
        emit folderCreated(QString());
    }
}

void DropboxApi::slotFileMoved()
{
    QVariantMap jsonMap = getReplyMap(sender());
    if (jsonMap.contains("metadata"))
    {
        Metadata metadata = parseMetadata(jsonMap.value("metadata").toMap());
        emit fileMoved(metadata.path);
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
        Metadata metadata = parseMetadata(jsonMap.value("metadata").toMap());
        emit fileDeleted(metadata.path);
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
    emit uploadCreated(identifier, jsonMap.value("session_id", "").toString());
}

void DropboxApi::slotUploadedChunk()
{
    bool ok = true;
    int status = getReplyStatus(sender());
    QVariantMap jsonMap = getReplyMap(sender());
    qDebug() << Q_FUNC_INFO << jsonMap;
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
    QVariantMap jsonMap = getReplyMap(sender());
    qDebug() << Q_FUNC_INFO << jsonMap;
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
    qDebug() << Q_FUNC_INFO << reply->errorString();
    if (reply && status >= 200 && status < 400)
    {
        QByteArray metadata = reply->rawHeader("dropbox-api-result");
        qDebug() << Q_FUNC_INFO << metadata;
        QVariantMap map;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(metadata, &err);
        bool ok = (err.error == QJsonParseError::NoError);
        if (ok)
        {
            map = doc.toVariant().toMap();
        }

        emit downloaded(identifier, reply->readAll(),
                        map.value("size", 0).toLongLong(),
                        true);
    }
    else
    {
        emit downloaded(identifier, QByteArray(), 0, false);
    }
}
