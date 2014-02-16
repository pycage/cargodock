#include "placesmodel.h"
#include "developermode.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#include <QDebug>

#include <sailfishapp.h>

namespace
{
const QString SD_CARD("/run/user/100000/media/sdcard");
const QString ANDROID_STORAGE("/data/sdcard");
}

PlacesModel::PlacesModel(QObject* parent)
    : FolderBase(parent)
{

}

void PlacesModel::init()
{
    setConfigValue("path", "Places");
    setConfigValue("local-services", QStringList() << "local0"
                                                   << "local1"
                                                   << "local2"
                                                   << "local3"
                                                   << "local4"
                                                   << "local5"
                                                   << "local6"
                                                   << "local7");

    setConfigValue("local0", "type", "local");
    setConfigValue("local0", "name", "Documents");
    setConfigValue("local0", "icon", "image://theme/icon-m-document");
    setConfigValue("local0", "path", QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0]);

    setConfigValue("local1", "type", "local");
    setConfigValue("local1", "name", "Downloads");
    setConfigValue("local1", "icon", "image://theme/icon-m-cloud-download");
    setConfigValue("local1", "path", QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]);

    setConfigValue("local2", "type", "local");
    setConfigValue("local2", "name", "Music");
    setConfigValue("local2", "icon", "image://theme/icon-m-music");
    setConfigValue("local2", "path", QStandardPaths::standardLocations(QStandardPaths::MusicLocation)[0]);

    setConfigValue("local3", "type", "local");
    setConfigValue("local3", "name", "Videos");
    setConfigValue("local3", "icon", "image://theme/icon-m-video");
    setConfigValue("local3", "path", QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0]);

    setConfigValue("local4", "type", "local");
    setConfigValue("local4", "name", "Pictures");
    setConfigValue("local4", "icon", "image://theme/icon-m-image");
    setConfigValue("local4", "path", QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0]);

    setConfigValue("local5", "type", "local");
    setConfigValue("local5", "name", "Camera");
    setConfigValue("local5", "icon", "image://theme/icon-m-camera");
    setConfigValue("local5", "path", QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0] + "/Camera");

    setConfigValue("local6", "type", "local");
    setConfigValue("local6", "name", "SD Card");
    setConfigValue("local6", "icon", "image://theme/icon-m-device");
    setConfigValue("local6", "path", SD_CARD);

    setConfigValue("local7", "type", "local");
    setConfigValue("local7", "name", "Android Storage");
    setConfigValue("local7", "icon", "image://theme/icon-m-folder");
    setConfigValue("local7", "path", ANDROID_STORAGE);

    setConfigValue("developer-services", QStringList() << "developer0");

    setConfigValue("developer0", "type", "local");
    setConfigValue("developer0", "name", "System");
    setConfigValue("developer0", "icon", "image://theme/icon-m-folder");
    setConfigValue("developer0", "path", "/");
    setConfigValue("developer0", "showHidden", true);
}

void PlacesModel::addService(const QString& serviceName,
                             const QString& icon,
                             const QString& name,
                             const QVariantMap& properties)
{
    QStringList services = configValue("user-services").toStringList();
    const QString uid = QString("user%1").arg(services.size());

    services << uid;
    setConfigValue("user-services", services);

    setConfigValue(uid, "type", serviceName);
    setConfigValue(uid, "name", name);
    setConfigValue(uid, "icon", icon);
    setConfigValue(uid, "path", "/");

    foreach (const QString& prop, properties.keys())
    {
        setConfigValue(uid, prop, properties.value(prop));
    }

    emit servicesChanged();
}

void PlacesModel::removeService(const QString& uid)
{
    QStringList services = configValue("user-services").toStringList();
    services.removeOne(uid);
    setConfigValue("user-services", services);
    emit servicesChanged();
}

QStringList PlacesModel::services() const
{
    return QStringList() << configValue("local-services").toStringList()
                         << configValue("developer-services").toStringList()
                         << configValue("user-services").toStringList();
}

QVariantMap PlacesModel::service(const QString& uid) const
{
    QVariantMap data;
    data["uid"] = uid;
    data["type"] = configValue(uid, "type");
    data["name"] = configValue(uid, "name");

    return data;
}

int PlacesModel::rowCount(const QModelIndex&) const
{
    return myPlaces.size();
}

QVariant PlacesModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= myPlaces.size())
    {
        return QVariant();
    }

    Item::ConstPtr item = myPlaces.at(index.row());

    switch (role)
    {
    case NameRole:
        return item->name;
    case SectionRole:
        return item->section;
    case IconRole:
        return item->icon;
    case LinkTargetRole:
        return item->uid;
    case ModelTargetRole:
        return item->type;
    case SelectedRole:
        return isSelected(index.row());
    case MtimeRole:
        return QDateTime();
    default:
        return QVariant();
    }
}

int PlacesModel::capabilities() const
{
    return AcceptBookmark | CanDelete;
}

bool PlacesModel::linkFile(const QString& path, const QString& source)
{
    qDebug() << Q_FUNC_INFO << path << source;

    QStringList localServices = configValue("bookmark-services").toStringList();
    const QString uid = QString("bookmark%1").arg(localServices.size());

    localServices << uid;
    setConfigValue("bookmark-services", localServices);

    setConfigValue(uid, "type", "local");
    setConfigValue(uid, "name", source);
    setConfigValue(uid, "icon", "image://theme/icon-m-folder");
    setConfigValue(uid, "path", source);

    return true;
}

void PlacesModel::loadDirectory(const QString&)
{
    beginResetModel();
    myPlaces.clear();

    DeveloperMode developerMode;

    QStringList localServices = configValue("local-services").toStringList();
    QStringList bookmarkServices = configValue("bookmark-services").toStringList();
    QStringList developerServices = configValue("developer-services").toStringList();
    QStringList userServices = configValue("user-services").toStringList();

    foreach (const QString& uid, localServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();

        if (QDir(path).exists())
        {
            myPlaces << Item::Ptr(new Item(uid, name, "", icon, type));
        }
    }

    foreach (const QString& uid, bookmarkServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();

        if (QDir(path).exists())
        {
            myPlaces << Item::Ptr(new Item(uid, name, "Bookmarks", icon, type));
        }
    }

    if (developerMode.enabled())
    {
        foreach (const QString& uid, developerServices)
        {
            const QString type = configValue(uid, "type").toString();
            const QString name = configValue(uid, "name").toString();
            const QString icon = configValue(uid, "icon").toString();

            myPlaces << Item::Ptr(new Item(uid, name, "Developer Mode", icon, type));
        }
    }

    foreach (const QString& uid, userServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();

        myPlaces << Item::Ptr(new Item(uid, name, "Cloud", icon, type));
    }

    endResetModel();
}

QString PlacesModel::itemName(int idx) const
{
    if (idx < myPlaces.size())
    {
        return myPlaces[idx]->name;
    }
    else
    {
        return QString();
    }
}
