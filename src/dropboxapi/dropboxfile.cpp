#include "dropboxfile.h"
#include "dropboxapi.h"

#include <QEventLoop>

#include <QDebug>

DropboxFile::DropboxFile(const QString& path,
                         QSharedPointer<DropboxApi> dropboxApi,
                         QObject* parent)
    : QIODevice(parent)
    , myDropboxApi(dropboxApi)
    , myPath(path)
    , myWaitingForUploadId(false)
    , myWaitingForCommit(false)
    , myWaitingForDownload(false)
    , myReadOffset(0)
    , myWriteOffset(0)
    , myIsEof(false)
    , myHasError(false)
{
    connect(myDropboxApi.data(), SIGNAL(uploadCreated(QString,QString)),
            this, SLOT(slotUploadCreated(QString,QString)));
    connect(myDropboxApi.data(), SIGNAL(uploaded(QString,bool)),
            this, SLOT(slotUploaded(QString,bool)));
    connect(myDropboxApi.data(), SIGNAL(uploadCommitted(QString,bool)),
            this, SLOT(slotUploadCommitted(QString,bool)));
    connect(myDropboxApi.data(), SIGNAL(downloaded(QString,QByteArray,qint64,bool)),
            this, SLOT(slotDownloaded(QString,QByteArray,qint64,bool)));
}

DropboxFile::~DropboxFile()
{

}

bool DropboxFile::open(OpenMode mode)
{
    bool success = false;
    myReadOffset = 0;
    myWriteOffset = 0;
    myReadBuffer.clear();
    myWriteBuffer.clear();
    myIsEof = false;

    if (mode & QIODevice::ReadOnly)
    {
        success = true;
    }

    if (mode & QIODevice::WriteOnly)
    {
        myWaitingForUploadId = true;
        myDropboxApi->createUpload(myPath);
        // this action needs to be synchronous, so wait for the response
        QEventLoop evLoop;
        while (myWaitingForUploadId)
        {
            evLoop.processEvents();
        }
        success = ! myUploadId.isEmpty();
    }

    return success && QIODevice::open(mode);
}

void DropboxFile::close()
{
    // commit
    if (myUploadId.size() && flush())
    {
        myWaitingForCommit = true;
        myDropboxApi->commitUpload(myUploadId, myPath, myWriteOffset);
        // this action needs to be synchronous, so wait for the response
        QEventLoop evLoop;
        while (myWaitingForCommit)
        {
            evLoop.processEvents();
        }
    }
    QIODevice::close();
}

qint64 DropboxFile::readData(char* data, qint64 maxlen)
{
    qDebug() << Q_FUNC_INFO << maxlen;
    myReadBuffer.clear();

    if (myIsEof)
    {
        return -1;
    }
    else if (maxlen > 0)
    {
        myWaitingForDownload = true;
        myDropboxApi->download(myPath, myReadOffset, myReadOffset + maxlen - 1);
        // this action needs to be synchronous, so wait for the response
        QEventLoop evLoop;
        while (myWaitingForDownload)
        {
            evLoop.processEvents();
        }
        if (myHasError || myReadBuffer.isEmpty())
        {
            return -1;
        }
        else
        {
            qDebug() << "memcpy" << myReadBuffer.size();
            memcpy(data, myReadBuffer.constData(), myReadBuffer.size());
            return myReadBuffer.size();
        }
    }
    else
    {
        return 0;
    }
}

qint64 DropboxFile::writeData(const char* data, qint64 len)
{
    qDebug() << Q_FUNC_INFO << len;
    myWriteBuffer.append(data, len);

    int size = myWriteBuffer.size();
    if (size > 1024 * 1024)
    {
        if (flush())
        {
            return len;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return len;
    }
}

bool DropboxFile::flush()
{
    qDebug() << "flushing cache";
    int size = myWriteBuffer.size();
    if (size > 0)
    {
        myHasError = false;
        myDropboxApi->upload(myUploadId, myWriteOffset, myWriteBuffer);
        // this action needs to be synchronous, so wait for the response
        QEventLoop evLoop;
        while (! myWriteBuffer.isEmpty())
        {
            evLoop.processEvents();
        }
        if (myHasError)
        {
            return false;
        }
        else
        {
            myWriteOffset += size;
            return true;
        }
    }
    else
    {
        return true;
    }

}

void DropboxFile::slotUploadCreated(const QString& identifier,
                                    const QString& uploadId)
{
    if (identifier == myPath)
    {
        qDebug() << Q_FUNC_INFO << identifier << uploadId;
        myUploadId = uploadId;
        myWaitingForUploadId = false;
    }
}

void DropboxFile::slotUploaded(const QString& uploadId, bool ok)
{
    if (uploadId == myUploadId)
    {
        myWriteBuffer.clear();
        myHasError = ! ok;
    }
}

void DropboxFile::slotUploadCommitted(const QString& uploadId, bool ok)
{
    if (uploadId == myUploadId)
    {
        myWaitingForCommit = false;
        myHasError = ! ok;
    }
}

void DropboxFile::slotDownloaded(const QString& path,
                                 const QByteArray& data,
                                 qint64 totalSize,
                                 bool ok)
{
    if (path == myPath)
    {
        if (ok)
        {
            myReadOffset += data.size();
            myReadBuffer.append(data);

            if (myReadOffset >= totalSize)
            {
                myIsEof = true;
            }
        }
        myHasError = ! ok;
        myWaitingForDownload = false;
    }
}
