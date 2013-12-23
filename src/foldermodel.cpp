#include "foldermodel.h"
#include "copyaction.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrl>
#include <QDebug>

FolderModel::FolderModel(QObject* parent)
    : FolderBase(parent)
    , myIsReadable(true)
    , myIsWritable(false)
{
    myIcons.insert("application/octet-stream",                "image://theme/icon-m-other");
    myIcons.insert("application/pdf",                         "image://theme/icon-m-document");
    myIcons.insert("application/vnd.android.package-archive", "image://theme/icon-s-android");
    myIcons.insert("application/xml",                         "image://theme/icon-m-levels");
    myIcons.insert("application/x-core",                      "image://theme/icon-m-crash-reporter");
    myIcons.insert("application/x-executable",                "image://theme/icon-m-play");
    myIcons.insert("application/x-gzip",                      "image://theme/icon-m-other");
    myIcons.insert("application/x-rpm",                       "image://theme/icon-lock-system-update");
    myIcons.insert("application/x-sharedlib",                 "image://theme/icon-m-share");
    myIcons.insert("application/x-shellscript",               "image://theme/icon-m-other");
    myIcons.insert("application/x-sqlite3",                   "image://theme/icon-m-levels");
    myIcons.insert("application/x-x509-ca-cert",              "image://theme/icon-m-certificates");
    myIcons.insert("audio/mp4",                               "image://theme/icon-m-music");
    myIcons.insert("audio/mpeg",                              "image://theme/icon-m-music");
    myIcons.insert("image/jpeg",                              "image://theme/icon-m-image");
    myIcons.insert("image/png",                               "");
    myIcons.insert("inode/directory",                         "image://theme/icon-m-folder");
    myIcons.insert("text/html",                               "image://theme/icon-m-region");
    myIcons.insert("text/plain",                              "image://theme/icon-m-document");
    myIcons.insert("text/vcard",                              "image://theme/icon-m-people");
    myIcons.insert("text/x-c++src",                           "image://theme/icon-m-document");
    myIcons.insert("text/x-qml",                              "image://theme/icon-m-levels");
    myIcons.insert("video/mp4",                               "image://theme/icon-m-video");
    myIcons.insert("video/x-flv",                             "image://theme/icon-m-video");
}

int FolderModel::rowCount(const QModelIndex&) const
{
    return myItems.size();
}

QVariant FolderModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= myItems.size())
    {
        return QVariant();
    }

    Item::ConstPtr item = myItems.at(index.row());

    switch (role)
    {
    case NameRole:
        return item->name;
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
    case LinkTargetRole:
        return item->linkTarget;
    case SelectedRole:
        return isSelected(index.row());
    default:
        return QVariant();
    }
}

void FolderModel::setPermissions(const QString& name, int permissions)
{
    qDebug() << "setting permissions" << QDir(path()).absoluteFilePath(name)
                << permissions;

    int idx = 0;
    QVector<int> roles;
    roles << PermissionsRole;
    foreach (Item::Ptr item, myItems)
    {
        if (item->name == name)
        {
            // Qt behaves in a weird way here, so we need to double permissions,
            // if we're the owner of the file
            if (item->owner == getenv("USER"))
            {
                if (permissions & QFile::ReadOwner)
                    permissions |= QFile::ReadUser;
                else if (permissions & QFile::ReadUser)
                    permissions ^= QFile::ReadUser;

                if (permissions & QFile::WriteOwner)
                    permissions |= QFile::WriteUser;
                else if (permissions & QFile::WriteUser)
                    permissions ^= QFile::WriteUser;

                if (permissions & QFile::ExeOwner)
                    permissions |= QFile::ExeUser;
                else if (permissions & QFile::ExeUser)
                    permissions ^= QFile::ExeUser;
            }

            if (QFile::setPermissions(QDir(path()).absoluteFilePath(name),
                                      (QFile::Permission) permissions))
            {
                item->permissions = permissions;
            }
            else
            {
                emit error(QString("cannot change permissions on %1").arg(1));
            }
            // emit a change even if it failed, so that the UI switch gets reset
            // to the correct state
            emit dataChanged(index(idx), index(idx), roles);
            break;
        }
        ++idx;
    }
}

