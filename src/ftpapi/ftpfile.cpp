#include "ftpfile.h"
#include "ftpapi.h"

#include <QEventLoop>

FtpFile::FtpFile(const QString& path,
                 QSharedPointer<FtpApi> api,
                 QObject* parent)
    : QIODevice(parent)
    , myFtpApi(api)
    , myPath(path)
    , myWaitingForDownload(false)
    , myWaitingForUpload(false)
    , myReply(0)
    , myHasError(false)
{
    connect(myFtpApi.data(), SIGNAL(retrieveFinished(bool)),
            this, SLOT(slotRetrieveFinished(bool)));
}

FtpFile::~FtpFile()
{
    if (myReply)
    {
        myReply->deleteLater();
    }
}

bool FtpFile::open(OpenMode mode)
{
    bool success = false;
    myReadOffset = 0;

    if (mode & QIODevice::ReadOnly)
    {
        success = true;
    }

    if (mode & QIODevice::WriteOnly)
    {
        success = true;
        myWaitingForUpload = true;
        //myFtpApi->store(&myWriteBuffer);
    }

    return success && QIODevice::open(mode);
}

void FtpFile::close()
{
    qDebug() << "closing write buffer";
    myWriteBuffer.close();

    // TODO: timeout!
    QEventLoop evLoop;
    qDebug() << Q_FUNC_INFO << myWaitingForDownload << myWaitingForUpload;
    /*
    while (myWaitingForDownload || myWaitingForUpload)
    {
        evLoop.processEvents();
    }
    */
    qDebug() << Q_FUNC_INFO << "finished";
}

qint64 FtpFile::readData(char* data, qint64 maxlen)
{
    qDebug() << Q_FUNC_INFO << maxlen;
    if (! myReply)
    {
        myWaitingForDownload = true;
        myReply = new PipeDevice;
        myFtpApi->retrieve(myPath, myReply);
        qDebug() << Q_FUNC_INFO << myReply;
    }

    if (maxlen > 0)
    {
        QEventLoop evLoop;
        qDebug() << Q_FUNC_INFO << "waiting for data";
        while (myReply->bytesAvailable() == 0 && myWaitingForDownload)
        {
            evLoop.processEvents();
        }
        qDebug() << Q_FUNC_INFO << "data received";
    }

    if (myHasError)
    {
        return -1;
    }
    else
    {
        qDebug() << "read" << maxlen;
        QByteArray readData = myReply->read(maxlen);
        if (readData.size() > 0)
        {
            memcpy(data, readData.constData(), readData.size());
            myReadOffset += readData.size();
            //qDebug() << "read" << readData.size() << "bytes";
        }

        return readData.size() > 0 ? readData.size()
                                   : -1;
    }
}

qint64 FtpFile::writeData(const char* data, qint64 len)
{
    qDebug() << Q_FUNC_INFO << len;
    return myWriteBuffer.write(data, len);
}

void FtpFile::slotRetrieveFinished(bool success)
{
    qDebug() << Q_FUNC_INFO << success;
    if (! success)
    {
        myHasError = true;
    }
    myWaitingForDownload = false;

}
