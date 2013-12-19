#include "foldermodel.h"
#include "copyaction.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrl>
#include <QDebug>

FolderModel::FolderModel(QObject* parent)
    : QAbstractListModel(parent)
    , myMinDepth(0)
    , myIsReadable(true)
    , mySelectionCount(0)
{
    myRolenames.insert(NameRole, "name");
    myRolenames.insert(PathRole, "path");
    myRolenames.insert(UriRole, "uri");
    myRolenames.insert(TypeRole, "type");
    myRolenames.insert(MimeTypeRole, "mimeType");
    myRolenames.insert(IconRole, "icon");
    myRolenames.insert(SizeRole, "size");
    myRolenames.insert(MtimeRole, "mtime");
    myRolenames.insert(LinkTargetRole, "linkTarget");
    myRolenames.insert(SelectedRole, "selected");

    myIcons.insert("application/pdf",                         "image://theme/icon-m-document");
    myIcons.insert("application/vnd.android.package-archive", "image://theme/icon-s-android");
    myIcons.insert("application/x-core",                      "image://theme/icon-m-crash-reporter");
    myIcons.insert("application/x-executable",                "image://theme/icon-m-other");
    myIcons.insert("application/x-gzip",                      "image://theme/icon-m-other");
    myIcons.insert("application/x-rpm",                       "image://theme/icon-lock-system-update");
    myIcons.insert("application/x-shellscript",               "image://theme/icon-m-other");
    myIcons.insert("audio/mpeg",                              "image://theme/icon-m-music");
    myIcons.insert("image/jpeg",                              "image://theme/icon-m-image");
    myIcons.insert("image/png",                               "");
    myIcons.insert("inode/directory",                         "image://theme/icon-m-folder");
    myIcons.insert("text/html",                               "image://theme/icon-m-region");
    myIcons.insert("text/plain",                              "image://theme/icon-m-clipboard");
    myIcons.insert("video/mp4",                               "image://theme/icon-m-video");
    myIcons.insert("video/x-flv",                             "image://theme/icon-m-video");
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return myRolenames;
}

int FolderModel::rowCount(const QModelIndex& parent) const
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
    case LinkTargetRole:
        return item->linkTarget;
    case SelectedRole:
        return item->isSelected;
    default:
        return QVariant();
    }
}

void FolderModel::refresh()
{
    loadDirectory(myPath);
}

void FolderModel::open(const QString& name)
{
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->name == name)
        {
            const QString path = QDir(myPath).filePath(name);
            if (item->type == Folder || item->type == FolderLink)
            {
                loadDirectory(path);
                myPath = path;
                emit pathChanged();
            }
            else
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            }
        }
    }
}

void FolderModel::cdUp(int amount)
{
    QDir dir(myPath);
    for (int i = 0; i < amount; ++i)
    {
        dir.cdUp();
    }

    const QString path = dir.absolutePath();
    qDebug() << "open" << amount << path;
    loadDirectory(path);
    myPath = path;
    emit pathChanged();
}

void FolderModel::setSelected(int idx, bool value)
{
    if (idx < myItems.size())
    {
        myItems[idx]->isSelected = value;
        mySelectionCount += value ? 1 : -1;
        emit selectionChanged();
        QVector<int> roles;
        roles << SelectedRole;
        emit dataChanged(index(idx), index(idx), roles);
    }
}

void FolderModel::unselectAll()
{
    mySelectionCount = 0;
    emit selectionChanged();

    QVector<int> roles;
    roles << SelectedRole;

    for (int idx = 0; idx < myItems.size(); ++idx)
    {
        if (myItems[idx]->isSelected)
        {
            myItems[idx]->isSelected = false;
            emit dataChanged(index(idx), index(idx), roles);
        }
    }
}

void FolderModel::copySelected(FolderModel* dest)
{
    QList<QString> paths;
    foreach (Item::ConstPtr item, selectedItems())
    {
        paths << joinPath(myPath, item->name);
    }

    CopyAction* action = new CopyAction(this, dest, paths, dest->path());
    connect(action, SIGNAL(finished()),
            this, SIGNAL(finished()));
    connect(action, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    action->start();
    unselectAll();
}

void FolderModel::deleteSelected()
{
    foreach (const Item::ConstPtr item, selectedItems())
    {
        qDebug() << "deleting" << item->name;
        bool ok = true;
        if (item->type == Folder)
        {
            QDir dir(joinPath(myPath, item->name));
            ok = dir.removeRecursively();
        }
        else
        {
            QDir dir(myPath);
            ok = dir.remove(item->name);
        }
        if (! ok)
        {
            emit error(QString("Could not delete file: %1").arg(item->name));
        }
    }
    unselectAll();
    emit finished();
}

void FolderModel::linkSelected(FolderModel* dest)
{
    qDebug() << "linking";
    foreach (Item::ConstPtr item, selectedItems())
    {
        const QString endpoint = joinPath(myPath, item->name);
        const QString destPath = dest->joinPath(dest->path(), item->name);
        bool ok = dest->linkFile(destPath, endpoint);
        if (! ok)
        {
            emit error(QString("Could not create link: %1 -> %2")
                       .arg(destPath)
                       .arg(endpoint));
        }
    }
    unselectAll();
    emit finished();
}

void FolderModel::newFolder(const QString& name)
{
    if (makeDirectory(joinPath(myPath, name)))
    {
        emit finished();
    }
    else
    {
        emit error("could not create folder");
    }
}

void FolderModel::setPath(const QString& path)
{
    loadDirectory(path);
    myPath = path;
    emit pathChanged();
}

void FolderModel::setMinDepth(int depth)
{
    myMinDepth = depth;
    emit minDepthChanged();
}

QStringList FolderModel::breadcrumbs() const
{
    QStringList crumbs;

    QDir dir(myPath);
    while (dir.cdUp())
    {
        qDebug() << "dir" << dir.dirName();
        if (dir.isRoot())
        {
            crumbs.prepend("System");
        }
        else
        {
            crumbs.prepend(dir.dirName());
        }
    }

    qDebug() << "breadcrumbs" << crumbs;
    return crumbs.mid(myMinDepth);
}

QString FolderModel::basename(const QString& path) const
{
    return QFileInfo(path).fileName();
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
        else
        {
            path += "/" + part;
        }
    }
    return path;
}

QString FolderModel::joinPath(const QString& p1, const QString& p2) const
{
    return joinPath(QStringList() << p1 << p2);
}

QList<QString> FolderModel::list(const QString& path) const
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

QString FolderModel::name() const
{
    return QDir(myPath).dirName();
}

void FolderModel::loadDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    QDir dir(path);

    beginResetModel();
    myItems.clear();
    mySelectionCount = 0;

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
            item->uri = joinPath(item->path, item->name);
            item->isSelected = false;
            item->type = type(finfo.absoluteFilePath());
            item->mimeType = mimeDb.mimeTypeForFile(finfo).name();
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

    endResetModel();
}

QList<FolderModel::Item::ConstPtr> FolderModel::selectedItems() const
{
    QList<Item::ConstPtr> items;
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->isSelected)
        {
            items << item;
        }
    }
    return items;
}

bool FolderModel::linkFile(const QString& path, const QString& source)
{
    qDebug() << "link file" << path << "to" << source;
    return QFile::link(source, path);
}
