#ifndef FTPCOMMAND_H
#define FTPCOMMAND_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

class QIODevice;

class FtpCommand : public QObject
{
    Q_OBJECT
public:
    FtpCommand(const QString& command,
               const QStringList& arguments = QStringList())
        : myCommand(command)
        , myArguments(arguments)
        , myPipe(0)
        , myIsAborted(false)
        , myIsCommandConfirmed(false)
        , myIsTransmissionConfirmed(false)
    { }

    QString command() const { return myCommand; }
    void setPipe(QIODevice* pipe) { myPipe = pipe; }
    QIODevice* pipe() { return myPipe; }
    QByteArray toBytes() const;

    void abort() { myIsAborted = true; checkFinished(); }
    void confirmCommand() { myIsCommandConfirmed = true; checkFinished(); }
    void confirmTransmission() { myIsTransmissionConfirmed = true; checkFinished(); }

    void checkFinished();

signals:
    void finished();

private:
    QString myCommand;
    QStringList myArguments;
    QIODevice* myPipe;

    // true if the command was aborted
    bool myIsAborted;
    // true if the server confirmed command execution
    bool myIsCommandConfirmed;
    // true if the server completed transmission
    bool myIsTransmissionConfirmed;
};

#endif // FTPCOMMAND_H
