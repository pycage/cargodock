#ifndef DAVMODEL_H
#define DAVMODEL_H

#include "folderbase.h"

#include <QDateTime>

class DavModel : public FolderBase
{
    Q_OBJECT
public:
    DavModel(QObject* parent = 0);

    virtual int rowCount(const QModelIndex &) const { return myItems.size(); }
    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual QString parentPath(const QString& path) const;
    virtual QString basename(const QString& path) const;
    virtual QString userBasename(const QString& path) const;
    virtual QString joinPath(const QStringList& parts) const;

    virtual ItemType type(const QString& path) const;

protected:
    virtual bool loading() const { return myIsLoading; }

    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;

private slots:
    void slotMetaDataReceived();

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

    QList<Item::Ptr> myItems;

    bool myIsLoading;
};

#endif // DAVMODEL_H
