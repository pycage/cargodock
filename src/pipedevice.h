#ifndef PIPEDEVICE_H
#define PIPEDEVICE_H

#include <QByteArray>
#include <QIODevice>

class PipeDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit PipeDevice(QObject* parent = 0);
    virtual ~PipeDevice();

    virtual bool open(OpenMode mode);
    virtual void close();

    virtual bool isSequential() const { return true; }

    virtual qint64 bytesAvailable() const;

protected:
    virtual qint64 readData(char* data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 len);

private:
    QByteArray myBuffer;
    int myOffset;
    bool myIsComplete;
    bool myIsUnread;
};

#endif // PIPEDEVICE_H
