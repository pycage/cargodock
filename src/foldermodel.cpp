#include "foldermodel.h"
#include "localfile.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <QUrl>
#include <QDebug>

namespace
{
const QStringList OPENABLE_TYPES = QStringList()
        << "application/pdf"
        << "application/vnd.android.package-archive"
        << "application/x-rpm"
        << "audio/mp4"
        << "audio/mpeg"
        << "image/jpeg"
        << "image/png"
        << "text/html"
        << "video/mp4";
}

FolderModel::FolderModel(QObject* parent)
    : FolderBase(parent)
    , myIsReadable(true)
    , myIsWritable(false)
{
    myMimeTypeIcons.insert("image/jpeg",                              "");
    myMimeTypeIcons.insert("image/png",                               "");

    myPreviewComponents.insert("audio/mp4",               "PreviewAudio");
    myPreviewComponents.insert("audio/mpeg",              "PreviewAudio");
    myPreviewComponents.insert("audio/x-vorbis+ogg",      "PreviewAudio");
    myPreviewComponents.insert("image/jpeg",              "PreviewImage");
    myPreviewComponents.insert("image/png",               "PreviewImage");
    myPreviewComponents.insert("text/plain",              "PreviewText");
}

FolderModel::FolderModel(const FolderModel& other)
    : FolderBase(other)
    , myMimeTypeIcons(other.myMimeTypeIcons)
    , myPreviewComponents(other.myPreviewComponents)
    , myIsReadable(other.myIsReadable)
    , myIsWritable(other.myIsWritable)
{

}

FolderBase* FolderModel::clone() const
{
    FolderModel* dolly = new FolderModel(*this);
    return dolly;
}

QVariant FolderModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= itemCount())
    {
        return QVariant();
    }

    Item::ConstPtr item = itemAt(index.row());

    switch (role)
    {
    case PreviewRole:
        return myPreviewComponents.contains(item->mimeType)
                ? myPreviewComponents[item->mimeType] + "#" + item->uri
                : QVariant();
    case CapabilitiesRole:
        return HasPermissions |
               (OPENABLE_TYPES.contains(item->mimeType) ? CanOpen
                                                        : NoCapabilities);
    default:
        return FolderBase::data(index, role);
    }
}

int FolderModel::capabilities() const
{
    int caps = (isWritable() ? AcceptCopy : NoCapabilities) |
               (isWritable() ? AcceptLink : NoCapabilities) |
               CanOpen;

    if (selected() > 0)
    {
        bool canBookmark = true;
        foreach (const QString& path, selection())
        {
            Item::ConstPtr item = itemByName(basename(path));
            if (item && item->type != Folder && item->type != FolderLink)
            {
                canBookmark = false;
                break;
            }
        }
        caps |= (canBookmark ? CanBookmark : NoCapabilities) |
                (isReadable() ? CanCopy : NoCapabilities) |
                (isWritable() ? CanDelete : NoCapabilities) |
                CanLink;
    }

    return caps;
}

void FolderModel::setPermissions(const QString& name, int permissions)
{
    qDebug() << "setting permissions" << QDir(path()).absoluteFilePath(name)
                << permissions;

    QVector<int> roles;
    roles << PermissionsRole;

    Item::Ptr item = itemByName(name);
    if (! item.isNull())
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
            emit error(QString("Could not change permissions: %1")
                       .arg(item->name));
        }
        // emit a change even if it failed, so that the UI switch gets reset
        // to the correct state
        int idx = findItem(item);
        emit dataChanged(index(idx), index(idx), roles);
    }
}

void FolderModel::rename(const QString& name, const QString& newName)
{
    const QString source = joinPath(QStringList() << path() << name);
    const QString dest = joinPath(QStringList() << path() << newName);
    qDebug() << "rename" << source << dest;
    if (! QFile::rename(source, dest))
    {
        emit error(QString("Could not rename file: %1").arg(name));
    }
    else
    {
        QVector<int> roles;
        roles << NameRole << UriRole;
        Item::Ptr item = itemByName(name);
        if (! item.isNull())
        {
            item->name = newName;
            item->uri = dest;
            int idx = findItem(item);
            emit dataChanged(index(idx), index(idx), roles);
        }
    }
}

QString FolderModel::readFile(const QString& name) const
{
    const QString filePath = joinPath(QStringList() << path() << name);
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        return file.read(8192);
    }
    else
    {
        return QString();
    }
}

QString FolderModel::friendlyBasename(const QString& path) const
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

QStringList FolderModel::list(const QString& path) const
{
    QDir dir(path);
    return dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
}

QIODevice* FolderModel::openFile(const QString& path,
                                 qint64 size,
                                 QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(size)
    LocalFile* fd = new LocalFile(path);
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

    clearItems();

    QMimeDatabase mimeDb;

    bool showHidden = configValue("showHidden").toBool();
    QDir::Filters filter = showHidden ? QDir::AllEntries |
                                        QDir::NoDotAndDotDot |
                                        QDir::Hidden
                                      : QDir::AllEntries |
                                        QDir::NoDotAndDotDot;

    if (dir.isReadable())
    {
        foreach (const QFileInfo& finfo, dir.entryInfoList(filter))
        {
            Item::Ptr item(new Item);
            item->selectable = true;
            item->name = finfo.fileName();
            item->path = path;
            item->uri = joinPath(QStringList() << item->path << item->name);

            if (finfo.isDir())
            {
                item->type = finfo.isSymLink() ? FolderLink : Folder;
            }
            else if (finfo.isFile())
            {
                item->type = finfo.isSymLink() ? FileLink : File;
            }
            else
            {
                // unsupported
                continue;
            }

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
            item->icon = mimeTypeIcon(item->mimeType);
            item->size = finfo.size();
            item->mtime = finfo.lastModified();

            if (item->type == FolderLink || item->type == FileLink)
            {
                item->linkTarget = finfo.symLinkTarget();
            }

            appendItem(item);
        }
        myIsReadable = true;
    }
    else
    {
        myIsReadable = false;
    }

    myIsWritable = QFileInfo(path).isWritable();
}

QString FolderModel::mimeTypeIcon(const QString& mimeType) const
{
    if (myMimeTypeIcons.contains(mimeType))
    {
        return myMimeTypeIcons[mimeType];
    }
    else
    {
        return FolderBase::mimeTypeIcon(mimeType);
    }
}

bool FolderModel::linkFile(const QString& path,
                           const QString& source,
                           const FolderBase*)
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
