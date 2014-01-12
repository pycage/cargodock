#include "dropboxmodel.h"

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

DropboxModel::DropboxModel(QObject* parent)
    : FolderBase(parent)
    , myDropboxApi(new DropboxApi)
{
    myIcons.insert("application/octet-stream",                "image://theme/icon-m-other");
    myIcons.insert("application/pdf",                         "image://theme/icon-m-document");
    myIcons.insert("application/rtf",                         "image://theme/icon-m-document");
    myIcons.insert("application/vnd.android.package-archive", "image://theme/icon-m-device");
    myIcons.insert("application/xml",                         "image://theme/icon-m-document");
    myIcons.insert("application/x-core",                      "image://theme/icon-m-crash-reporter");
    myIcons.insert("application/x-executable",                "image://theme/icon-m-play");
    myIcons.insert("application/x-gzip",                      "image://theme/icon-m-other");
    myIcons.insert("application/x-redhat-package-manager",    "image://theme/icon-lock-system-update");
    myIcons.insert("application/x-sharedlib",                 "image://theme/icon-m-share");
    myIcons.insert("application/x-shellscript",               "image://theme/icon-m-document");
    myIcons.insert("application/x-sqlite3",                   "image://theme/icon-m-levels");
    myIcons.insert("application/x-x509-ca-cert",              "image://theme/icon-m-certificates");
    myIcons.insert("audio/mp4",                               "image://theme/icon-m-music");
    myIcons.insert("audio/mpeg",                              "image://theme/icon-m-music");
    myIcons.insert("image/jpeg",                              "image://theme/icon-m-image");
    myIcons.insert("image/png",                               "image://theme/icon-m-image");
    myIcons.insert("inode/directory",                         "image://theme/icon-m-folder");
    myIcons.insert("text/html",                               "image://theme/icon-m-region");
    myIcons.insert("text/plain",                              "image://theme/icon-m-document");
    myIcons.insert("text/vcard",                              "image://theme/icon-m-people");
    myIcons.insert("text/x-c++src",                           "image://theme/icon-m-document");
    myIcons.insert("text/x-qml",                              "image://theme/icon-m-document");
    myIcons.insert("video/mp4",                               "image://theme/icon-m-video");
    myIcons.insert("video/x-flv",                             "image://theme/icon-m-video");

    connect(myDropboxApi.data(), SIGNAL(authorizationRequest(QUrl,QUrl)),
            this, SIGNAL(authorizationRequired(QUrl,QUrl)));
    connect(myDropboxApi.data(), SIGNAL(authorized()),
            this, SLOT(slotAuthorized()));
    connect(myDropboxApi.data(), SIGNAL(error(DropboxApi::ErrorCode)),
            this, SLOT(slotError(DropboxApi::ErrorCode)));

    connect(myDropboxApi.data(), SIGNAL(metadataReceived(DropboxApi::Metadata)),
            this, SLOT(slotMetaDataReceived(DropboxApi::Metadata)));
    connect(myDropboxApi.data(), SIGNAL(folderCreated(QString)),
            this, SLOT(slotFolderCreated(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileMoved(QString)),
            this, SLOT(slotFileMoved(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileDeleted(QString)),
            this, SLOT(slotFileDeleted(QString)));

    QTimer::singleShot(0, this, SLOT(slotInit()));
}

int DropboxModel::rowCount(const QModelIndex&) const
{
    return myItems.size();
}

QVariant DropboxModel::data(const QModelIndex& index, int role) const
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
    return QVariant();
}

void DropboxModel::rename(const QString& name, const QString& newName)
{
    myRenamedPath = "/";
    const QString source = joinPath(QStringList() << path() << name);
    const QString dest = joinPath(QStringList() << path() << newName);
    myDropboxApi->moveFile(source, dest);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myRenamedPath == "/")
    {
        evLoop.processEvents();
    }
    if (myRenamedPath.isEmpty())
    {
        emit error(QString("Could not rename file: %1").arg(name));
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

QString DropboxModel::parentPath(const QString& path) const
{
    qDebug() << "parent path of" << path;
    int idx = path.lastIndexOf("/");
    if (idx == 0)
    {
        return "/";
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

QString DropboxModel::basename(const QString& path) const
{
    int idx = path.lastIndexOf("/");
    if (idx != -1)
    {
        return path.mid(idx + 1);
    }
    else
    {
        return path;
    }
}

QString DropboxModel::userBasename(const QString& path) const
{
    if (path == "/")
    {
        return "Dropbox";
    }
    else
    {
        return basename(path);
    }
}

QString DropboxModel::joinPath(const QStringList& parts) const
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

FolderBase::ItemType DropboxModel::type(const QString& path) const
{
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->path == path)
        {
            return item->type;
        }
    }
    return File;
}

QIODevice* DropboxModel::openFile(const QString& path,
                                  QIODevice::OpenModeFlag mode)
{
    /*
    QDropboxFile* fd = new QDropboxFile("/dropbox" + path, myDropbox.data());
    fd->open(mode);
    return fd;
    */
    return 0;
}

bool DropboxModel::makeDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    myNewFolderPath = "/";
    myDropboxApi->createFolder(path);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myNewFolderPath == "/")
    {
        evLoop.processEvents();
    }
    return ! myNewFolderPath.isEmpty();
}

