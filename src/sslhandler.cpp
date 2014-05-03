#include "sslhandler.h"
#include "network.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

SslHandler::SslHandler(QNetworkAccessManager& nam, QObject* parent)
    : QObject(parent)
    , myChoice(Declined)
{
    connect(&nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(slotSslErrors(QNetworkReply*,QList<QSslError>)));
}

void SslHandler::accept()
{
    myChoice = Accepted;
}

void SslHandler::decline()
{
    myChoice = Declined;
}

void SslHandler::slotSslErrors(QNetworkReply* reply,
                               const QList<QSslError>& errors)
{
    qDebug() << Q_FUNC_INFO << errors;

    emit error();

    if (myChoice == Pending)
    {
        return;
    }

    myChoice = Pending;
    QEventLoop evLoop;
    while (myChoice == Pending && Network::accessManager())
    {
        evLoop.processEvents();
    }

    if (myChoice == Accepted)
    {
        reply->ignoreSslErrors(errors);
    }
    else
    {
        reply->abort();
    }
}
