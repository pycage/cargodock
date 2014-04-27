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

    virtual QString parentPath(const QString& path) const { return path; }
    virtual QString basename(const QString& path) const { return path; }
    virtual QString joinPath(const QStringList& parts) const { return parts.last(); }

    int capabilities() const;

    virtual bool linkFile(const QString& path,
                          const QString& source,
                          const FolderBase* sourceModel);

    virtual bool deleteFile(const QString& path);

signals:
    void servicesChanged();

protected:
    virtual void init();

    virtual void loadDirectory(const QString& path);

private:
    QStringList services() const;
    Item::Ptr makeItem(const QString& uid,
                       const QString& name,
                       const QString& section,
                       const QString& icon,
                       const QString& type,
                       bool selectable);
};

#endif // PLACESMODEL_H
