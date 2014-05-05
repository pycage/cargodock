#ifndef DAVFILE_H
#define DAVFILE_H

#include "../pipedevice.h"

#include <QByteArray>
#include <QIODevice>
#include <QSharedPointer>
#include <QString>

class DavApi;
class QNetworkReply;

class DavFile : public QIODevice
{
    Q_OBJECT
public:
    DavFile(const QString& path,
            qint64 size,
            QSharedPointer<DavApi> api,
            QObject* parent = 0);
    virtual ~DavFile();

    virtual bool open(OpenMode mode);
    virtual void close();

protected:
    virtual qint64 readData(char* data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 len);

private slots:
    void slotResourceReceived(const QString& path,
                              int result);
    void slotPutFinished(const QString& path,
                         int result);

private:
    QSharedPointer<DavApi> myDavApi;
    qint64 mySize;
    QString myPath;
    bool myWaitingForDownload;
    bool myWaitingForUpload;

    PipeDevice myWriteBuffer;
    qint64 myReadOffset;

    QNetworkReply* myReply;

    bool myHasError;
};

#endif // DAVFILE_H
