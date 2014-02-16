#ifndef DROPBOXFILE_H
#define DROPBOXFILE_H

#include <QByteArray>
#include <QIODevice>
#include <QSharedPointer>
#include <QString>

class DropboxApi;

class DropboxFile : public QIODevice
{
    Q_OBJECT
public:
    DropboxFile(const QString& path,
                QSharedPointer<DropboxApi> dropboxApi,
                QObject* parent = 0);
    virtual ~DropboxFile();

    virtual bool open(OpenMode mode);
    virtual void close();

protected:
    virtual qint64 readData(char* data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 len);

private:
    bool flush();

private slots:
    void slotUploadCreated(const QString& identifier,
                           const QString& uploadId);
    void slotUploaded(const QString& uploadId, bool ok);
    void slotUploadCommitted(const QString& uploadId, bool ok);
    void slotDownloaded(const QString& path, const QByteArray& data,
                        qint64 totalSize, bool ok);

private:
    QSharedPointer<DropboxApi> myDropboxApi;
    QString myPath;
    QString myUploadId;
    bool myWaitingForUploadId;
    bool myWaitingForCommit;
    bool myWaitingForDownload;

    QByteArray myReadBuffer;
    QByteArray myWriteBuffer;
    qint64 myReadOffset;
    qint64 myWriteOffset;

    bool myIsEof;
    bool myHasError;
};

#endif // DROPBOXFILE_H
