#include "folderbase.h"
#include "copyaction.h"

#include <QSettings>
#include <QDebug>


FolderBase::FolderBase(QObject* parent)
    : QAbstractListModel(parent)
    , myMinDepth(INT_MAX)
{
    myRolenames.insert(NameRole, "name");
    myRolenames.insert(FriendlyNameRole, "friendlyName");
    myRolenames.insert(SectionRole, "section");
    myRolenames.insert(PathRole, "path");
    myRolenames.insert(UriRole, "uri");
    myRolenames.insert(PreviewRole, "preview");
    myRolenames.insert(TypeRole, "type");
    myRolenames.insert(MimeTypeRole, "mimeType");
    myRolenames.insert(IconRole, "icon");
    myRolenames.insert(SizeRole, "size");
    myRolenames.insert(MtimeRole, "mtime");
    myRolenames.insert(OwnerRole, "owner");
    myRolenames.insert(GroupRole, "group");
    myRolenames.insert(PermissionsRole, "permissions");
    myRolenames.insert(LinkTargetRole, "linkTarget");
    myRolenames.insert(LinkModelRole, "modelTarget");
    myRolenames.insert(SelectableRole, "selectable");
    myRolenames.insert(SelectedRole, "selected");
    myRolenames.insert(CapabilitiesRole, "capabilities");

    myMimeTypeIcons.insert("application/epub-zip",                    "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/octet-stream",                "image://theme/icon-m-other");
    myMimeTypeIcons.insert("application/pdf",                         "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/rtf",                         "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/vnd.android.package-archive", "image://theme/icon-m-device");
    myMimeTypeIcons.insert("application/xml",                         "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/x-core",                      "image://theme/icon-m-crash-reporter");
    myMimeTypeIcons.insert("application/x-debian-package",            "image://theme/icon-m-other");
    myMimeTypeIcons.insert("application/x-executable",                "image://theme/icon-m-play");
    myMimeTypeIcons.insert("application/x-gzip",                      "image://theme/icon-m-other");
    myMimeTypeIcons.insert("application/x-mobipocket-ebook",          "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/x-redhat-package-manager",    "image://theme/icon-lock-system-update");
    myMimeTypeIcons.insert("application/x-rpm",                       "image://theme/icon-lock-system-update");
    myMimeTypeIcons.insert("application/x-sharedlib",                 "image://theme/icon-m-share");
    myMimeTypeIcons.insert("application/x-shellscript",               "image://theme/icon-m-document");
    myMimeTypeIcons.insert("application/x-sqlite3",                   "image://theme/icon-m-levels");
    myMimeTypeIcons.insert("application/x-trash",                     "image://theme/icon-m-dismiss");
    myMimeTypeIcons.insert("application/x-x509-ca-cert",              "image://theme/icon-m-certificates");
    myMimeTypeIcons.insert("application/x-xcf",                       "image://theme/icon-m-image");
    myMimeTypeIcons.insert("application/zip",                         "image://theme/icon-m-other");
    myMimeTypeIcons.insert("audio/mp4",                               "image://theme/icon-m-music");
    myMimeTypeIcons.insert("audio/mpeg",                              "image://theme/icon-m-music");
    myMimeTypeIcons.insert("image/jpeg",                              "image://theme/icon-m-image");
    myMimeTypeIcons.insert("image/png",                               "image://theme/icon-m-image");
    myMimeTypeIcons.insert("image/rle",                               "image://theme/icon-m-image");
    myMimeTypeIcons.insert("image/svg+xml",                           "image://theme/icon-m-image");
    myMimeTypeIcons.insert("inode/directory",                         "image://theme/icon-m-folder");
    myMimeTypeIcons.insert("text/html",                               "image://theme/icon-m-region");
    myMimeTypeIcons.insert("text/plain",                              "image://theme/icon-m-document");
    myMimeTypeIcons.insert("text/vcard",                              "image://theme/icon-m-people");
    myMimeTypeIcons.insert("text/x-c++src",                           "image://theme/icon-m-document");
    myMimeTypeIcons.insert("text/x-qml",                              "image://theme/icon-m-document");
    myMimeTypeIcons.insert("video/mp4",                               "image://theme/icon-m-video");
    myMimeTypeIcons.insert("video/x-flv",                             "image://theme/icon-m-video");
}

FolderBase::FolderBase(const FolderBase& other)
    : QAbstractListModel()
    , myRolenames(other.myRolenames)
    , myMimeTypeIcons(other.myMimeTypeIcons)
    , myItems(other.myItems)
    , myUid(other.myUid)
    , myPath(other.myPath)
    , myMinDepth(other.myMinDepth)
    , mySelection(other.mySelection)
{

}

void FolderBase::setUid(const QString& uid)
{
    myUid = uid;
    init();
    setPath(configValue("path").toString());
}

void FolderBase::clearItems()
{
    if (myItems.size())
    {
        beginRemoveRows(QModelIndex(), 0, myItems.size() - 1);
        myItems.clear();
        endRemoveRows();
    }
}

