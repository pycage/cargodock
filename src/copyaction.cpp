#include "copyaction.h"
#include "folderbase.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QDebug>

class CopyThread : public QThread
{
public:
    CopyThread(QIODevice* source, QIODevice* dest)
        : QThread()
        , mySource(source)
        , myDestination(dest)
        , myHasFailed(false)
    {

    }

    bool hasFailed() const { return myHasFailed; }

protected:
    virtual void run()
    {
        qDebug() << "start thread" << mySource->isOpen() << mySource->isReadable();
        while (mySource &&
               mySource->isOpen() &&
               mySource->isReadable())
        {
            if (myDestination &&
                myDestination->isOpen() &&
                myDestination->isWritable())
            {
                qint64 bytes = myDestination->write(mySource->read(4096));
                if (bytes == 0)
                {
                    // done
                    break;
                }
                else if (bytes < 0)
                {
                    myHasFailed = true;
                    break;
                }
            }
            else
            {
                myHasFailed = true;
                break;
            }
        }
        mySource->close();
        myDestination->close();
    }

private:
    QIODevice* mySource;
    QIODevice* myDestination;
    bool myHasFailed;
};




CopyJob::CopyJob(QIODevice* source,
                 QIODevice* dest,
                 const QString& name,
                 qint64 size)
    : QObject()
    , mySource(source)
    , myDestination(dest)
    , myName(name)
    , mySize(size)
    , myBytesWritten(0)
    , myHasFailed(false)
{

}

void CopyJob::start()
{
    emit progress(myName, 0);

    if (mySource &&
            mySource->isOpen() &&
            mySource->isReadable() &&
            myDestination &&
            myDestination->isOpen() &&
            myDestination->isWritable())
    {
        run();
    }
    else
    {
        close();
    }
}

void CopyJob::close()
{
    mySource->close();
    myDestination->close();
    mySource->deleteLater();
    myDestination->deleteLater();
    emit finished();
}

void CopyJob::run()
{
    QTime now;
    now.start();
    while (now.elapsed() < 5)
    {
        QString noError = mySource->errorString();
        QByteArray data = mySource->read(1024 * 1024);
        if (mySource->errorString() != noError)
        {
            qDebug() << "error reading data";
            myHasFailed = true;
            myError = mySource->errorString();
            close();
            return;
        }
        else if (data.isEmpty())
        {
            // done
            qDebug() << Q_FUNC_INFO << "end of file";
            close();
            return;
        }
        else
        {
            qint64 bytes = myDestination->write(data);
            qDebug() << Q_FUNC_INFO << bytes << "bytes copied";
            if (bytes == -1)
            {
                qDebug() << "error writing data";
                myHasFailed = true;
                myError = myDestination->errorString();
                close();
                return;
            }

            myBytesWritten += bytes;
            emit progress(myName, myBytesWritten / (double) mySize);
        }
    }
    QTimer::singleShot(1, this, SLOT(run()));
}



CopyAction::CopyAction(FolderBase* source,
                       FolderBase* dest,
                       const QList<QString>& sourcePaths,
                       const QString& destPath)
    : mySource(source->clone())
    , myDestination(dest)
    , mySourcePaths(sourcePaths)
    , myDestinationPath(destPath)
    , myCopyThread(0)
    , myCopyJob(0)
{

}

CopyAction::~CopyAction()
{
    qDebug() << Q_FUNC_INFO;
    mySource->deleteLater();
}

void CopyAction::start()
{
    if (! mySource)
    {
        emit error("Source does not support copying.");
        return;
    }

    for (int i = 0; i < mySource->rowCount(QModelIndex()); ++i)
    {
        const QString name =
                mySource->data(mySource->index(i), FolderBase::NameRole)
                .toString();
        FolderBase::ItemType type =
                (FolderBase::ItemType)
                mySource->data(mySource->index(i), FolderBase::TypeRole)
                .toInt();
        qint64 size = mySource->data(mySource->index(i), FolderBase::SizeRole)
                .toLongLong();
        const QString fullPath =
                mySource->joinPath(QStringList() << mySource->path() << name);

        qDebug() << "types" << fullPath << type;
        myTypeMap.insert(fullPath, type);
        mySizeMap.insert(fullPath, size);
    }
    QTimer::singleShot(0, this, SLOT(slotProcessNext()));
}

