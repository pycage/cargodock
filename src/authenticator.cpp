#include "authenticator.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QNetworkRequest>
#include <QString>

#include <QDebug>

namespace
{

QMap<QByteArray, QByteArray> parseWwwAuthenticate(const QByteArray& s)
{
    QMap<QByteArray, QByteArray> result;

    QList<QByteArray> parts = s.split(',');
    foreach (const QByteArray& part, parts)
    {
        int pos = part.indexOf("=");
        if (pos != -1)
        {
            QByteArray key = part.left(pos).trimmed();
            QByteArray value = part.mid(pos + 1).trimmed();
            if (value.startsWith("\"") && value.endsWith("\""))
            {
                value = value.mid(1, value.size() - 2);
            }
            result.insert(key, value);
        }
    }
    return result;
}

}


namespace Network
{

Authenticator::Authenticator()
    : myAuthType(Basic)
    , myNc(0)
{

}

Authenticator::Authenticator(const QByteArray& wwwAuthenticate)
    : myAuthType(Unsupported)
    , myNc(0)
{
    if (wwwAuthenticate.startsWith("Basic"))
    {
        myAuthType = Basic;
    }
    else if (wwwAuthenticate.startsWith("Digest"))
    {
        myAuthType = Digest;
        myDigestParameters =
                parseWwwAuthenticate(wwwAuthenticate.mid(7));
        qDebug() << "Digest parameters:" << myDigestParameters;

        myCnonce = QDateTime::currentDateTime().toString().toUtf8().toBase64();
    }
    else
    {
        qDebug() << "Unsupported authentication type:" << wwwAuthenticate;
    }
}

QByteArray Authenticator::makeDigestA1() const
{
    QByteArray algorithm = myDigestParameters.value("algorithm", "MD5");
    QByteArray realm = myDigestParameters.value("realm");
    QByteArray nonce = myDigestParameters.value("nonce");

    QByteArray a1;
    if (algorithm == "MD5-sess")
    {
        a1 = myUserName.toUtf8() + ":" +
                realm + ":" +
                myPassword.toUtf8() + ":" +
                nonce + ":" +
                myCnonce;
    }
    else
    {
        a1 = myUserName.toUtf8() + ":" +
                realm + ":" +
                myPassword.toLatin1();
    }
    qDebug() << Q_FUNC_INFO << a1;
    return QCryptographicHash::hash(a1, QCryptographicHash::Md5).toHex();
}

QByteArray Authenticator::makeDigestA2(const QByteArray& requestMethod,
                                       const QByteArray& requestUri,
                                       const QByteArray& body) const
{
    QByteArray qop = myDigestParameters.value("qop", QByteArray());

    QByteArray a2;
    if (qop == "auth-int")
    {
        const QByteArray hashedBody =
                QCryptographicHash::hash(body, QCryptographicHash::Md5).toHex();
        a2 = requestMethod + ":" +
                requestUri + ":" +
                hashedBody;
    }
    else
    {
        a2 = requestMethod + ":" +
                requestUri;
    }
    qDebug() << Q_FUNC_INFO << a2;
    return QCryptographicHash::hash(a2, QCryptographicHash::Md5).toHex();
}

QByteArray Authenticator::makeDigestResponse(const QByteArray& a1,
                                             const QByteArray& a2,
                                             const QByteArray& hexNc) const
{
    QByteArray nonce = myDigestParameters.value("nonce");
    QByteArray qop = myDigestParameters.value("qop", QByteArray());

    QByteArray response;
    if (qop.size())
    {
        response = a1 + ":" +
                nonce + ":" +
                hexNc + ":" +
                myCnonce + ":" +
                qop + ":" +
                a2;
    }
    else
    {
        response = a1 + ":" +
                nonce + ":" +
                a2;
    }
    qDebug() << Q_FUNC_INFO << response;
    return QCryptographicHash::hash(response, QCryptographicHash::Md5).toHex();
}

void Authenticator::authenticate(QNetworkRequest& request,
                                 const QByteArray& requestMethod,
                                 const QByteArray& body)
{
    if (myAuthType == Basic)
    {
        const QByteArray token = QString("%1:%2")
                .arg(myUserName)
                .arg(myPassword)
                .toUtf8()
                .toBase64();
        request.setRawHeader("Authorization",
                             QString("Basic %1").arg(QString(token)).toLatin1());
    }
    else if (myAuthType == Digest)
    {
        if (myDigestA1.isEmpty())
        {
            myDigestA1 = makeDigestA1();
        }
        QByteArray a2 = makeDigestA2(requestMethod,
                                     request.url().path().toLatin1(),
                                     body);

        ++myNc;
        QByteArray hexNc = QString::number(myNc, 16).toLatin1();
        while (hexNc.size() < 8)
        {
            hexNc = "0" + hexNc;
        }
        QByteArray response = makeDigestResponse(myDigestA1, a2, hexNc);

        QByteArray realm = myDigestParameters.value("realm");
        QByteArray nonce = myDigestParameters.value("nonce");
        QByteArray qop = myDigestParameters.value("qop", QByteArray());
        QByteArray opaque = myDigestParameters.value("opaque", QByteArray());

        QByteArray header = QString("Digest username=\"%1\"").arg(myUserName).toLatin1();
        header += QString(", realm=\"%1\"").arg(QLatin1String(realm)).toLatin1();
        header += QString(", nonce=\"%1\"").arg(QLatin1String(nonce)).toLatin1();
        header += QString(", uri=\"%1\"").arg(request.url().path()).toLatin1();
        if (qop.size())
        {
            if (qop == "auth-int")
            {
                header += ", qop=\"auth-int\"";
            }
            else
            {
                header += ", qop=\"auth\"";
            }
            header += QString(", nc=%1").arg(QLatin1String(hexNc)).toLatin1();
            header += QString(", cnonce=\"%1\"").arg(QLatin1String(myCnonce)).toLatin1();
        }
        header += QString(", response=\"%1\"").arg(QLatin1String(response)).toLatin1();
        if (opaque.size())
        {
            header += QString(", opaque=\"%1\"").arg(QLatin1String(opaque)).toLatin1();
        }

        request.setRawHeader("Authorization", header);
        qDebug() << "Authorization" << request.rawHeader("Authorization");
    }
}

}