void FolderBase::appendItem(Item::Ptr item)
{
    beginInsertRows(QModelIndex(), myItems.size(), myItems.size());
    myItems << item;
    endInsertRows();
}

void FolderBase::removeItem(int idx)
{
    beginRemoveRows(QModelIndex(), idx, idx);
    myItems.removeAt(idx);
    endRemoveRows();
}

FolderBase::Item::Ptr FolderBase::itemAt(int pos)
{
    return myItems.value(pos);
}

FolderBase::Item::ConstPtr FolderBase::itemAt(int pos) const
{
    return myItems.value(pos);
}

FolderBase::Item::Ptr FolderBase::itemByName(const QString& name)
{
    foreach (Item::Ptr item, myItems)
    {
        if (item->name == name)
        {
            return item;
        }
    }
    return Item::Ptr(0);
}

int FolderBase::findItem(Item::ConstPtr item) const
{
    int pos = 0;
    foreach (Item::ConstPtr i, myItems)
    {
        if (item == i)
        {
            return pos;
        }
        ++pos;
    }
    return -1;
}

FolderBase::Item::ConstPtr FolderBase::itemByName(const QString& name) const
{
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->name == name)
        {
            return item;
        }
    }
    return Item::ConstPtr(0);
}

int FolderBase::rowCount(const QModelIndex&) const
{
    return myItems.size();
}

QVariant FolderBase::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= rowCount(QModelIndex()))
    {
        return QVariant();
    }

    Item::ConstPtr item = myItems.at(index.row());

    switch (role)
    {
    case NameRole:
        return item->name;
    case FriendlyNameRole:
        return item->friendlyName;
    case SectionRole:
        return item->sectionName;
    case PathRole:
        return item->path;
    case UriRole:
        return item->uri;
    case TypeRole:
        return item->type;
    case MimeTypeRole:
        return item->mimeType;
    case IconRole:
        return item->icon;
    case SizeRole:
        return item->size;
    case MtimeRole:
        return item->mtime;
    case OwnerRole:
        return item->owner;
    case GroupRole:
        return item->group;
    case PermissionsRole:
        return item->permissions;
    case LinkModelRole:
        return item->linkModel;
    case LinkTargetRole:
        return item->linkTarget;
    case SelectableRole:
        return item->selectable;
    case SelectedRole:
        return isSelected(index.row());
    case CapabilitiesRole:
        return NoCapabilities;
    default:
        return QVariant();
    }
}

void FolderBase::setPath(const QString& path)
{
    unselectAll();
    loadDirectory(path);
    mySelection.clear();

    int depth = 0;
    QString p = path;
    QString pp = parentPath(p);
    while (pp != p)
    {
        p = pp;
        pp = parentPath(p);
        ++depth;
    }
    if (depth < myMinDepth)
    {
        myMinDepth = depth;
    }

    myPath = path;
    emit pathChanged();
}

QStringList FolderBase::breadcrumbs() const
{
    QStringList crumbs;

    QString path = myPath;
    QString parent = parentPath(path);

    crumbs << friendlyBasename(path);

    while (parent != path)
    {
        path = parent;
        crumbs.prepend(friendlyBasename(path));
        parent = parentPath(path);
    }

    return crumbs.mid(myMinDepth);
}

QStringList FolderBase::selection() const
{
    QStringList items;
    foreach (int idx, mySelection)
    {
        items << joinPath(QStringList() << myPath << myItems.at(idx)->name);
    }
    return items;
}

void FolderBase::open(const QString& name)
{
    Item::ConstPtr item = itemByName(name);
    if (! item.isNull())
    {
        QString path = joinPath(QStringList() << myPath << name);

        if (item->type == Folder || item->type == FolderLink)
        {
            qDebug() << "opening folder" << path;
            setPath(path);
        }
        else
        {
            qDebug() << "opening file" << path;
            runFile(path);
        }
    }
}

