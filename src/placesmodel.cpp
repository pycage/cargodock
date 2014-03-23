#include "placesmodel.h"
#include "developermode.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

#include <QDebug>

#include <sailfishapp.h>

namespace
{
const QString LEGACY_SD_CARD("/run/user/100000/media/sdcard");
const QString SD_CARD_BASE("/media/sdcard");
const QString ANDROID_STORAGE("/data/sdcard");

QString makeUid()
{
    return QUuid::createUuid().toString();
}

QString sdPath()
{
    if (QDir(LEGACY_SD_CARD).exists())
    {
        return LEGACY_SD_CARD;
    }

    QStringList partitions;
    QDir baseDir(SD_CARD_BASE);
    foreach (const QString& partition,
             baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (QFile(QString("/dev/disk/by-uuid/%1").arg(partition)).exists())
        {
            partitions << partition;
        }
    }

    qDebug() << "PARTITIONS" << partitions;
    if (partitions.size() == 1)
    {
        return SD_CARD_BASE + "/" + partitions.at(0);
    }
    else if (partitions.empty())
    {
        return QString();
    }
    else
    {
        return SD_CARD_BASE;
    }

}

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
                                                   << "local5");

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

    setConfigValue("storage-services", QStringList() << "storage0"
                                                     << "storage1");

    // FIXME: we need proper SD card handling, probably with a new folder model
    setConfigValue("storage0", "type", "local");
    setConfigValue("storage0", "name", "SD Card");
    setConfigValue("storage0", "icon", "image://theme/icon-m-device");
    setConfigValue("storage0", "path", sdPath());

    setConfigValue("storage1", "type", "local");
    setConfigValue("storage1", "name", "Android Storage");
    setConfigValue("storage1", "icon", "image://theme/icon-m-folder");
    setConfigValue("storage1", "path", ANDROID_STORAGE);

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
    const QString uid = makeUid();

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
    removeConfigValues(uid);
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
    case SelectableRole:
        return item->selectable;
    case MtimeRole:
        return QDateTime();
    default:
        return FolderBase::data(index, role);
    }
}

int PlacesModel::capabilities() const
{
    int caps = AcceptBookmark;
    if (selected() > 0)
    {
        caps |= CanDelete;
    }
    return caps;
}

bool PlacesModel::linkFile(const QString& path,
                           const QString& source,
                           const FolderBase* sourceModel)
{
    qDebug() << Q_FUNC_INFO << path << source;

    const QString sourceUid = sourceModel->uid();
    const QString uid = makeUid();
    cloneConfigValues(sourceUid, uid);
    setConfigValue(uid, "name", sourceModel->basename(source));
    setConfigValue(uid, "path", source);

    QStringList bookmarkServices = configValue("bookmark-services").toStringList();
    bookmarkServices << uid;
    setConfigValue("bookmark-services", bookmarkServices);

    return true;
}

bool PlacesModel::deleteFile(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;

    int idx = 0;
    foreach (Item::ConstPtr item, myPlaces)
    {
        if (item->uid == path)
        {
            beginRemoveRows(QModelIndex(), idx, idx);
            QStringList bookmarkServices = configValue("bookmark-services").toStringList();
            bookmarkServices.removeAll(item->uid);
            setConfigValue("bookmark-services", bookmarkServices);

            removeConfigValues(item->uid);
            myPlaces.removeAt(idx);
            endRemoveRows();

            return true;
        }
        ++idx;
    }

    return false;
}

void PlacesModel::loadDirectory(const QString&)
{
    beginResetModel();
    myPlaces.clear();

    DeveloperMode developerMode;

    QStringList bookmarkServices = configValue("bookmark-services").toStringList();
    QStringList localServices = configValue("local-services").toStringList();
    QStringList storageServices = configValue("storage-services").toStringList();
    QStringList developerServices = configValue("developer-services").toStringList();
    QStringList userServices = configValue("user-services").toStringList();

    foreach (const QString& uid, bookmarkServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();
        qDebug() << "service" << type << name << path;

        myPlaces << Item::Ptr(new Item(uid, name, "Bookmarks", icon, type, true));
    }

    foreach (const QString& uid, localServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();

        if (path.size() && QDir(path).exists())
        {
            myPlaces << Item::Ptr(new Item(uid, name, "Media", icon, type, false));
        }
    }

    foreach (const QString& uid, storageServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();

        if (path.size() && QDir(path).exists())
        {
            myPlaces << Item::Ptr(new Item(uid, name, "Storage", icon, type, false));
        }
    }

    if (developerMode.enabled())
    {
        foreach (const QString& uid, developerServices)
        {
            const QString type = configValue(uid, "type").toString();
            const QString name = configValue(uid, "name").toString();
            const QString icon = configValue(uid, "icon").toString();

            myPlaces << Item::Ptr(new Item(uid, name, "Developer Mode", icon, type, false));
        }
    }

    foreach (const QString& uid, userServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();

        myPlaces << Item::Ptr(new Item(uid, name, "Cloud", icon, type, false));
    }

    endResetModel();
}

QString PlacesModel::itemName(int idx) const
{
    if (idx < myPlaces.size())
    {
        return myPlaces.at(idx)->uid;
    }
    else
    {
        return QString();
    }
}
