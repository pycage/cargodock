#include "folderbase.h"
#include "copyaction.h"

#include <QSettings>
#include <QDebug>


FolderBase::FolderBase(QObject* parent)
    : QAbstractListModel(parent)
    , myMinDepth(INT_MAX)
{
    myRolenames.insert(NameRole, "name");
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
    myRolenames.insert(ModelTargetRole, "modelTarget");
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

void FolderBase::setUid(const QString& uid)
{
    myUid = uid;
    init();
    setPath(configValue("path").toString());
}

QVariant FolderBase::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= rowCount(QModelIndex()))
    {
        return QVariant();
    }

    switch (role)
    {
    case NameRole:
        return itemName(index.row());
    case SelectableRole:
        return true;
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

    crumbs << userBasename(path);

    while (parent != path)
    {
        path = parent;
        crumbs.prepend(userBasename(path));
        parent = parentPath(path);
    }

    return crumbs.mid(myMinDepth);
}

QStringList FolderBase::selection() const
{
    QStringList items;
    foreach (int idx, mySelection)
    {
        items << joinPath(QStringList() << myPath << itemName(idx));
    }
    return items;
}

void FolderBase::open(const QString& name)
{
    QString path = joinPath(QStringList() << myPath << name);
    ItemType itemType = type(path);
    if (itemType == Folder || itemType == FolderLink)
    {
        setPath(path);
    }
    else
    {
        runFile(path);
    }
}

void FolderBase::copySelected(FolderBase* dest)
{
    QStringList paths;
    foreach (int idx, mySelection)
    {
        paths << joinPath(QStringList() << myPath << itemName(idx));
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
        const QString endpoint = joinPath(QStringList() << myPath << itemName(idx));
        const QString destPath = dest->joinPath(QStringList() << dest->path() << itemName(idx));

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
        if (! mySelection.contains(i))
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

FolderBase::ItemType FolderBase::type(const QString&) const
{
    return Unsupported;
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
