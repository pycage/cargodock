#include "ftpapi.h"
#include "ftpcommand.h"
#include "../network.h"
#include "../pipedevice.h"

#include <QHostAddress>
#include <QRegExp>
#include <QTcpSocket>
#include <QDebug>

// For testing, you can launch an FTP server on OS-X via
//   sudo -s launchctl load -w /System/Library/LaunchDaemons/ftp.plist
// and stop via
//   sudo -s launchctl unload -w /System/Library/LaunchDaemons/ftp.plist

namespace
{

enum ReplyType
{
    PositivePreliminary,
    PositiveCompletion,
    PositiveIntermediate,
    TransientNegative,
    PermanentNegative
};

enum ReplyGroup
{
    Syntax,
    Information,
    Connections,
    Authentication,
    Unspecified,
    FileSystem
};

enum ReplyDetails
{
    RestartMarkerReply = 110,
    ServiceReadyInXMinutes = 120,
    DataConnectionAlreadyOpen = 125,
    FileStatusOk = 150,

    CommandOk = 200,
    CommandNotImplemented = 202,
    NameSystemType = 215,
    ServiceReady = 220,
    ServiceClosingControlConnection = 221,
    DataConnectionOpenNoTransferInProgress = 225,
    ClosingDataConnection = 226,
    PassiveModeOk = 227,
    LogInOk = 230,
    FileActionOk = 250,
    PathCreated = 257,

    PasswordRequired = 331,
    LoginRequired = 332,
    FileActionPending = 350,

    ServiceNotAvailable = 421,
    CannotOpenDataConnection = 425,
    ConnectionClosed = 426,
    FileUnavailable = 450,
    LocalError = 451,
    OutOfSpace = 452,

    CommandSyntaxError = 500,
    ParametersSyntaxError = 501,
    CommandNotAllowed = 502,
    BadCommandSequence = 503,
    CommandNotImplementedForParameter = 504,
    LoginError = 530,
    NeedAcountForStoringFiles = 532,
    FileUnavailableError = 550,
    PageTypeUnknown = 551,
    ExceededStorage = 552,
    InvalidFilename = 553
};

ReplyType replyType(int code)
{
    if (code >= 100 && code <= 199)
        return PositivePreliminary;
    else if (code >= 200 && code <= 299)
        return PositiveCompletion;
    else if (code >= 300 && code <= 399)
        return PositiveIntermediate;
    else if (code >= 400 && code <= 499)
        return TransientNegative;
    else
        return PermanentNegative;
}

ReplyGroup replyGroup(int code)
{
    code %= 100;
    if (code >= 0 && code <= 9)
        return Syntax;
    else if (code >= 10 && code <= 19)
        return Information;
    else if (code >= 20 && code <= 29)
        return Connections;
    else if (code >= 30 && code <= 39)
        return Authentication;
    else if (code >= 40 && code <= 49)
        return Unspecified;
    else
        return FileSystem;
}

QRegExp RE_PASSIVE_MODE("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");

}

FtpApi::FtpApi(QObject* parent)
    : QObject(parent)
    , myCurrentCommand(0)
{
    myControlSocket = new QTcpSocket(this);
    myPassiveSocket = new QTcpSocket(this);

    connect(myControlSocket, SIGNAL(connected()),
            this, SLOT(slotControlConnected()));
    connect(myControlSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotControlError(QAbstractSocket::SocketError)));
    connect(myControlSocket, SIGNAL(readyRead()),
            this, SLOT(slotControlReply()));

    connect(myPassiveSocket, SIGNAL(connected()),
            this, SLOT(slotPassiveConnected()));
    connect(myPassiveSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotPassiveError(QAbstractSocket::SocketError)));
    connect(myPassiveSocket, SIGNAL(readyRead()),
            this, SLOT(slotPassiveReply()));

}

FtpApi::~FtpApi()
{
    qDebug() << Q_FUNC_INFO << this;
}

bool FtpApi::loggedIn() const
{
    return myPassiveSocket->isOpen();
}

void FtpApi::setAddress(const QString& address)
{
    myAddress = address;

    // bogus command to await the server's welcome reply
    myCurrentCommand = new FtpCommand("CONNECT");
    connect(myCurrentCommand, SIGNAL(finished()),
            this, SLOT(slotCommandFinished()));

    myControlSocket->connectToHost(address, 21, QIODevice::ReadWrite);
}

void FtpApi::setCredentials(const QString& login, const QString& password)
{
    myUsername = login;
    myPassword = password;
    sendCommand("USER", myUsername);
    sendCommand("PASS", myPassword);
}

void FtpApi::list(const QString& path)
{
    qDebug() << Q_FUNC_INFO;
    sendCommand("TYPE", "A");
    sendCommand("PASV");
    sendCommand("LIST", path.startsWith("/") ? path.mid(1) : path);
}

void FtpApi::retrieve(const QString& path, QIODevice* pipe)
{
    qDebug() << Q_FUNC_INFO << path;
    sendCommand("TYPE", "I");
    sendCommand("PASV");
    FtpCommand* cmd = sendCommand("RETR", path.startsWith("/") ? path.mid(1) : path);
    cmd->setPipe(pipe);
}