void CopyAction::copy(const QString& sourcePath, const QString& destPath)
{
    qDebug() << Q_FUNC_INFO << sourcePath << "->" << destPath;
    qDebug() << "type of" << sourcePath << myTypeMap.value(sourcePath);
    if (myTypeMap.value(sourcePath, FolderBase::Unsupported) /*mySource->type(sourcePath)*/ == FolderBase::Folder)
    {
        qDebug() << "folder type";
        const QString fileName = mySource->basename(sourcePath);
        const QString destDir = myDestination->joinPath(
                    QStringList() << destPath << fileName);

        myDestination->makeDirectory(destDir);

        mySource->setPath(sourcePath);
        QEventLoop loop;
        qDebug() << "loading" << sourcePath;
        while (mySource->property("loading").toBool())
        {
            loop.processEvents();
        }
        qDebug() << "loading finished" << sourcePath;

        for (int i = 0; i < mySource->rowCount(QModelIndex()); ++i)
        {
            const QString name =
                    mySource->data(mySource->index(i), FolderBase::NameRole)
                    .toString();
            FolderBase::ItemType type =
                    (FolderBase::ItemType)
                    mySource->data(mySource->index(i), FolderBase::TypeRole)
                    .toInt();
            qint64 size = mySource->data(mySource->index(i), FolderBase::SizeRole)
                    .toLongLong();
            const QString fullPath =
                    mySource->joinPath(QStringList() << sourcePath << name);

            qDebug() << "types" << fullPath << type;
            myCopyPaths << QPair<QString, QString>(fullPath, destDir);
            myTypeMap.insert(fullPath, type);
            mySizeMap.insert(fullPath, size);
        }

        QTimer::singleShot(0, this, SLOT(slotProcessNext()));
    }
    else
    {
        qDebug() << "file type";
        if (! myCopyJob)
        {
            qint64 size = mySizeMap.value(sourcePath, 0);
            qDebug() << "size" << size;

            const QString& destFile =
                    myDestination->joinPath(QStringList()
                                            << destPath
                                            << mySource->basename(sourcePath));
            QIODevice* srcFd = mySource->openFile(sourcePath,
                                                  size,
                                                  QIODevice::ReadOnly);
            QIODevice* destFd = myDestination->openFile(destFile,
                                                        size,
                                                        QIODevice::WriteOnly);

            // TODO: check if source and dest are the same to improve copy

            if (srcFd && destFd && srcFd->isOpen() && destFd->isOpen())
            {
                myCopyJob = new CopyJob(srcFd,
                                        destFd,
                                        mySource->basename(sourcePath),
                                        size);
                connect(myCopyJob, SIGNAL(progress(QString,double)),
                        this, SIGNAL(progress(QString,double)));
                connect(myCopyJob, SIGNAL(finished()),
                        this, SLOT(slotCopyThreadFinished()));
                myCopyJob->start();
            }
            else
            {
                emit error("Could not copy to destination.");
            }
        }
    }
}

void CopyAction::slotProcessNext()
{
    if (myCopyPaths.size())
    {
        const QPair<QString, QString> copyItem = myCopyPaths.takeFirst();
        copy(copyItem.first, copyItem.second);
    }
    else if (mySourcePaths.size())
    {
        const QString sourcePath = mySourcePaths.takeFirst();
        copy(sourcePath, myDestinationPath);
    }
    else
    {
        emit finished();
    }
}

void CopyAction::slotCopyThreadFinished()
{
    if (myCopyJob)
    {
        if (! myCopyJob->hasFailed())
        {
            QTimer::singleShot(0, this, SLOT(slotProcessNext()));
        }
        else
        {
            emit error(myCopyJob->error());
            emit finished();
        }
        delete myCopyJob;
        myCopyJob = 0;
    }
}
