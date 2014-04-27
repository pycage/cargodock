#ifndef DAVAPI_H
#define DAVAPI_H

#include <QDateTime>
#include <QObject>
#include <QString>

class QIODevice;
class QNetworkReply;

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
        Created = 201,
        NoContent = 204,
        MultiStatus = 207,
        Forbidden = 403,
        NotAllowed = 405,
        Conflict = 409,
        UnsupportedMediaType = 415,
        FailedDependency = 424,
        InsufficientStorage = 507
    };

    DavApi(QObject* parent = 0);

    void setAddress(const QString& address) { myAddress = address; }

    void propfind(const QString& path);
    void mkcol(const QString& path);
    void deleteResource(const QString& path);
    QNetworkReply* getResource(const QString& path, qint64 offset, qint64 size);
    void putResource(const QString& path, QIODevice* buffer);

signals:
    void propertiesReceived(const DavApi::Properties& props);
    void mkColFinished(int result);
    void deleteFinished(int result);
    void resourceReceived(const QString& path,
                          int result);
    void putFinished(const QString&, int result);

private slots:
    void slotPropfindReceived();
    void slotMkColFinished();
    void slotDeleteFinished();
    void slotResourceReceived();
    void slotPutFinished();

private:
    QString myAddress;
};

#endif // DAVAPI_H