FtpCommand* FtpApi::sendCommand(const QString& cmd,
                                const QString& arg1,
                                const QString& arg2)
{
    QStringList arguments;
    if (arg1.size())
    {
        arguments << arg1;
    }
    if (arg2.size())
    {
        arguments << arg2;
    }
    qDebug() << cmd << arguments;
    FtpCommand* ftpCmd = new FtpCommand(cmd, arguments);
    connect(ftpCmd, SIGNAL(finished()),
            this, SLOT(slotCommandFinished()));
    myControlQueue << ftpCmd;

    if (! myCurrentCommand)
    {
        slotCommandFinished();
    }

    return ftpCmd;
}

void FtpApi::clearCommands()
{
    myControlQueue.clear();
}

void FtpApi::connectPassive(int h1, int h2, int h3, int h4, int p1, int p2)
{
    QString host = QString("%1.%2.%3.%4")
            .arg(h1)
            .arg(h2)
            .arg(h3)
            .arg(h4);
    int port = (p1 << 8) + p2;
    qDebug() << "Opening passive connection to host" << host
             << "port" << port;
    myPassiveSocket->connectToHost(host,
                                   port,
                                   QIODevice::ReadWrite);
}

void FtpApi::processReply(const QByteArray& reply)
{
    int status = reply.left(3).toInt();
    QString message = QString::fromLatin1(reply.mid(3));
    if (message.startsWith("-"))
    {
        // multiline reply
        return;
    }

    qDebug() << reply;

    switch (status)
    {
    case CommandOk:
        break;

    case CommandNotAllowed:
        if (myCurrentCommand)
        {
            emit error(QString("Command rejected by server:\n%1")
                       .arg(QString::fromLatin1(myCurrentCommand->toBytes())));
        }
        break;

    case ServiceReady:
        break;
    case LoginRequired:
        //sendCommand("USER", myUsername);
        break;

    case PasswordRequired:
        //sendCommand("PASS", myPassword);
        break;

    case LogInOk:
        //sendCommand("TYPE", "I");
        break;

    case LoginError:
        emit error("Login error");
        break;

    case PassiveModeOk:
        if (RE_PASSIVE_MODE.indexIn(message) != -1)
        {
            connectPassive(RE_PASSIVE_MODE.cap(1).toInt(),
                           RE_PASSIVE_MODE.cap(2).toInt(),
                           RE_PASSIVE_MODE.cap(3).toInt(),
                           RE_PASSIVE_MODE.cap(4).toInt(),
                           RE_PASSIVE_MODE.cap(5).toInt(),
                           RE_PASSIVE_MODE.cap(6).toInt());
        }
        else
        {
            qDebug() << "Invalid reply to PASV:" << message;
            emit error("Failed to enter passive mode.");
        }
        break;

    case CannotOpenDataConnection:
        emit error("Failed to transfer data.");
        break;

    case FileStatusOk:
        break;

    case ClosingDataConnection:
        break;

    default:
        break;
    }

    switch (replyType(status))
    {
    case PositivePreliminary:
        break;
    case TransientNegative:
    case PermanentNegative:
        clearCommands();
        if (myCurrentCommand->command() == "LIST")
        {
            emit listReceived(QString());
        }
        else if (myCurrentCommand->command() == "RETR")
        {
            emit retrieveFinished(false);
        }
        myCurrentCommand->abort();
        emit error(QString::fromLatin1(reply));
        break;
    case PositiveCompletion:
    case PositiveIntermediate:
    default:
        myCurrentCommand->confirmCommand();
        break;
    }
}

void FtpApi::slotControlConnected()
{
    qDebug() << Q_FUNC_INFO;
}

void FtpApi::slotControlError(QAbstractSocket::SocketError err)
{
    qDebug() << Q_FUNC_INFO << err;
}

void FtpApi::slotPassiveConnected()
{
    qDebug() << Q_FUNC_INFO;
}

void FtpApi::slotPassiveError(QAbstractSocket::SocketError err)
{
    qDebug() << Q_FUNC_INFO << err;
    if (err == QAbstractSocket::RemoteHostClosedError)
    {
        // not an error; transmission finished in stream mode
        if (! myCurrentCommand)
        {
            return;
        }
        else if (myCurrentCommand->command() == "LIST")
        {
            emit listReceived(QString());
        }
        else if (myCurrentCommand->command() == "RETR")
        {
            emit retrieveFinished(true);
        }
        myCurrentCommand->confirmTransmission();
    }
}

void FtpApi::slotControlReply()
{
    while (myControlSocket->canReadLine())
    {
        processReply(myControlSocket->readLine());
    }
}

void FtpApi::slotPassiveReply()
{
    qDebug() << Q_FUNC_INFO;

    if (! myCurrentCommand)
    {
        return;
    }
    else if (myCurrentCommand->command() == "LIST")
    {
        emit listReceived(QString::fromLatin1(
                              myPassiveSocket->read(
                                  myPassiveSocket->bytesAvailable())));
    }
    else if (myCurrentCommand->command() == "RETR")
    {
        myCurrentCommand->pipe()->write(
                    myPassiveSocket->read(myPassiveSocket->bytesAvailable()));
    }
}

void FtpApi::slotCommandFinished()
{
    // remove command
    if (myCurrentCommand)
    {
        delete myCurrentCommand;
        myCurrentCommand = 0;
    }

    // get and send next command
    if (myControlQueue.size())
    {
        myCurrentCommand = myControlQueue.takeFirst();
        qDebug() << ">" << myCurrentCommand->toBytes();
        myControlSocket->write(myCurrentCommand->toBytes());
        myControlSocket->write("\r\n");
    }
}
