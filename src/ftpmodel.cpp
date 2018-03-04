#include "ftpmodel.h"
#include "ftpapi/ftpapi.h"
#include "ftpapi/ftpfile.h"

#include <QMimeDatabase>
#include <QRegExp>

namespace
{
// lrwxr-xr-x    1 root  wheel     49 Dec 28  2012 Benutzerinformationen -> /Library/Documentation/User Information.localized
const QRegExp RE_UNIX_LS("^(\\S+) *(\\d+) *(\\w+) *(\\w+) *(\\d+) *"
                         "(\\S+) *(\\S+) *(\\S+) *(\\S.+)$");
}

FtpModel::FtpModel(QObject* parent)
    : FolderBase(parent)
    , myIsLoading(false)
    , myFtpApi(new FtpApi)
{

}

FtpModel::FtpModel(const FtpModel& other)
    : FolderBase(other)
    , myIsLoading(false)
    , myFtpApi(new FtpApi)
{
    init();
}

FolderBase* FtpModel::clone() const
{
    FtpModel* dolly = new FtpModel(*this);
    return dolly;
}

void FtpModel::init()
{
    connect(myFtpApi.data(), SIGNAL(error(QString)),
            this, SLOT(slotError(QString)));
    connect(myFtpApi.data(), SIGNAL(listReceived(QString)),
            this, SLOT(slotListReceived(QString)));

    const QString serverAddress = configValue("address").toString();
    myFtpApi->setAddress(serverAddress);

    const QString login = configValue("login").toString();
    const QString password = configValue("password:blowfish").toString();
    if (login.size())
    {
        myFtpApi->setCredentials(login, password);
    }
}

QVariant FtpModel::data(const QModelIndex& index, int role) const
{
    if (! index.isValid() || index.row() >= itemCount())
    {
        return QVariant();
    }

    switch (role)
    {
    case PermissionsRole:
        return FolderBase::ReadOwner || FolderBase::WriteOwner;
    default:
        return FolderBase::data(index, role);
    }
}

int FtpModel::capabilities() const
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

QString FtpModel::friendlyBasename(const QString& path) const
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

QIODevice* FtpModel::openFile(const QString& path,
                              qint64 size,
                              QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(size)

    FtpFile* fd = new FtpFile(path, myFtpApi);
    fd->open(mode);
    return fd;
}

void FtpModel::loadDirectory(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;
    clearItems();
    myIsLoading = true;
    emit loadingChanged();
    myFtpApi->list(path);
}

void FtpModel::slotError(const QString& details)
{
    qDebug() << Q_FUNC_INFO << details;
    emit error(details);
}

void FtpModel::slotListReceived(const QString& data)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << data;

    if (data.size())
    {
        QMimeDatabase mimeDb;

        foreach (const QString& line, data.split('\n'))
        {

            if (RE_UNIX_LS.indexIn(line.trimmed()) != -1)
            {
                // lrwxr-xr-x    1 root  wheel     49 Dec 28  2012 Benutzerinformationen -> /Library/Documentation/User Information.localized
                Item::Ptr item(new Item);
                item->selectable = true;
                item->name = RE_UNIX_LS.cap(9);
                item->path = path();
                item->uri = joinPath(QStringList() << item->path << item->name);
                item->size = RE_UNIX_LS.cap(5).toLongLong();
                item->owner = RE_UNIX_LS.cap(3);
                item->type = RE_UNIX_LS.cap(1).startsWith("d") ? Folder : File;
                item->mimeType = mimeDb.mimeTypeForName(item->name).name(); //"application/x-octet-stream";
                item->icon = item->type == Folder ? "image://theme/icon-m-folder"
                                                  : mimeTypeIcon(item->mimeType);

                QString month = RE_UNIX_LS.cap(6);
                QString day = RE_UNIX_LS.cap(7);
                QString yearOrTime = RE_UNIX_LS.cap(8);
                QString year = yearOrTime.contains(":") ? QString::number(QDate::currentDate().year())
                                                        : yearOrTime;
                QString time = yearOrTime.contains(":") ? yearOrTime
                                                        : "00:00";

                item->mtime = QDateTime::fromString(QString("%1 %2 %3 %4")
                                                    .arg(year)
                                                    .arg(month)
                                                    .arg(day)
                                                    .arg(time),
                                                    "yyyy MMM dd HH:mm");
                appendItem(item);
            }

        }
    }
    else
    {
        myIsLoading = false;
        emit loadingChanged();
    }


}
