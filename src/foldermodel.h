#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QIODevice>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

class FolderModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(ItemType)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QStringList breadcrumbs READ breadcrumbs NOTIFY pathChanged)
    Q_PROPERTY(QString name READ name NOTIFY pathChanged)
    Q_PROPERTY(bool isReadable READ isReadable NOTIFY pathChanged)
public:
    enum ItemType
    {
        Folder,
        File,
        Link
    };

    explicit FolderModel(QObject* parent = 0);

    virtual QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    Q_INVOKABLE void refresh();

    Q_INVOKABLE void open(const QString& name);
    Q_INVOKABLE void cdUp(int amount);

    Q_INVOKABLE void setSelected(int index, bool value);

    Q_INVOKABLE void copySelected(FolderModel* dest);
    Q_INVOKABLE void linkSelected(FolderModel* dest);

protected:
    void openFile(const QString& filename);
    void writeFile(const QByteArray& data);
    void closeFile();
    void linkFile(const QString& source);

signals:
    void pathChanged();

private:
    enum
    {
        NameRole,
        TypeRole,
        SelectedRole
    };

    struct Item
    {
        typedef QSharedPointer<Item> Ptr;
        typedef QSharedPointer<const Item> ConstPtr;

        QString name;
        ItemType type;
        bool isSelected;
    };

    QString path() const { return myPath; }
    void setPath(const QString& path);

    QStringList breadcrumbs() const;

    QString name() const;
    bool isReadable() const { return myIsReadable; }

    void loadDirectory(const QString& path);

private:
    QHash<int, QByteArray> myRolenames;
    QString myPath;
    QList<Item::Ptr> myItems;
    bool myIsReadable;

    QIODevice* myCurrentWriteFile;
};

#endif // FOLDERMODEL_H
