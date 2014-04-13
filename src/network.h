#ifndef NETWORK_H
#define NETWORK_H

#include <QNetworkAccessManager>

namespace Network
{
void setAccessManager(QNetworkAccessManager* m);
QNetworkAccessManager* accessManager();
}

#endif // NETWORK_H
