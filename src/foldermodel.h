#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include "folderbase.h"

#include <QList>
#include <QMap>

class FolderModel : public FolderBase
{
    Q_OBJECT


public:
    FolderModel(QObject* parent = 0);
    virtual FolderBase* clone() const;

    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual bool isReadable() const { return myIsReadable; }
    virtual bool isWritable() const { return myIsWritable; }
    virtual int capabilities() const;

    Q_INVOKABLE virtual void setPermissions(const QString& name, int permissions);
    Q_INVOKABLE virtual void rename(const QString& name, const QString& newName);
    Q_INVOKABLE virtual QString readFile(const QString& name) const;
    Q_INVOKABLE virtual qint64 writeFile(const QString& name, const QByteArray& data) const;

    virtual QString friendlyBasename(const QString& path) const;

    virtual QStringList list(const QString& path) const;
    virtual QIODevice* openFile(const QString& path,
                                qint64 size,
                                QIODevice::OpenModeFlag mode);
    virtual bool makeDirectory(const QString& path);
    virtual bool linkFile(const QString& path,
                          const QString& source,
                          const FolderBase* sourceModel);
    virtual bool deleteFile(const QString& path);
    virtual void runFile(const QString& path);

protected:
    FolderModel(const FolderModel& other);

    virtual void loadDirectory(const QString& path);

    virtual QString mimeTypeIcon(const QString& mimeType) const;

private:
    QMap<QString, QString> myMimeTypeIcons;
    QMap<QString, QString> myPreviewComponents;

    bool myIsReadable;
    bool myIsWritable;
};

#endif // FOLDERMODEL_H
