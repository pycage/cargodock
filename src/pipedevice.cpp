#include "pipedevice.h"

#include <QDebug>
#include <QEventLoop>

PipeDevice::PipeDevice(QObject* parent)
    : QIODevice(parent)
    , myOffset(0)
    , myIsComplete(false)
    , myIsUnread(false)
{
    open(QIODevice::ReadWrite);
}

PipeDevice::~PipeDevice()
{
    qDebug() << Q_FUNC_INFO;
}

bool PipeDevice::open(OpenMode mode)
{
    return QIODevice::open(mode);
}

void PipeDevice::close()
{
    if (! myIsComplete)
    {
        myIsComplete = true;
        emit readyRead();
    }

    // wait until the buffer was read
    /*
    qDebug() << Q_FUNC_INFO;
    while (myIsUnread)
    {
        QEventLoop ev;
        ev.processEvents();
    }
    */
    qDebug() << Q_FUNC_INFO << "finished";

    QIODevice::close();
}

qint64 PipeDevice::bytesAvailable() const
{
    qint64 bytes =  (myBuffer.size() - myOffset) + QIODevice::bytesAvailable();
    //qDebug() << Q_FUNC_INFO << bytes;
    return bytes;
}

qint64 PipeDevice::readData(char* data, qint64 maxlen)
{
    if (myIsComplete && myBuffer.isEmpty())
    {
        qDebug() << "EOF";
        myIsUnread = false;
        return -1;
    }
    else if (myBuffer.isEmpty())
    {
        myIsUnread = false;
        return 0;
    }
    else
    {
        QByteArray newData = myBuffer.mid(myOffset, maxlen);
        int size = newData.size();
        myOffset += size;
        memcpy(data, newData.constData(), size);
        //qDebug() << "pipe read data" << size << "maxlen" << maxlen;
        //qDebug() << "data:" << newData.toHex();

        if (myOffset >= myBuffer.size())
        {
            // all read
            myBuffer.clear();
            myOffset = 0;
            myIsUnread = false;
        }

        return size;
    }
}

qint64 PipeDevice::writeData(const char* data, qint64 len)
{
    qDebug() << "pipe write data" << len << "bytes";

    if (myIsComplete)
    {
        // accept nothing after the stream was terminated
        return 0;
    }
    else if (len == 0)
    {
        // writing 0 bytes terminates the stream
        myIsComplete = true;
        emit readyRead();
    }
    else
    {
        myIsUnread = true;
        myBuffer.append(data, len);
        qDebug() << "written";
        emit readyRead();

        // wait until the buffer was read
        while (myIsUnread && ! myIsComplete)
        {
            QEventLoop ev;
            ev.processEvents();
        }
    }

    return len;
}
