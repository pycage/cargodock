#include "davmodel.h"
#include "davapi/davfile.h"

#include <QEventLoop>
#include <QList>
#include <QUrl>
#include <QDebug>

namespace
{

QString normalizePath(const QString& path)
{
    if (path.size() > 1 && path.endsWith("/"))
    {
        return path.left(path.size() - 1);
    }
    else
    {
        return path;
    }
}

}

DavModel::DavModel(QObject* parent)
    : FolderBase(parent)
    , myDavApi(new DavApi)
    , myMkColResult(DavApi::NoContent)
    , myDeleteResult(DavApi::NoContent)
    , myMoveResult(DavApi::NoContent)
    , myIsLoading(false)
{
    connect(myDavApi.data(), SIGNAL(propertiesReceived(int,DavApi::Properties)),
            this, SLOT(slotPropertiesReceived(int,DavApi::Properties)));
    connect(myDavApi.data(), SIGNAL(mkColFinished(int)),
            this, SLOT(slotMkColFinished(int)));
    connect(myDavApi.data(), SIGNAL(deleteFinished(int)),
            this, SLOT(slotDeleteFinished(int)));
    connect(myDavApi.data(), SIGNAL(moveFinished(int)),
            this, SLOT(slotMoveFinished(int)));
}

DavModel::DavModel(const DavModel& other)
    : FolderBase(other)
    , myDavApi(new DavApi)
    , myMkColResult(DavApi::NoContent)
    , myDeleteResult(DavApi::NoContent)
    , myMoveResult(DavApi::NoContent)
    , myIsLoading(false)
{
    connect(myDavApi.data(), SIGNAL(propertiesReceived(int,DavApi::Properties)),
            this, SLOT(slotPropertiesReceived(int,DavApi::Properties)));
    connect(myDavApi.data(), SIGNAL(mkColFinished(int)),
            this, SLOT(slotMkColFinished(int)));
    connect(myDavApi.data(), SIGNAL(deleteFinished(int)),
            this, SLOT(slotDeleteFinished(int)));
    connect(myDavApi.data(), SIGNAL(moveFinished(int)),
            this, SLOT(slotMoveFinished(int)));

    init();

    myDavApi->setAuthenticator(other.myDavApi->authenticator());
}

FolderBase* DavModel::clone() const
{
    DavModel* dolly = new DavModel(*this);
    return dolly;
}

void DavModel::init()
{
    const QString securityMethod = configValue("securityMethod").toString();
    const QString serverAddress = configValue("address").toString();

    const QString address = QString("%1://%2")
            .arg(securityMethod == "ssl" ? "https" : "http")
            .arg(serverAddress);

    myDavApi->setAddress(address);

    const QString login = configValue("login").toString();
    const QString password = configValue("password").toString();
    if (login.size())
    {
        myDavApi->setAuthorization(login, password);
    }
}

QVariant DavModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= itemCount())
    {
        return QVariant();
    }

    switch (role)
    {
    case PermissionsRole:
        return FolderBase::ReadOwner | FolderBase::WriteOwner;
    default:
        return FolderBase::data(index, role);
    }
}

int DavModel::capabilities() const
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

void DavModel::rename(const QString& name, const QString& newName)
{
    const QString source = joinPath(QStringList() << path() << name);
    const QString dest = joinPath(QStringList() << path() << newName);

    myMoveResult = 0;
    myDavApi->moveResource(source, dest);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myMoveResult == 0)
    {
        evLoop.processEvents();
    }
    if (myMoveResult == DavApi::Created || myMoveResult == DavApi::NoContent)
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
    else
    {
        emit error(QString("Could not rename file: %1").arg(name));
    }
}

QString DavModel::friendlyBasename(const QString& path) const
{
    if (normalizePath(path) == normalizePath(configValue("path").toString()))
    {
        return configValue("name").toString();
    }
    else
    {
        return basename(path).toUtf8();
    }
}

QIODevice* DavModel::openFile(const QString& path,
                              qint64 size,
                              QIODevice::OpenModeFlag mode)
{
    DavFile* fd = new DavFile(path, size, myDavApi);
    fd->open(mode);
    return fd;
}

bool DavModel::makeDirectory(const QString& path)
{
    myMkColResult = 0;
    myDavApi->mkcol(path);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myMkColResult == 0)
    {
        evLoop.processEvents();
    }
    return myMkColResult == DavApi::Created;
}

bool DavModel::deleteFile(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    myDeleteResult = 0;
    myDavApi->deleteResource(path);

    // this action needs to be synchronous, so wait for the response
    QEventLoop evLoop;
    while (myDeleteResult == 0)
    {
        evLoop.processEvents();
    }
    return myDeleteResult == DavApi::NoContent;
}

void DavModel::loadDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    clearItems();

    myIsLoading = true;
    emit loadingChanged();
    myDavApi->propfind(path);
}

void DavModel::handleResult(int result)
{
    switch (result)
    {
    case DavApi::ServerUnreachable:
        emit error("The server did not respond.");
        break;
    case DavApi::Unauthorized:
        emit error("You are not authorized.");
        break;
    case DavApi::Forbidden:
        emit error("Access denied.");
        break;
    case DavApi::NotFound:
        emit error("Resource is not available.");
        break;
    case DavApi::Gone:
        emit error("Resource is no longer available.");
        break;
    case DavApi::NotAllowed:
        emit error("This action is not allowed.");
        break;
    case DavApi::Conflict:
        emit error("Conflict detected.");
        break;
    case DavApi::UnsupportedMediaType:
        emit error("Media type not supported.");
        break;
    case DavApi::ServerError:
        emit error("Server error.");
        break;
    case DavApi::InsufficientStorage:
        emit error("The remote storage is full.");
        break;
    default:
        break;
    }
}

void DavModel::slotPropertiesReceived(int result, const DavApi::Properties& props)
{
    handleResult(result);

    if (result != DavApi::MultiStatus)
    {
        invalidateFolder();
    }

    if (props.href.size())
    {
        Item::Ptr item(new Item);
        item->selectable = true;
        item->name = props.name.toUtf8();
        item->path = parentPath(props.href);
        item->uri = joinPath(QStringList() << path() << item->name);
        item->type = props.resourceType == "collection" ? Folder : File;
        item->mimeType = props.contentType.size() ? props.contentType
                                                  : "application/x-octet-stream";
        item->size = props.contentLength;
        item->icon = item->type == Folder ? "image://theme/icon-m-folder"
                                          : mimeTypeIcon(item->mimeType);
        item->mtime = props.lastModified;

        appendItem(item);

        qDebug() << props.name << props.contentType << props.contentLength
                    << props.resourceType << props.lastModified;
    }
    else
    {
        myIsLoading = false;
        emit loadingChanged();
    }
}

void DavModel::slotMkColFinished(int result)
{
    myMkColResult = result;
}

void DavModel::slotDeleteFinished(int result)
{
    myDeleteResult = result;
}

void DavModel::slotMoveFinished(int result)
{
    myMoveResult = result;
}
