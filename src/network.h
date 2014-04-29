#ifndef NETWORK_H
#define NETWORK_H

#include <QNetworkAccessManager>

class QNetworkRequest;
class QString;

namespace Network
{
void setAccessManager(QNetworkAccessManager* m);
QNetworkAccessManager* accessManager();

void basicAuth(QNetworkRequest& request,
               const QString& username,
               const QString& password);

}

#endif // NETWORK_H
