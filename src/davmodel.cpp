#include "davmodel.h"
#include "network.h"

#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>

#include <QDebug>

namespace
{

const QString NS_DAV("DAV:");

struct Resource
{
    QString name;
    QString href;
    QString resourceType;
    QString contentType;
    int contentLength;
    QDateTime lastModified;

};

Resource parseResult(const QDomElement& elem)
{
    Resource resource;

    QDomNodeList nodes = elem.elementsByTagNameNS(NS_DAV, "href");
    if (nodes.length())
    {
        const QString href = nodes.at(0).toElement().text();
        const QStringList parts = href.split("/");
        resource.name = parts.size() ? parts.last() : QString();
        resource.href = href;
    }

    nodes = elem.elementsByTagNameNS(NS_DAV, "resourcetype");
    if (nodes.length())
    {
        const QDomNode typeNode = nodes.at(0).firstChild();
        resource.resourceType = typeNode.toElement().tagName();
    }

    nodes = elem.elementsByTagNameNS(NS_DAV, "getcontenttype");
    if (nodes.length())
    {
        resource.contentType = nodes.at(0).toElement().text();
    }

    nodes = elem.elementsByTagNameNS(NS_DAV, "getlastmodified");
    if (nodes.length())
    {
        resource.lastModified = QDateTime::fromString(nodes.at(0).toElement().text());
    }

    nodes = elem.elementsByTagNameNS(NS_DAV, "getcontentlength");
    if (nodes.length())
    {
        resource.contentLength = nodes.at(0).toElement().text().toInt();
    }

    return resource;
}

QList<Resource> parseResult(const QByteArray& data)
{
    QList<Resource> result;

    QDomDocument doc;
    doc.setContent(data, true);

    QDomNodeList nodes = doc.elementsByTagNameNS(NS_DAV, "response");
    for (int i = 0; i < nodes.length(); ++i)
    {
        QDomNode node = nodes.at(i);
        if (node.isElement())
        {
            result << parseResult(node.toElement());
        }
    }

    return result;
}

}

DavModel::DavModel(QObject* parent)
    : FolderBase(parent)
    , myIsLoading(false)
{
}

QVariant DavModel::data(const QModelIndex& index, int role) const
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
        return "-";
    case GroupRole:
        return "-";
    case PermissionsRole:
        return FolderBase::ReadOwner | FolderBase::WriteOwner;
    case LinkTargetRole:
        return item->linkTarget;
    default:
        return FolderBase::data(index, role);
    }
}

QString DavModel::parentPath(const QString& path) const
{
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

QString DavModel::basename(const QString& path) const
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

QString DavModel::userBasename(const QString& path) const
{
    if (path == "/")
    {
        return "DAV";
    }
    else
    {
        return basename(path);
    }
}

QString DavModel::joinPath(const QStringList& parts) const
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

FolderBase::ItemType DavModel::type(const QString& path) const
{
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->uri == path)
        {
            return item->type;
        }
    }
    return File;
}

void DavModel::loadDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << configValue("url") << path;

    QUrl url(configValue("url").toString());
    url.setPath(QUrl::toPercentEncoding(path, "/"));

    beginResetModel();
    myItems.clear();
    endResetModel();

    QNetworkAccessManager* nam = Network::accessManager();
    if (nam)
    {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(0));
        req.setRawHeader("Depth", "1");
        QNetworkReply* reply = nam->sendCustomRequest(req, "PROPFIND");

        connect(reply, SIGNAL(finished()),
                this, SLOT(slotMetaDataReceived()));

        myIsLoading = true;
        emit loadingChanged();
    }
}

QString DavModel::itemName(int idx) const
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

void DavModel::slotMetaDataReceived()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();
    qDebug() << data;

    QList<Resource> resources = parseResult(data);
    foreach (const Resource& resource, resources.mid(1))
    {
        Item::Ptr item(new Item);
        item->name = QUrl::fromPercentEncoding(resource.name.toUtf8());
        item->path = parentPath(resource.href);
        item->uri = joinPath(QStringList() << path() << item->name);
        item->type = resource.resourceType == "collection" ? Folder : File;
        item->mimeType = resource.contentType.size() ? resource.contentType
                                                     : "application/x-octet-stream";
        item->size = resource.contentLength;
        item->icon = item->type == Folder ? "image://theme/icon-m-folder"
                                          : mimeTypeIcon(item->mimeType);

        beginInsertRows(QModelIndex(), myItems.size(), myItems.size());
        myItems << item;
        endInsertRows();

        qDebug() << resource.name << resource.contentType << resource.contentLength
                    << resource.resourceType << resource.lastModified;
    }

    myIsLoading = false;
    emit loadingChanged();
}
