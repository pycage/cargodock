#ifndef SSLHANDLER_H
#define SSLHANDLER_H

#include <QList>
#include <QObject>
#include <QSslError>

class QNetworkAccessManager;
class QNetworkReply;

class SslHandler : public QObject
{
    Q_OBJECT
public:
    explicit SslHandler(QNetworkAccessManager& nam, QObject* parent = 0);

    Q_INVOKABLE void accept();
    Q_INVOKABLE void decline();

signals:
    void error();

private:
    enum Choice
    {
        Pending,
        Accepted,
        Declined
    };

private slots:
    void slotSslErrors(QNetworkReply* reply,
                       const QList<QSslError>& errors);

private:
    Choice myChoice;

};

#endif // SSLHANDLER_H
