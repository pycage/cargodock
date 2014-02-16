#ifndef DROPBOXMODEL_H
#define DROPBOXMODEL_H

#include "folderbase.h"
#include "dropboxapi/dropboxapi.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSharedPointer>

class DropboxModel : public FolderBase
{
    Q_OBJECT
public:
    DropboxModel(QObject* parent = 0);

    virtual void init();

    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex&, int) const;

    virtual bool isWritable() const { return true; }
    virtual int capabilities() const { return CanCopy | AcceptCopy | CanDelete; }

    Q_INVOKABLE virtual void rename(const QString& name, const QString& newName);

    virtual QString parentPath(const QString& path) const;
    virtual QString basename(const QString& path) const;
    virtual QString userBasename(const QString& path) const;
    virtual QString joinPath(const QStringList& parts) const;

    virtual ItemType type(const QString& path) const;

    Q_INVOKABLE void authorize(const QUrl& uri);

    virtual QIODevice* openFile(const QString& path,
                                QIODevice::OpenModeFlag mode);
    virtual bool makeDirectory(const QString& path);
    virtual bool deleteFile(const QString& path);

signals:
    void authorizationRequired(const QUrl& url, const QUrl& redirectionUri);

protected:
    virtual bool loading() const { return myIsLoading; }

    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;

private slots:
    void slotAccountInfoReceived(const DropboxApi::AccountInfo& info);
    void slotMetaDataReceived(const DropboxApi::Metadata& metadata);
    void slotFolderCreated(const QString& path);
    void slotFileMoved(const QString& path);
    void slotFileDeleted(const QString& path);

    void slotAuthorized();
    void slotError(DropboxApi::ErrorCode errorCode);

private:
    struct Item
    {
        typedef QSharedPointer<Item> Ptr;
        typedef QSharedPointer<const Item> ConstPtr;

        QString name;
        QString path;
        QString uri;
        ItemType type;
        QString mimeType;
        QString icon;
        qint64 size;
        QDateTime mtime;
        int permissions;
        QString linkTarget;
    };

    QSharedPointer<DropboxApi> myDropboxApi;
    QMap<QString, QString> myMimeTypeIcons;
    QList<Item::Ptr> myItems;

    QString myUserName;

    QString myNewFolderPath;
    QString myRenamedPath;
    QString myDeletedPath;

    bool myIsLoading;
};

#endif // DROPBOXMODEL_H
