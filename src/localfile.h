#ifndef LOCALFILE_H
#define LOCALFILE_H

#include <QFile>
#include <QIODevice>
#include <QString>

class LocalFile : public QIODevice
{
    Q_OBJECT
public:
    LocalFile(const QString& path, QObject* parent = 0);

    virtual bool open(OpenMode mode);
    virtual void close();

protected:
    virtual qint64 readData(char* data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 len);

private:
    QFile myFile;
};

#endif // LOCALFILE_H
