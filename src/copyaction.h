#ifndef COPYACTION_H
#define COPYACTION_H

#include "folderaction.h"
#include <QList>
#include <QPair>
#include <QString>

class FolderBase;
class CopyThread;

class CopyAction : public FolderAction
{
    Q_OBJECT
public:
    CopyAction(FolderBase* source,
               FolderBase* dest,
               const QList<QString>& sourcePaths,
               const QString& destPath);

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

    CopyThread* myCopyThread;
};

#endif // COPYACTION_H
