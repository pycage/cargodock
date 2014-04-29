#include "network.h"

#include <QNetworkRequest>
#include <QString>

namespace
{
QNetworkAccessManager* manager;
}

namespace Network
{


void setAccessManager(QNetworkAccessManager* m)
{
    manager = m;
}

QNetworkAccessManager* accessManager()
{
    if (qobject_cast<QNetworkAccessManager*>(manager))
    {
        return manager;
    }
    else
    {
        return 0;
    }
}

void basicAuth(QNetworkRequest& request,
               const QString& username, const QString& password)
{
    const QByteArray token = QString("%1:%2")
                             .arg(username)
                             .arg(password)
                             .toUtf8()
                             .toBase64();
    request.setRawHeader("Authorization",
                         QString("Basic %1").arg(QString(token)).toUtf8());
}

}
