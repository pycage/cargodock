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
    if (qobject_cast<QNetworkAccessManager*>(manager))
    {
        return manager;
    }
    else
    {
        return 0;
    }
}

}
