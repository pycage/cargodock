#include "placesmodel.h"
#include "developermode.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

namespace
{
const QString SD_CARD("/run/user/100000/media/sdcard");
const QString ANDROID_STORAGE("/data/sdcard");
}

PlacesModel::PlacesModel(QObject* parent)
    : FolderBase(parent)
{
    DeveloperMode developerMode;

    myPlaces << Item::Ptr(new Item("Documents",
                                   "image://theme/icon-m-document",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0]))
             << Item::Ptr(new Item("Downloads",
                                   "image://theme/icon-m-cloud-download",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]))
             << Item::Ptr(new Item("Music",
                                   "image://theme/icon-m-music",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::MusicLocation)[0]))
             << Item::Ptr(new Item("Videos",
                                   "image://theme/icon-m-video",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0]))
             << Item::Ptr(new Item("Pictures",
                                   "image://theme/icon-m-image",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0]))
             << Item::Ptr(new Item("Camera",
                                   "image://theme/icon-m-camera",
                                   "local",
                                   QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0] + "/Camera"));

    if (QDir(SD_CARD).exists())
    {
        myPlaces << Item::Ptr(new Item("SD Card",
                                       "image://theme/icon-m-device",
                                       "local",
                                       SD_CARD));
    }

    if (QDir(ANDROID_STORAGE).exists())
    {
        myPlaces << Item::Ptr(new Item("Android Storage",
                                       "image://theme/icon-m-folder",
                                       "local",
                                       ANDROID_STORAGE));
    }

    if (developerMode.enabled())
    {
        myPlaces << Item::Ptr(new Item("System",
                                       "image://theme/icon-m-folder",
                                       "local",
                                       "/"));
    }
}

int PlacesModel::rowCount(const QModelIndex&) const
{
    return myPlaces.size();
}

QVariant PlacesModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= myPlaces.size())
    {
        return QVariant();
    }

    Item::ConstPtr item = myPlaces.at(index.row());

    switch (role)
    {
    case NameRole:
        return item->name;
    case IconRole:
        return item->icon;
    case LinkTargetRole:
        return item->path;
    case ModelTargetRole:
        return item->model;
    case SelectedRole:
        return isSelected(index.row());
    case MtimeRole:
        return QDateTime();
    default:
        return QVariant();
    }
}

void PlacesModel::loadDirectory(const QString&)
{

}

QString PlacesModel::itemName(int idx) const
{
    if (idx < myPlaces.size())
    {
        return myPlaces[idx]->name;
    }
    else
    {
        return QString();
    }
}