bool DropboxModel::deleteFile(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    myDeletedPath = "/";
    myDropboxApi->deleteFile(path);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myDeletedPath == "/")
    {
        evLoop.processEvents();
    }
    return ! myDeletedPath.isEmpty();
}

void DropboxModel::loadDirectory(const QString& path)
{
    qDebug() << "loading directory" << path;

    beginResetModel();
    myItems.clear();
    endResetModel();

    if (myDropboxApi->accessToken().size())
    {
        myDropboxApi->requestMetadata(path);
    }
}

QString DropboxModel::itemName(int idx) const
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

void DropboxModel::authorize(const QUrl& uri)
{
    qDebug() << "Authorizing Dropbox client";
    myDropboxApi->authorize(uri);
    setPath(path());
}

void DropboxModel::slotInit()
{
    if (configValue("dropbox-access-token").isValid())
    {
        qDebug() << "Already authorized by Dropbox";
        myDropboxApi->setAccessToken(configValue("dropbox-access-token").toString(),
                                     configValue("dropbox-uid").toString());
        setPath(path());
    }
    else
    {
        qDebug() << "Authorization required for Dropbox";
        myDropboxApi->authorize();
    }
}

void DropboxModel::slotAuthorized()
{
    qDebug() << "got Dropbox access token";
    setConfigValue("dropbox-access-token", myDropboxApi->accessToken());
    setConfigValue("dropbox-uid", myDropboxApi->userId());
}

void DropboxModel::slotMetaDataReceived(const DropboxApi::Metadata& metadata)
{
    qDebug() << Q_FUNC_INFO << metadata.path << metadata.isDir;
    if (metadata.isDir)
    {
        foreach (const DropboxApi::Metadata& child, metadata.contents)
        {
            Item::Ptr item(new Item);
            item->name = basename(child.path);
            item->path = child.path;
            item->type = child.isDir ? Folder : File;
            item->mimeType = child.mimeType;
            item->mtime = child.mtime;
            item->size = child.bytes;

            if (child.thumb.size())
            {
                item->icon = QString("image://dropbox%1")
                        .arg(child.thumb);
            }
            else
            {
                item->icon = child.isDir
                        ? "image://theme/icon-m-folder"
                        : myIcons.value(item->mimeType, "image://theme/icon-m-other");
            }
            qDebug() << "Icon" << item->name << item->icon;

            beginInsertRows(QModelIndex(), myItems.size(), myItems.size());
            myItems << item;
            endInsertRows();
        }
    }
}

void DropboxModel::slotFolderCreated(const QString& path)
{
    myNewFolderPath = path;
}

void DropboxModel::slotFileMoved(const QString& path)
{
    myRenamedPath = path;
}

void DropboxModel::slotFileDeleted(const QString& path)
{
    myDeletedPath = path;
}

void DropboxModel::slotError(DropboxApi::ErrorCode errorCode)
{
    qDebug() << Q_FUNC_INFO << errorCode;
    emit error("Dropbox Error");
}
