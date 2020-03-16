#include "placesmodel.h"
#include "developermode.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

#include <QDebug>

#include <sailfishapp.h>

namespace
{
const QString LEGACY_SD_CARD("/run/user/100000/media/sdcard");
const QString SD_CARD_BASE("/run/media/nemo");
const QString LEGACY_ANDROID_STORAGE("/data/sdcard");
const QString ANDROID_STORAGE(QDir::homePath() + "/android_storage");

QString makeUid()
{
    return QUuid::createUuid().toString();
}

QString sdPath()
{
    QStringList partitions;
    QDir baseDir(SD_CARD_BASE);
    foreach (const QString& partition,
             baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
            partitions << partition;
    }

    qDebug() << "PARTITIONS" << partitions;
    if (partitions.size() )
    {
        return SD_CARD_BASE;
    }
    else
    {
        if (QDir(LEGACY_SD_CARD).exists())
        {
            return LEGACY_SD_CARD;
        }
        return QString();
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
    setConfigValue("storage1", "path",
                   QDir(ANDROID_STORAGE).exists()?
                       ANDROID_STORAGE:
                       QDir(LEGACY_ANDROID_STORAGE).exists()?
                           LEGACY_ANDROID_STORAGE:QString());

    setConfigValue("developer-services", QStringList() << "developer0");

    setConfigValue("developer0", "type", "local");
    setConfigValue("developer0", "name", "System");
    setConfigValue("developer0", "icon", "image://theme/icon-m-folder");
    setConfigValue("developer0", "path", "/");
    setConfigValue("developer0", "showHidden", true);
}

void PlacesModel::addService(const QString& serviceName,
                             const QString& icon,
                             const QVariantMap& properties)
{
    QStringList services = configValue("user-services").toStringList();
    const QString uid = makeUid();

    services << uid;
    setConfigValue("user-services", services);

    setConfigValue(uid, "type", serviceName);
    setConfigValue(uid, "icon", icon);

    updateService(uid, properties);
}

void PlacesModel::updateService(const QString &uid,
                                const QVariantMap& properties)
{
    foreach (const QString& prop, properties.keys())
    {
        setConfigValue(uid, prop, properties.value(prop));
    }

    emit servicesChanged();
}

QVariantMap PlacesModel::serviceProperties(const QString& uid) const
{
    QVariantMap properties;
    foreach (const QString& key, configKeys(uid))
    {
        properties.insert(key, configValue(uid, key));
    }
    return properties;
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

QStringList PlacesModel::tools() const
{
    return QStringList() << configValue("tools").toStringList();
}

QVariantMap PlacesModel::service(const QString& uid) const
{
    QVariantMap data;
    data["uid"] = uid;
    data["type"] = configValue(uid, "type");
    data["name"] = configValue(uid, "name");

    return data;
}

void PlacesModel::addTool(const QVariantMap &options)
{
    QStringList toolList=configValue("tools").toStringList();
    const QString uid = makeUid();
    toolList << uid;
    setConfigValue("tools",toolList);
    updateTool(uid,options);
}

void PlacesModel::updateTool(const QString &uid, const QVariantMap &options)
{
    foreach (const QString& prop, options.keys())
    {
        setConfigValue(uid, prop, options.value(prop));
    }
    emit toolsChanged();
}

void PlacesModel::removeTool(const QString &uid)
{
    QStringList toolList=configValue("tools").toStringList();
    toolList.removeOne(uid);
    setConfigValue("tools",toolList);
    removeConfigValues(uid);
    emit toolsChanged();
}

QVariantMap PlacesModel::getToolOptions(const QString &uid) const
{
    QVariantMap options;
    foreach (const QString& key, configKeys(uid))
    {
        options.insert(key, configValue(uid, key));
    }
    return options;
}

FolderBase::Item::Ptr PlacesModel::makeItem(const QString& uid,
                                            const QString& name,
                                            const QString& section,
                                            const QString& icon,
                                            const QString& type,
                                            bool selectable)
{
    Item::Ptr item(new Item);
    item->linkTarget = uid;
    item->name = uid;
    item->friendlyName = name;
    item->sectionName = section;
    item->icon = icon;
    item->linkModel = type;
    item->selectable = selectable;
    item->type = Folder;
    return item;
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

    Item::ConstPtr item = itemByName(basename(path));
    if (! item.isNull())
    {
        QStringList bookmarkServices = configValue("bookmark-services").toStringList();
        bookmarkServices.removeAll(item->linkTarget);
        setConfigValue("bookmark-services", bookmarkServices);
        removeConfigValues(item->linkTarget);

        removeItem(findItem(item));

        return true;
    }
    else
    {
        return false;
    }
}

void PlacesModel::loadDirectory(const QString&)
{
    clearItems();

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

        appendItem(makeItem(uid, name, "Bookmarks", icon, type, true));
    }

    foreach (const QString& uid, localServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();
        const QString path = configValue(uid, "path").toString();

        if (path.size() && QDir(path).exists())
        {
            appendItem(makeItem(uid, name, "Media", icon, type, false));
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
            appendItem(makeItem(uid, name, "Storage", icon, type, false));
        }
    }

    if (developerMode.enabled())
    {
        foreach (const QString& uid, developerServices)
        {
            const QString type = configValue(uid, "type").toString();
            const QString name = configValue(uid, "name").toString();
            const QString icon = configValue(uid, "icon").toString();

            appendItem(makeItem(uid, name, "Developer Mode", icon, type, false));
        }
    }

    foreach (const QString& uid, userServices)
    {
        const QString type = configValue(uid, "type").toString();
        const QString name = configValue(uid, "name").toString();
        const QString icon = configValue(uid, "icon").toString();

        appendItem(makeItem(uid, name, "Cloud", icon, type, false));
    }
}

bool PlacesModel::useEncryptionPassphrase() const
{
    return configValue("useEncryptionPassphrase").toBool();
}

void PlacesModel::setUseEncryptionPassphrase(bool value)
{
    setConfigValue("useEncryptionPassphrase", value);
    if (value)
    {
        setConfigValue("encryptionPassphraseHash",
                       encryptionPassphraseHash());
    }
    emit useEncryptionPassphraseChanged();
}

bool PlacesModel::useNewUI() const
{
    return configValue("uiconfig","useNewUI").toBool();
}

void PlacesModel::setUseNewUI(bool value)
{
    setConfigValue("uiconfig","useNewUI", value);
    emit useNewUIChanged();
}

int PlacesModel::sortBy() const
{
    return configValue("uiconfig","sortBy").toInt();
}

void PlacesModel::setSortBy(int value)
{
    setConfigValue("uiconfig","sortBy", value);
    emit sortByChanged();
}

int PlacesModel::dirsPosition() const
{
    return configValue("uiconfig","dirsPosition").toInt();
}

void PlacesModel::setDirsPosition(int value)
{
    setConfigValue("uiconfig","dirsPosition", value);
    emit dirsPositionChanged();
}

bool PlacesModel::reverseSort() const
{
    return configValue("uiconfig","reverseSort").toBool();
}

void PlacesModel::setReverseSort(bool value)
{
    setConfigValue("uiconfig","reverseSort", value);
    emit reverseSortChanged();
}

bool PlacesModel::ignoreCase() const
{
    return configValue("uiconfig","ignoreCase").toBool();
}

void PlacesModel::setIgnoreCase(bool value)
{
    setConfigValue("uiconfig","ignoreCase", value);
    emit ignoreCaseChanged();
}

bool PlacesModel::verifyEncryptionPassphrase(const QString& passphrase) const
{
    QByteArray hash = configValue("encryptionPassphraseHash").toByteArray();
    return (encryptionPassphraseHash(passphrase.toUtf8()) == hash);
}
