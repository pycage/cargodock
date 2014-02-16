#include "copyaction.h"
#include "folderbase.h"

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




CopyJob::CopyJob(QIODevice* source, QIODevice* dest)
    : QObject()
    , mySource(source)
    , myDestination(dest)
    , myHasFailed(false)
{

}

void CopyJob::start()
{
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
    emit finished();
}

void CopyJob::run()
{
    QTime now;
    now.start();
    qDebug() << Q_FUNC_INFO;
    while (now.elapsed() < 5)
    {
        qint64 bytes = myDestination->write(mySource->read(1024 * 1024));
        if (bytes == 0)
        {
            // done
            close();
            return;
        }
        else if (bytes < 0)
        {
            myHasFailed = true;
            close();
            return;
        }
    }
    QTimer::singleShot(1, this, SLOT(run()));
}



CopyAction::CopyAction(FolderBase* source,
                       FolderBase* dest,
                       const QList<QString>& sourcePaths,
                       const QString& destPath)
    : mySource(source)
    , myDestination(dest)
    , mySourcePaths(sourcePaths)
    , myDestinationPath(destPath)
    , myCopyThread(0)
    , myCopyJob(0)
{

}

void CopyAction::start()
{
    QTimer::singleShot(0, this, SLOT(slotProcessNext()));
}

void CopyAction::copy(const QString& sourcePath, const QString& destPath)
{
    qDebug() << Q_FUNC_INFO << sourcePath << "->" << destPath;
    if (mySource->type(sourcePath) == FolderBase::Folder)
    {
        qDebug() << "folder type";
        const QString fileName = mySource->basename(sourcePath);
        const QString destDir = myDestination->joinPath(
                    QStringList() << destPath << fileName);

        myDestination->makeDirectory(destDir);

        QStringList paths = mySource->list(sourcePath);
        qDebug() << "paths" << paths;
        foreach (const QString& path, paths)
        {
            myCopyPaths << QPair<QString, QString>(
                        mySource->joinPath(QStringList() << sourcePath << path),
                        destDir);
        }
        QTimer::singleShot(0, this, SLOT(slotProcessNext()));
    }
    else
    {
        qDebug() << "file type";
        if (! myCopyJob)
        {
            const QString& destFile =
                    myDestination->joinPath(QStringList()
                                            << destPath
                                            << mySource->basename(sourcePath));
            QIODevice* srcFd = mySource->openFile(sourcePath,
                                                  QIODevice::ReadOnly);
            QIODevice* destFd = myDestination->openFile(destFile,
                                                        QIODevice::WriteOnly);

            // TODO: check if source and dest are the same to improve copy

            if (srcFd && destFd && srcFd->isOpen() && destFd->isOpen())
            {
                myCopyJob = new CopyJob(srcFd, destFd);
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
            qDebug() << "copying succeeded";
            QTimer::singleShot(0, this, SLOT(slotProcessNext()));
        }
        else
        {
            qDebug() << "copying failed";
            emit finished();
        }
        delete myCopyJob;
        myCopyJob = 0;
    }
}
