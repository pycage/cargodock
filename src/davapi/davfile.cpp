#include "davfile.h"
#include "davapi.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QDebug>

DavFile::DavFile(const QString& path,
                 qint64 size,
                 QSharedPointer<DavApi> api,
                 QObject* parent)
    : QIODevice(parent)
    , myDavApi(api)
    , mySize(size)
    , myPath(path)
    , myWaitingForDownload(false)
    , myWaitingForUpload(false)
    , myReadOffset(0)
    , myReply(0)
    , myHasError(false)
{
    connect(myDavApi.data(), SIGNAL(resourceReceived(QString,int)),
            this, SLOT(slotResourceReceived(QString,int)));
    connect(myDavApi.data(), SIGNAL(putFinished(QString,int)),
            this, SLOT(slotPutFinished(QString,int)));
}

DavFile::~DavFile()
{
    qDebug() << Q_FUNC_INFO;
    if (myReply)
    {
        myReply->deleteLater();
    }
}

bool DavFile::open(OpenMode mode)
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
        myDavApi->putResource(myPath, mySize, &myWriteBuffer);
    }

    return success && QIODevice::open(mode);
}

void DavFile::close()
{
    qDebug() << "closing write buffer";
    myWriteBuffer.close();

    // TODO: timeout!
    QEventLoop evLoop;
    qDebug() << Q_FUNC_INFO << myWaitingForDownload << myWaitingForUpload;
    while (myWaitingForDownload || myWaitingForUpload)
    {
        evLoop.processEvents();
    }
    qDebug() << Q_FUNC_INFO << "finished";
}

qint64 DavFile::readData(char* data, qint64 maxlen)
{
    qDebug() << Q_FUNC_INFO << maxlen;
    if (! myReply)
    {
        myWaitingForDownload = true;
        myReply = myDavApi->getResource(myPath, myReadOffset, maxlen);
    }

    if (maxlen > 0)
    {
        QEventLoop evLoop;
        while (myReply->bytesAvailable() == 0 && myWaitingForDownload)
        {
            evLoop.processEvents();
        }
    }

    if (myHasError)
    {
        return -1;
    }
    else
    {
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

qint64 DavFile::writeData(const char* data, qint64 len)
{
    //qDebug() << Q_FUNC_INFO << len;
    return myWriteBuffer.write(data, len);
}

void DavFile::slotResourceReceived(const QString& path,
                                   int result)
{
    if (path == myPath)
    {
        if (result >= 200 && result < 300)
        {
            // OK
        }
        else
        {
            myHasError = true;
            setErrorString(QString("Server error: %1").arg(result));
        }
        myWaitingForDownload = false;
    }
}

void DavFile::slotPutFinished(const QString& path,
                              int result)
{
    if (path == myPath)
    {
        if (result == DavApi::ServerUnreachable)
        {
            // Wow, the connection died. Probably the server crashed when
            // allocating space for our file (stupid servers hold large files
            // in RAM).
            myWriteBuffer.close();
            myHasError = true;
            setErrorString("Connection to server aborted.");
        }
        else if (result >= 200 && result < 300)
        {
            // OK
        }
        else
        {
            myHasError = true;
            setErrorString(QString("Server error: %1").arg(result));
        }
    }
    myWaitingForUpload = false;
}
