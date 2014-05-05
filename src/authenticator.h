#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QByteArray>
#include <QMap>
#include <QString>

class QNetworkRequest;

namespace Network
{

/* Class for performing HTTP authentication.
 *
 * This class supports Basic Authentication and Digest Access Authentication
 * (RFC 2617), including integrity protection.
 */
class Authenticator
{
public:
    /* Creates an Authenticator for Basic Authentication.
     */
    Authenticator();

    /* Creates an Authenticator for the given WWW-Authenticate header line,
     * which requests a certain type of authentication and may provide necessary
     * parameters.
     */
    Authenticator(const QByteArray& wwwAuthenticate);

    /* Sets the user name to use for authentication. You need to call this
     * method once before attempting to authenticate.
     */
    void setUserName(const QString& userName) { myUserName = userName; }

    /* Sets the password to use for authentication. You need to call this
     * method once before attempting to authenticate.
     */
    void setPassword(const QString& password) { myPassword = password; }

    /* Authenticates the given network request. Digest Access Authentication
     * requires you to pass the request method (e.g. GET), and if integrity
     * checking is demanded by the server, the message body as well.
     */
    void authenticate(QNetworkRequest& request,
                      const QByteArray& requestMethod = QByteArray(),
                      const QByteArray& body = QByteArray());

private:
    QByteArray makeDigestA1() const;
    QByteArray makeDigestA2(const QByteArray& requestMethod,
                            const QByteArray& requestUri,
                            const QByteArray& body = QByteArray()) const;
    QByteArray makeDigestResponse(const QByteArray& a1,
                                  const QByteArray& a2,
                                  const QByteArray& hexNc = QByteArray()) const;

private:
    enum AuthType {
        Basic,
        Digest,
        Unsupported
    };

    AuthType myAuthType;
    QString myUserName;
    QString myPassword;
    QMap<QByteArray, QByteArray> myDigestParameters;
    QByteArray myDigestA1;
    QByteArray myCnonce;
    int myNc;
};

}

#endif // AUTHENTICATOR_H
