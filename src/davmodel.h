#ifndef DAVMODEL_H
#define DAVMODEL_H

#include "folderbase.h"

class DavModel : public FolderBase
{
    Q_OBJECT
public:
    DavModel(QObject* parent = 0);

    virtual QString parentPath(const QString& path) const;
    virtual QString basename(const QString& path) const;
    virtual QString joinPath(const QStringList& parts) const;

protected:
    virtual void loadDirectory(const QString& path);
    virtual QString itemName(int idx) const;
};

#endif // DAVMODEL_H
