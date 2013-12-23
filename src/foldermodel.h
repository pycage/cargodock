#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include "folderbase.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSharedPointer>

class FolderModel : public FolderBase
{
    Q_OBJECT


public:
    FolderModel(QObject* parent = 0);

    virtual int rowCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual bool isReadable() const { return myIsReadable; }
    virtual bool isWritable() const { return myIsWritable; }

    Q_INVOKABLE virtual void setPermissions(const QString& name, int permissions);
    Q_INVOKABLE virtual void rename(const QString& name, const QString& newName);

    virtual QString basename(const QString& path) const;
    virtual QString userBasename(const QString& path) const;
    virtual QString joinPath(const QStringList& parts) const;
    virtual QString parentPath(const QString& path) const;

    virtual QStringList list(const QString& path) const;
    virtual ItemType type(const QString& path) const;
    virtual QIODevice* openFile(const QString& path,
                                QIODevice::OpenModeFlag mode);
    virtual bool makeDirectory(const QString& path);
    virtual bool linkFile(const QString& path, const QString& source);
    virtual bool deleteFile(const QString& path);
    virtual void runFile(const QString& path);

protected:
    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;

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
        QString owner;
        QString group;
        int permissions;
        QString linkTarget;
    };

private:
    QMap<QString, QString> myIcons;
    QList<Item::Ptr> myItems;
    bool myIsReadable;
    bool myIsWritable;
};

#endif // FOLDERMODEL_H