void FolderBase::copySelected(FolderBase* dest)
{
    QStringList paths;
    foreach (int idx, mySelection)
    {
        paths << joinPath(QStringList() << myPath << myItems.at(idx)->name);
    }

    CopyAction* action = new CopyAction(this, dest, paths, dest->path());
    connect(action, SIGNAL(finished()),
            this, SIGNAL(finished()));
    connect(action, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    action->start();
    unselectAll();
}

void FolderBase::deleteItems(const QStringList& items)
{
    foreach (const QString& path, items)
    {
        if (! deleteFile(path))
        {
            emit error(QString("Could not delete file: %1").arg(basename(path)));
        }
    }
    unselectAll();
    emit finished();
}

void FolderBase::linkSelected(FolderBase* dest)
{
    foreach (int idx, mySelection)
    {
        const QString endpoint = joinPath(QStringList() << myPath << myItems.at(idx)->name);
        const QString destPath = dest->joinPath(QStringList() << dest->path() << myItems.at(idx)->name);

        if (! dest->linkFile(destPath, endpoint, this))
        {
            emit error("Could not link to destination.");
        }
    }
    unselectAll();
    emit finished();
}

void FolderBase::newFolder(const QString& name)
{
    if (makeDirectory(joinPath(QStringList() << myPath << name)))
    {
        emit finished();
    }
    else
    {
        emit error(QString("Could not create folder: %1").arg(name));
    }
}

void FolderBase::setPermissions(const QString&, int)
{
    emit error("Changing permissions is not supported.");
}

void FolderBase::rename(const QString&, const QString&)
{
    emit error("Renaming is not supported.");
}

QString FolderBase::readFile(const QString&) const
{
    return QString();
}

void FolderBase::refresh()
{
    loadDirectory(myPath);
}

void FolderBase::cdUp(int amount)
{
    QString path = myPath;
    for (int i = 0; i < amount; ++i)
    {
        path = parentPath(path);
    }
    setPath(path);
}

void FolderBase::setSelected(int idx, bool value)
{
    if (idx < rowCount(QModelIndex()))
    {
        if (value)
        {
            mySelection << idx;
        }
        else
        {
            mySelection.remove(idx);
        }
        emit selectionChanged();

        QVector<int> roles;
        roles << SelectedRole;
        emit dataChanged(index(idx), index(idx), roles);
    }
}

void FolderBase::selectAll()
{
    int size = rowCount(QModelIndex());
    for (int i = 0; i < size; ++i)
    {
        bool selectable = data(index(i), SelectableRole).toBool();
        if (! mySelection.contains(i) && selectable)
        {
            setSelected(i, true);
        }
    }
}

void FolderBase::unselectAll()
{
    int size = rowCount(QModelIndex());
    for (int i = 0; i < size; ++i)
    {
        if (mySelection.contains(i))
        {
            setSelected(i, false);
        }
    }
}

void FolderBase::invertSelection()
{
    int size = rowCount(QModelIndex());
    for (int i = 0; i < size; ++i)
    {
        setSelected(i, ! mySelection.contains(i));
    }
}

QString FolderBase::parentPath(const QString& path,
                               const QString& separator) const
{
    int idx = path.lastIndexOf(separator);
    if (idx == 0)
    {
        return separator;
    }
    else if (idx != -1)
    {
        return path.left(idx);
    }
    else
    {
        return path;
    }
}

QString FolderBase::basename(const QString& path,
                           const QString& separator) const
{
    int idx = path.lastIndexOf(separator);
    if (idx != -1)
    {
        return path.mid(idx + 1);
    }
    else
    {
        return path;
    }
}

QString FolderBase::joinPath(const QStringList& parts,
                             const QString& separator) const
{
    QString path;
    foreach (const QString part, parts)
    {
        if (part.startsWith(separator))
        {
            path += part;
        }
        else if (! path.endsWith(separator))
        {
            path += separator + part;
        }
        else
        {
            path += part;
        }
    }
    return path;
}

bool FolderBase::isSelected(int idx) const
{
    return mySelection.contains(idx);
}

QString FolderBase::mimeTypeIcon(const QString& mimeType) const
{
    return myMimeTypeIcons.value(mimeType, "image://theme/icon-m-other");
}

QStringList FolderBase::list(const QString&) const
{
    return QStringList();
}

QIODevice* FolderBase::openFile(const QString&, QIODevice::OpenModeFlag)
{
    return 0;
}

bool FolderBase::makeDirectory(const QString&)
{
    return false;
}

bool FolderBase::linkFile(const QString&, const QString&, const FolderBase*)
{
    return false;
}

bool FolderBase::deleteFile(const QString&)
{
    return false;
}

void FolderBase::runFile(const QString& path)
{

}

void FolderBase::setConfigValue(const QString& key,
                                const QVariant& value)
{
    setConfigValue(myUid, key, value);
}

QVariant FolderBase::configValue(const QString& key) const
{
    return configValue(myUid, key);
}

void FolderBase::setConfigValue(const QString& uid,
                                const QString& key,
                                const QVariant& value)
{
    QSettings settings("harbour-cargodock", "CargoDock");
    settings.beginGroup(uid);
    settings.setValue(key, value);
}

QVariant FolderBase::configValue(const QString& uid,
                                 const QString& key) const
{
    QSettings settings("harbour-cargodock", "CargoDock");
    settings.beginGroup(uid);
    return settings.value(key);
}

void FolderBase::removeConfigValues(const QString& uid)
{
    QSettings settings("harbour-cargodock", "CargoDock");
    settings.beginGroup(uid);
    settings.remove("");
}

void FolderBase::cloneConfigValues(const QString& uid, const QString& cloneUid)
{
    QMap<QString, QVariant> values;
    QSettings settings("harbour-cargodock", "CargoDock");
    settings.beginGroup(uid);
    foreach (const QString& key, settings.childKeys())
    {
        values[key] = settings.value(key);
    }
    settings.endGroup();

    settings.beginGroup(cloneUid);
    foreach (const QString& key, values.keys())
    {
        settings.setValue(key, values[key]);
    }
}
