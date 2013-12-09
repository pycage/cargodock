#include "foldermodel.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

FolderModel::FolderModel(QObject* parent)
    : QAbstractListModel(parent)
    , myIsReadable(true)
{
    myRolenames.insert(NameRole, "name");
    myRolenames.insert(TypeRole, "type");
    myRolenames.insert(SelectedRole, "selected");
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
    case TypeRole:
        return item->type;
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
            if (item->type == Folder)
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
    }
    QVector<int> roles;
    roles << SelectedRole;
    emit dataChanged(index(idx), index(idx), roles);
}

void FolderModel::copySelected(FolderModel* dest)
{
    QList<Item::ConstPtr> selectedItems;
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->isSelected)
        {
            selectedItems << item;
        }
    }

    QDir dir(myPath);
    foreach (Item::ConstPtr item, selectedItems)
    {
        QFile file(dir.filePath(item->name));
        if (file.open(QIODevice::ReadOnly))
        {
            dest->openFile(item->name);
            dest->writeFile(file.readAll());
            dest->closeFile();
        }
    }
}

void FolderModel::linkSelected(FolderModel* dest)
{
    qDebug() << "linking";
    QList<Item::ConstPtr> selectedItems;
    foreach (Item::ConstPtr item, myItems)
    {
        if (item->isSelected)
        {
            selectedItems << item;
        }
    }

    QDir dir(myPath);
    foreach (Item::ConstPtr item, selectedItems)
    {
        dest->linkFile(dir.filePath(item->name));
    }
}

void FolderModel::setPath(const QString& path)
{
    loadDirectory(path);
    myPath = path;
    emit pathChanged();
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
    return crumbs;
}

QString FolderModel::name() const
{
    return QDir(myPath).dirName();
}

void FolderModel::loadDirectory(const QString& path)
{
    QDir dir(path);

    beginResetModel();
    myItems.clear();

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
            item->isSelected = false;
            if (finfo.isDir())
            {
                item->type = Folder;
            }
            else if (finfo.isSymLink())
            {
                item->type = Link;
            }
            else if (finfo.isFile())
            {
                item->type = File;
            }
            else
            {
                // unsupported
                continue;
            }

            qDebug() << "name" << item->name;
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

void FolderModel::openFile(const QString& filename)
{
    QDir dir(myPath);
    myCurrentWriteFile = new QFile(dir.filePath(filename));
    myCurrentWriteFile->open(QIODevice::WriteOnly);
    // TODO: emit error
}

void FolderModel::writeFile(const QByteArray& data)
{
    if (myCurrentWriteFile && myCurrentWriteFile->isOpen())
    {
        myCurrentWriteFile->write(data);
    }
}

void FolderModel::closeFile()
{
    if (myCurrentWriteFile && myCurrentWriteFile->isOpen())
    {
        myCurrentWriteFile->close();
    }
    delete myCurrentWriteFile;
    myCurrentWriteFile = 0;
}

void FolderModel::linkFile(const QString& source)
{
    QFileInfo sourceFile(source);
    QString name = sourceFile.fileName();
    qDebug() << "link file" << source << "to" << name;
    QFile::link(source, QDir(myPath).filePath(name));
}
