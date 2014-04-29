#include "davfile.h"
#include "davapi.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QDebug>

DavFile::DavFile(const QString& path,
                 QSharedPointer<DavApi> api,
                 QObject* parent)
    : QIODevice(parent)
    , myDavApi(api)
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
        myDavApi->putResource(myPath, &myWriteBuffer);
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

    qDebug() << "here";
    if (maxlen > 0)
    {
        QEventLoop evLoop;
        while (myReply->bytesAvailable() == 0 && myWaitingForDownload)
        {
            evLoop.processEvents();
        }
    }

    qDebug() << "there";
    if (myHasError)
    {
        return -1;
    }
    else
    {
        qDebug() << "try read";
        QByteArray readData = myReply->read(maxlen);
        qDebug() << "read";
        if (readData.size() > 0)
        {
            memcpy(data, readData.constData(), readData.size());
            myReadOffset += readData.size();
            qDebug() << "read" << readData.size() << "bytes";
        }

        return readData.size() > 0 ? readData.size()
                                   : -1;
    }
}

qint64 DavFile::writeData(const char* data, qint64 len)
{
    qDebug() << Q_FUNC_INFO << len;
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
        }
        myWaitingForDownload = false;
    }
}

void DavFile::slotPutFinished(const QString& path,
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
        }
    }
    myWaitingForUpload = false;
}
