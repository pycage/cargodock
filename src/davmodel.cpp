#include "davmodel.h"

DavModel::DavModel(QObject* parent)
    : FolderBase(parent)
{
}

QString DavModel::parentPath(const QString& path) const
{
    return QString();
}

QString DavModel::basename(const QString& path) const
{
    return QString();
}

QString DavModel::joinPath(const QStringList& parts) const
{
    return parts.join("/");
}

void DavModel::loadDirectory(const QString& path)
{

}

QString DavModel::itemName(int idx) const
{
    return QString();
}
