#ifndef FTPAPI_H
#define FTPAPI_H

#include <QAbstractSocket>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class QIODevice;
class QTcpSocket;
class FtpCommand;

class FtpApi : public QObject
{
    Q_OBJECT
public:

    explicit FtpApi(QObject* parent = 0);
    virtual ~FtpApi();

    void setAddress(const QString& address);
    void setCredentials(const QString& login, const QString& password);

    bool loggedIn() const;

    void list(const QString& path);
    void retrieve(const QString& path, QIODevice* pipe);

signals:
    void error(const QString& details);
    void listReceived(const QString& data);
    void retrieveFinished(bool success);
    void storeFinished(bool success);

private:
    FtpCommand* sendCommand(const QString& cmd,
                            const QString& arg1 = QString(),
                            const QString& arg2 = QString());
    void clearCommands();
    void connectPassive(int h1, int h2, int h3, int h4, int p1, int p2);
    void processReply(const QByteArray& reply);

private slots:
    void slotControlConnected();
    void slotPassiveConnected();
    void slotControlError(QAbstractSocket::SocketError err);
    void slotPassiveError(QAbstractSocket::SocketError err);
    void slotControlReply();
    void slotPassiveReply();
    void slotCommandFinished();

private:
    QString myAddress;
    QString myUsername;
    QString myPassword;

    QTcpSocket* myControlSocket;
    QTcpSocket* myPassiveSocket;

    QList<FtpCommand*> myControlQueue;
    FtpCommand* myCurrentCommand;
};

#endif // FTPAPI_H
