#ifndef FTPFILE_H
#define FTPFILE_H

#include "../pipedevice.h"

#include <QIODevice>
#include <QSharedPointer>

class FtpApi;

class FtpFile : public QIODevice
{
    Q_OBJECT
public:
    FtpFile(const QString& path,
            QSharedPointer<FtpApi> api,
            QObject* parent = 0);
    virtual ~FtpFile();

    virtual bool open(OpenMode mode);
    virtual void close();

protected:
    virtual qint64 readData(char* data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 len);

private slots:
    void slotRetrieveFinished(bool success);

private:
    QSharedPointer<FtpApi> myFtpApi;

    QString myPath;

    bool myWaitingForDownload;
    bool myWaitingForUpload;

    PipeDevice myWriteBuffer;
    qint64 myReadOffset;

    QIODevice* myReply;
    bool myHasError;
};

#endif // FTPFILE_H
