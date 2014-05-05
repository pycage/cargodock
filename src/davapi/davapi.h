#ifndef DAVAPI_H
#define DAVAPI_H

#include <QDateTime>
#include <QNetworkRequest>
#include <QObject>
#include <QSharedPointer>
#include <QString>

class QIODevice;
class QNetworkReply;
class QUrl;

namespace Network
{
class Authenticator;
}

class DavApi : public QObject
{
    Q_OBJECT
public:
    struct Properties
    {
        QString name;
        QString href;
        QString resourceType;
        QString contentType;
        int contentLength;
        QDateTime lastModified;
    };

    enum
    {
        ServerUnreachable = 0,
        Created = 201,
        NoContent = 204,
        MultiStatus = 207,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        Gone = 410,
        NotAllowed = 405,
        Conflict = 409,
        UnsupportedMediaType = 415,
        FailedDependency = 424,
        ServerError = 500,
        InsufficientStorage = 507
    };

    DavApi(QObject* parent = 0);

    void setAddress(const QString& address);
    void setAuthorization(const QString& login, const QString& password)
    {
        myLogin = login;
        myPassword = password;
    }

    void propfind(const QString& path);
    void mkcol(const QString& path);
    void deleteResource(const QString& path);
    void moveResource(const QString& path, const QString& newPath);
    QNetworkReply* getResource(const QString& path, qint64 offset, qint64 size);
    void putResource(const QString& path, qint64 size, QIODevice* buffer);

    QSharedPointer<Network::Authenticator> authenticator() { return myAuthenticator; }
    void setAuthenticator(QSharedPointer<Network::Authenticator> authenticator) { myAuthenticator = authenticator; }

signals:
    void propertiesReceived(int result, const DavApi::Properties& props);
    void mkColFinished(int result);
    void deleteFinished(int result);
    void moveFinished(int result);
    void resourceReceived(const QString& path,
                          int result);
    void putFinished(const QString&, int result);

private:
    QNetworkRequest makeRequest(const QUrl& url,
                                const QByteArray& requestMethod);

private slots:
    void slotPropfindReceived();
    void slotMkColFinished();
    void slotDeleteFinished();
    void slotMoveFinished();
    void slotResourceReceived();
    void slotPutFinished();

private:
    QString myAddress;
    QString myLogin;
    QString myPassword;
    QString myPropfindPath;
    bool myAuthAttempted;

    QSharedPointer<Network::Authenticator> myAuthenticator;
};

#endif // DAVAPI_H
