#include "localfile.h"

#include <QEventLoop>

LocalFile::LocalFile(const QString& path, QObject* parent)
    : QIODevice(parent)
    , myFile(path)
{

}

bool LocalFile::open(OpenMode mode)
{
    return myFile.open(mode) && QIODevice::open(mode);
}

void LocalFile::close()
{
    myFile.close();
    QIODevice::close();
}

qint64 LocalFile::readData(char* data, qint64 maxlen)
{
    qint64 totalBytesRead = 0;
    QEventLoop evLoop;
    for (qint64 i = 0; i < maxlen; i += 1024)
    {
        qint64 length = qMin(qint64(1024), maxlen - i);
        qint64 bytesRead = myFile.read(data + i, length);
        if (bytesRead == -1)
        {
            return -1;
        }
        else
        {
            totalBytesRead += bytesRead;
        }
        evLoop.processEvents();
    }

    return totalBytesRead;
}

qint64 LocalFile::writeData(const char* data, qint64 len)
{
    qint64 totalBytesWritten = 0;
    QEventLoop evLoop;
    for (qint64 i = 0; i < len; i += 1024)
    {
        qint64 length = qMin(qint64(1024), len - i);
        qint64 bytesWritten = myFile.write(data + i, length);
        if (bytesWritten == -1)
        {
            return -1;
        }
        else
        {
            totalBytesWritten += bytesWritten;
        }
        evLoop.processEvents();
    }

    return totalBytesWritten;
}
