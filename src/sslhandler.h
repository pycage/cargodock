#ifndef SSLHANDLER_H
#define SSLHANDLER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSet>
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
    void error(const QString& message, const QString& details);

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
    QSet<QByteArray> myAcceptedCerts;

};

#endif // SSLHANDLER_H
