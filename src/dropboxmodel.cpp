#include "dropboxmodel.h"
#include "dropboxapi/dropboxfile.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QString>
#include <QDebug>

namespace
{
const QString COOKIE_PATH_1(QDir::homePath() +
                            "/.local/share/harbour-cargodock/"
                            ".QtWebKit/cookies.db");
const QString COOKIE_PATH_2(QDir::homePath() +
                            "/.local/share/harbour-cargodock/harbour-cargodock/"
                            ".QtWebKit/cookies.db");
}

DropboxModel::DropboxModel(QObject* parent)
    : FolderBase(parent)
    , myDropboxApi(new DropboxApi)
    , myUserName("-")
    , myIsLoading(false)
{
    connect(myDropboxApi.data(), SIGNAL(authorizationRequest(QUrl,QUrl)),
            this, SIGNAL(authorizationRequired(QUrl,QUrl)));
    connect(myDropboxApi.data(), SIGNAL(authorized()),
            this, SLOT(slotAuthorized()));
    connect(myDropboxApi.data(), SIGNAL(error(DropboxApi::ErrorCode)),
            this, SLOT(slotError(DropboxApi::ErrorCode)));

    connect(myDropboxApi.data(), SIGNAL(accountInfoReceived(DropboxApi::AccountInfo)),
            this, SLOT(slotAccountInfoReceived(DropboxApi::AccountInfo)));
    connect(myDropboxApi.data(), SIGNAL(metadataReceived(DropboxApi::Metadata)),
            this, SLOT(slotMetaDataReceived(DropboxApi::Metadata)));
    connect(myDropboxApi.data(), SIGNAL(folderCreated(QString)),
            this, SLOT(slotFolderCreated(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileMoved(QString)),
            this, SLOT(slotFileMoved(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileDeleted(QString)),
            this, SLOT(slotFileDeleted(QString)));
}

DropboxModel::DropboxModel(const DropboxModel& other)
    : FolderBase(other)
    , myDropboxApi(new DropboxApi)
    , myUserName(other.myUserName)
    , myIsLoading(false)
{
    connect(myDropboxApi.data(), SIGNAL(authorizationRequest(QUrl,QUrl)),
            this, SIGNAL(authorizationRequired(QUrl,QUrl)));
    connect(myDropboxApi.data(), SIGNAL(authorized()),
            this, SLOT(slotAuthorized()));
    connect(myDropboxApi.data(), SIGNAL(error(DropboxApi::ErrorCode)),
            this, SLOT(slotError(DropboxApi::ErrorCode)));

    connect(myDropboxApi.data(), SIGNAL(accountInfoReceived(DropboxApi::AccountInfo)),
            this, SLOT(slotAccountInfoReceived(DropboxApi::AccountInfo)));
    connect(myDropboxApi.data(), SIGNAL(metadataReceived(DropboxApi::Metadata)),
            this, SLOT(slotMetaDataReceived(DropboxApi::Metadata)));
    connect(myDropboxApi.data(), SIGNAL(folderCreated(QString)),
            this, SLOT(slotFolderCreated(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileMoved(QString)),
            this, SLOT(slotFileMoved(QString)));
    connect(myDropboxApi.data(), SIGNAL(fileDeleted(QString)),
            this, SLOT(slotFileDeleted(QString)));

    init();
}

FolderBase* DropboxModel::clone() const
{
    DropboxModel* dolly = new DropboxModel(*this);
    return dolly;
}

QVariant DropboxModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= itemCount())
    {
        return QVariant();
    }

    Item::ConstPtr item = itemAt(index.row());

    switch (role)
    {
    case PreviewRole:
        return item->mimeType.startsWith("image/")
                ? "PreviewImage#" + item->icon + "?large"
                : QVariant();
    default:
        return FolderBase::data(index, role);
    }
}

int DropboxModel::capabilities() const
{
    int caps = AcceptCopy;
    if (selected() > 0)
    {
        bool canBookmark = true;
        foreach (const QString& path, selection())
        {
            Item::ConstPtr item = itemByName(basename(path));
            if (item && item->type != Folder)
            {
                canBookmark = false;
                break;
            }
        }

        caps |= (canBookmark ? CanBookmark : NoCapabilities) |
                CanCopy |
                CanDelete;
    }
    return caps;
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
        Item::Ptr item = itemByName(name);
        if (! item.isNull())
        {
            item->name = newName;
            item->uri = dest;
            QVector<int> roles;
            roles << NameRole << UriRole;
            int idx = findItem(item);
            emit dataChanged(index(idx), index(idx), roles);
        }
    }
}

QString DropboxModel::friendlyBasename(const QString& path) const
{
    if (path == "/")
    {
        return configValue("name").toString();
    }
    else
    {
        return basename(path);
    }
}

QIODevice* DropboxModel::openFile(const QString& path,
                                  qint64 size,
                                  QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(size)
    DropboxFile* fd = new DropboxFile(path, myDropboxApi);
    fd->open(mode);
    return fd;
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

    clearItems();

    if (myDropboxApi->accessToken().size())
    {
        myIsLoading = true;
        emit loadingChanged();
        myDropboxApi->requestListFolder(path);
    }
}

void DropboxModel::authorize(const QUrl& uri)
{
    qDebug() << "Authorizing Dropbox client";
    myDropboxApi->authorize(uri);
    setPath(path());
}

void DropboxModel::init()
{
    if (configValue("dropbox-access-token").isValid())
    {
        qDebug() << "Already authorized by Dropbox";
        myDropboxApi->setAccessToken(configValue("dropbox-access-token").toString(),
                                     configValue("dropbox-uid").toString());
    }
    else
    {
        qDebug() << "Authorization required for Dropbox";
        if (QFile(COOKIE_PATH_1).exists())
        {
            QFile(COOKIE_PATH_1).remove();
        }
        if (QFile(COOKIE_PATH_2).exists())
        {
            QFile(COOKIE_PATH_2).remove();
        }
        myDropboxApi->authorize();
    }
}

void DropboxModel::slotAuthorized()
{
    qDebug() << "got Dropbox access token";
    setConfigValue("dropbox-access-token", myDropboxApi->accessToken());
    setConfigValue("dropbox-uid", myDropboxApi->userId());
}

void DropboxModel::slotAccountInfoReceived(const DropboxApi::AccountInfo& info)
{
    myUserName = info.displayName;
}

void DropboxModel::slotMetaDataReceived(const DropboxApi::Metadata& metadata)
{
    qDebug() << Q_FUNC_INFO << metadata.path << metadata.isDir;
    if (metadata.isDir)
    {
        foreach (const DropboxApi::Metadata& child, metadata.contents)
        {
            Item::Ptr item(new Item);
            item->selectable = true;
            item->name = basename(child.path);
            item->path = parentPath(child.path);
            item->uri = child.path;
            item->type = child.isDir ? Folder : File;
            item->mimeType = child.mimeType;
            item->mtime = child.mtime;
            item->size = child.bytes;
            item->owner = myUserName;
            item->permissions = FolderBase::ReadOwner | FolderBase::WriteOwner;

            if (child.thumb.size())
            {
                item->icon = QString("image://dropbox%1")
                        .arg(child.thumb);
            }
            else
            {
                item->icon = child.isDir
                        ? "image://theme/icon-m-folder"
                        : mimeTypeIcon(item->mimeType);
            }
            qDebug() << "Icon" << item->name << item->icon;

            appendItem(item);
        }

        myIsLoading = false;
        emit loadingChanged();
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
