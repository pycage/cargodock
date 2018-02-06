#include "ftpcommand.h"

QByteArray FtpCommand::toBytes() const
{
    QByteArray data = myCommand.toLatin1();
    foreach (const QString& arg, myArguments)
    {
        data += " " + arg.toLatin1();
    }
    return data;
}

void FtpCommand::checkFinished()
{
    bool isFinished = false;
    if (myIsAborted)
    {
         isFinished = true;
    }
    else if (myCommand == "LIST" ||
             myCommand == "RETR")
    {
        isFinished = myIsCommandConfirmed && myIsTransmissionConfirmed;
    }
    else
    {
        isFinished = myIsCommandConfirmed;
    }

    if (isFinished)
    {
        emit finished();
    }
}
