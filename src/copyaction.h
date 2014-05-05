#ifndef COPYACTION_H
#define COPYACTION_H

#include "folderaction.h"
#include "folderbase.h"

#include <QIODevice>
#include <QMap>
#include <QList>
#include <QPair>
#include <QString>

class FolderBase;
class CopyThread;

class CopyJob : public QObject
{
    Q_OBJECT
public:
    CopyJob(QIODevice* source,
            QIODevice* dest,
            const QString& name,
            qint64 size);
    bool hasFailed() const { return myHasFailed; }
    QString error() const { return myError; }
    void start();

signals:
    void progress(const QString& name, double amount);
    void finished();

private:
    void close();

private slots:
    void run();

private:
    QIODevice* mySource;
    QIODevice* myDestination;
    QString myName;
    qint64 mySize;
    qint64 myBytesWritten;
    bool myHasFailed;
    QString myError;
};

class CopyAction : public FolderAction
{
    Q_OBJECT
public:
    CopyAction(FolderBase* source,
               FolderBase* dest,
               const QList<QString>& sourcePaths,
               const QString& destPath);
    virtual ~CopyAction();

    virtual void start();

private:
    void copy(const QString& sourcePath, const QString& destPath);

private slots:
    void slotProcessNext();
    void slotCopyThreadFinished();

private:
    FolderBase* mySource;
    FolderBase* myDestination;
    QList<QString> mySourcePaths;
    QList<QPair<QString, QString> > myCopyPaths;
    QString myDestinationPath;

    QMap<QString, FolderBase::ItemType> myTypeMap;
    QMap<QString, qint64> mySizeMap;

    CopyThread* myCopyThread;
    CopyJob* myCopyJob;
};

#endif // COPYACTION_H
