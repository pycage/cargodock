#include "network.h"

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
    return manager;
}

}
