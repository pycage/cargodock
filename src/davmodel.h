#ifndef DAVMODEL_H
#define DAVMODEL_H

#include "folderbase.h"
#include "davapi/davapi.h"

#include <QSharedPointer>

class DavModel : public FolderBase
{
    Q_OBJECT
public:    
    DavModel(QObject* parent = 0);
    virtual FolderBase* clone() const;

    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual bool isWritable() const { return true; }
    virtual int capabilities() const;

    Q_INVOKABLE virtual void rename(const QString& name, const QString& newName);

    virtual QString friendlyBasename(const QString& path) const;

    virtual QIODevice* openFile(const QString& path,
                                QIODevice::OpenModeFlag mode);

    virtual bool makeDirectory(const QString& path);
    virtual bool deleteFile(const QString& path);

protected:
    DavModel(const DavModel& other);

    virtual void init();

    virtual bool loading() const { return myIsLoading; }

    virtual void loadDirectory(const QString& path);

private slots:
    void slotPropertiesReceived(const DavApi::Properties& props);
    void slotMkColFinished(int result);
    void slotDeleteFinished(int result);
    void slotMoveFinished(int result);

private:
    QSharedPointer<DavApi> myDavApi;

    int myMkColResult;
    int myDeleteResult;
    int myMoveResult;

    bool myIsLoading;
};

#endif // DAVMODEL_H
