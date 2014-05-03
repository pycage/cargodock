#include "sslhandler.h"
#include "network.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QStringList>
#include <QDebug>

namespace
{

const QString RT_HEADER("<tr><td colspan=2><br><b>%1</b><br></td></tr>");
const QString RT_LINE("<tr><td colspan=2>%1</td></tr>");
const QString RT_KEY_VALUE("<tr><td><b>%1</b></td><td>%2</td></tr>");

QString formatDigest(const QByteArray& digest)
{
    QString out;
    QByteArray hexDigest = digest.toHex().toUpper();
    for (int i = 0; i < hexDigest.size(); ++i)
    {
        if (i %2 == 0 && i > 0)
        {
            out += ":";
        }
        out += hexDigest.at(i);
    }
    return out;
}

}

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

    if (errors.isEmpty())
    {
        return;
    }

    QString message;
    foreach (const QSslError& err, errors)
    {
        if (message.size())
        {
            message += "\n\n";
        }
        message += err.errorString();
    }

    QSslCertificate cert = errors.first().certificate();
    const QByteArray certDigest = cert.digest();

    if (! myAcceptedCerts.contains(certDigest))
    {
        QString details;
        details += "<table width='100%'>";

        details += RT_KEY_VALUE
                .arg("Issued")
                .arg(cert.effectiveDate().toString(Qt::SystemLocaleShortDate));
        details += RT_KEY_VALUE
                .arg("Expires")
                .arg(cert.expiryDate().toString(Qt::SystemLocaleShortDate));

        details += RT_HEADER.arg("Issued for");
        foreach (const QByteArray& attr, cert.subjectInfoAttributes())
        {
            details += RT_KEY_VALUE
                    .arg(QString(attr))
                    .arg(cert.subjectInfo(attr).join(", "));
            qDebug() << "Subject Info Attribute" << attr
                     << cert.subjectInfo(attr);
        }

        details += RT_HEADER.arg("Issued by");
        foreach (const QByteArray& attr, cert.issuerInfoAttributes())
        {
            details += RT_KEY_VALUE
                    .arg(QString(attr))
                    .arg(cert.issuerInfo(attr).join(", "));
            qDebug() << "Issuer Info Attribute" << attr
                     << cert.issuerInfo(attr);
        }

        details += RT_HEADER.arg("Alternative names");
        QMultiMap<QSsl::AlternativeNameEntryType, QString> altNames =
                cert.subjectAlternativeNames();
        for (QMultiMap<QSsl::AlternativeNameEntryType, QString>::ConstIterator iter = altNames.begin();
             iter != altNames.end(); ++iter)
        {
            details += RT_KEY_VALUE
                    .arg(iter.key() == QSsl::EmailEntry ? "Mail" : "DNS")
                    .arg(iter.value());
        }

        details += RT_HEADER.arg("Serial");
        details += RT_LINE.arg(QString(cert.serialNumber().toUpper()));

        details += RT_HEADER.arg("SHA1 Digest");
        details += RT_LINE.arg(formatDigest(cert.digest(QCryptographicHash::Sha1)));
        details += RT_HEADER.arg("MD5 Digest");
        details += RT_LINE.arg(formatDigest(cert.digest(QCryptographicHash::Md5)));

        details += "</table>";

        foreach (const QSslCertificateExtension& ext, cert.extensions())
        {
            qDebug() << "Extension" << ext.name() << ext.value();
        }

        emit error(message, details);

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
    }
    else
    {
        myChoice = Accepted;
    }

    if (myChoice == Accepted)
    {
        qDebug() << "Accepting SSL certificate" << certDigest;
        reply->ignoreSslErrors(errors);
        myAcceptedCerts << certDigest;
    }
    else
    {
        reply->abort();
    }
}
