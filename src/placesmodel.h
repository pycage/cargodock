#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include "folderbase.h"

#include <QList>
#include <QSharedPointer>

class PlacesModel : public FolderBase
{
    Q_OBJECT
public:
    PlacesModel(QObject* parent = 0);

    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex&, int) const;

    virtual QString parentPath(const QString& path) const { return path; }
    virtual QString basename(const QString& path) const { return path; }
    virtual QString joinPath(const QStringList& parts) const { return QString(); }

    virtual ItemType type(const QString&) const { return Folder; }

protected:
    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;

private:
    struct Item
    {
        typedef QSharedPointer<Item> Ptr;
        typedef QSharedPointer<const Item> ConstPtr;

        Item(const QString& n, const QString& i, const QString& m,
             const QString& p)
            : name(n)
            , icon(i)
            , model(m)
            , path(p)
        { }

        QString name;
        QString icon;
        QString model;
        QString path;
    };

    QList<Item::Ptr> myPlaces;
};

#endif // PLACESMODEL_H