void FolderModel::rename(const QString& name, const QString& newName)
{
    const QString source = joinPath(QStringList() << path() << name);
    const QString dest = joinPath(QStringList() << path() << newName);
    qDebug() << "rename" << source << dest;
    if (! QFile::rename(source, dest))
    {
        emit error("could not rename");
    }
    else
    {
        int idx = 0;
        QVector<int> roles;
        roles << NameRole << UriRole;
        foreach (Item::Ptr item, myItems)
        {
            if (item->name == name)
            {
                item->name = newName;
                item->uri = dest;
                emit dataChanged(index(idx), index(idx), roles);
                break;
            }
            ++idx;
        }
    }
}

QString FolderModel::basename(const QString& path) const
{
    return QFileInfo(path).fileName();
}

QString FolderModel::userBasename(const QString& path) const
{
    if (path == "/")
    {
        return "System";
    }
    else
    {
        return basename(path);
    }
}

QString FolderModel::joinPath(const QStringList& parts) const
{
    QString path;
    foreach (const QString part, parts)
    {
        if (part.startsWith("/"))
        {
            path += part;
        }
        else if (! path.endsWith("/"))
        {
            path += "/" + part;
        }
        else
        {
            path += part;
        }
    }
    return path;
}

QString FolderModel::parentPath(const QString& path) const
{
    QDir dir(path);
    dir.cdUp();
    return dir.path();
}

QStringList FolderModel::list(const QString& path) const
{
    QDir dir(path);
    return dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
}

FolderModel::ItemType FolderModel::type(const QString& path) const
{
    QFileInfo finfo(path);
    if (finfo.isDir())
    {
        return finfo.isSymLink() ? FolderLink : Folder;
    }
    else if (finfo.isFile())
    {
        return finfo.isSymLink() ? FileLink : File;
    }
    else
    {
        return Unsupported;
    }
}

QIODevice* FolderModel::openFile(const QString& path,
                                 QIODevice::OpenModeFlag mode)
{
    QFile* fd = new QFile(path);
    fd->open(mode);
    return fd;
}

bool FolderModel::makeDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    QDir dir = QFileInfo(path).dir();
    return dir.mkdir(QFileInfo(path).fileName());
}

void FolderModel::loadDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    QDir dir(path);

    beginResetModel();
    myItems.clear();

    QMimeDatabase mimeDb;

    if (dir.isReadable())
    {
        foreach (const QFileInfo& finfo, dir.entryInfoList())
        {
            if (finfo.baseName().isEmpty())
            {
                continue;
            }

            Item::Ptr item(new Item);
            item->name = finfo.fileName();
            item->path = path;
            item->uri = joinPath(QStringList() << item->path << item->name);
            item->type = type(finfo.absoluteFilePath());
            item->mimeType = mimeDb.mimeTypeForFile(finfo).name();
            item->owner = finfo.owner();
            if (item->owner.isEmpty())
            {
                item->owner = QString::number(finfo.ownerId());
            }
            item->group = finfo.group();
            if (item->group.isEmpty())
            {
                item->group = QString::number(finfo.groupId());
            }
            item->permissions = finfo.permissions();
            item->icon = myIcons.value(item->mimeType, "image://theme/icon-m-other");
            item->size = finfo.size();
            item->mtime = finfo.lastModified();

            qDebug() << "MIME type" << item->mimeType;
            qDebug() << "icon" << item->icon;

            if (item->type == FolderLink || item->type == FileLink)
            {
                item->linkTarget = finfo.symLinkTarget();
            }

            qDebug() << "name" << item->name << item->type;
            myItems << item;
        }
        myIsReadable = true;
    }
    else
    {
        myIsReadable = false;
    }

    myIsWritable = QFileInfo(path).isWritable();

    endResetModel();
}

QString FolderModel::itemName(int idx) const
{
    if (idx < myItems.size())
    {
        return myItems[idx]->name;
    }
    else
    {
        return QString();
    }
}

bool FolderModel::linkFile(const QString& path, const QString& source)
{
    qDebug() << "link file" << path << "to" << source;
    return QFile::link(source, path);
}

bool FolderModel::deleteFile(const QString& path)
{
    bool ok = false;
    QFileInfo finfo(path);
    if (! finfo.isSymLink() && finfo.isDir())
    {
        QDir dir(path);
        ok = dir.removeRecursively();
    }
    else
    {
        QDir dir = finfo.dir();
        ok = dir.remove(finfo.fileName());
    }
    return ok;
}

void FolderModel::runFile(const QString& path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
