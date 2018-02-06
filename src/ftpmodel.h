#ifndef FTPMODEL_H
#define FTPMODEL_H

#include "folderbase.h"

#include <QObject>

class FtpApi;

class FtpModel : public FolderBase
{
    Q_OBJECT
public:
    explicit FtpModel(QObject* parent = 0);
    virtual FolderBase* clone() const;

    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual bool isWritable() const { return isValid(); }
    virtual int capabilities() const;

    virtual QString friendlyBasename(const QString& path) const;

    virtual QIODevice* openFile(const QString& path,
                                qint64 size,
                                QIODevice::OpenModeFlag mode);

protected:
    FtpModel(const FtpModel& other);

    virtual void init();

    virtual bool loading() const { return myIsLoading; }

    virtual void loadDirectory(const QString& path);

private slots:
    void slotError(const QString& details);
    void slotListReceived(const QString& data);

private:
    bool myIsLoading;
    QSharedPointer<FtpApi> myFtpApi;
};

#endif // FTPMODEL_H
