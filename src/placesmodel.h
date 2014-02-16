#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include "folderbase.h"

#include <QList>
#include <QSharedPointer>
#include <QVariantMap>

class PlacesModel : public FolderBase
{
    Q_OBJECT
    Q_PROPERTY(QStringList services READ services NOTIFY servicesChanged)
public:
    PlacesModel(QObject* parent = 0);

    /* Adds a service instance with icon and name.
     */
    Q_INVOKABLE void addService(const QString& serviceName,
                                const QString& icon,
                                const QString& name,
                                const QVariantMap& properties);

    /* Removes the given service instance.
     */
    Q_INVOKABLE void removeService(const QString& uid);

    /* Lists the UIDs of the current service instances.
     */
    Q_INVOKABLE QVariantMap service(const QString& uid) const;

    virtual int rowCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex&, int) const;

    virtual QString parentPath(const QString& path) const { return path; }
    virtual QString basename(const QString& path) const { return path; }
    virtual QString joinPath(const QStringList& parts) const { return QString(); }

    int capabilities() const;

    virtual ItemType type(const QString&) const { return Folder; }

    virtual bool linkFile(const QString& path, const QString& source);

signals:
    void servicesChanged();

protected:
    virtual void init();

    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;

private:
    QStringList services() const;

private:
    struct Item
    {
        typedef QSharedPointer<Item> Ptr;
        typedef QSharedPointer<const Item> ConstPtr;

        Item(const QString& u,
             const QString& n,
             const QString& s,
             const QString& i,
             const QString& t)
            : uid(u)
            , name(n)
            , section(s)
            , icon(i)
            , type(t)
        { }

        QString uid;
        QString name;
        QString section;
        QString icon;
        QString type;
    };

    QList<Item::Ptr> myPlaces;
};

#endif // PLACESMODEL_H
