#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

class FolderModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(ItemType)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int minDepth READ minDepth WRITE setMinDepth
               NOTIFY minDepthChanged)
    Q_PROPERTY(QStringList breadcrumbs READ breadcrumbs NOTIFY pathChanged)
    Q_PROPERTY(QString name READ name NOTIFY pathChanged)
    Q_PROPERTY(bool isReadable READ isReadable NOTIFY pathChanged)
    Q_PROPERTY(int selected READ selected NOTIFY selectionChanged)
public:
    enum ItemType
    {
        Folder,
        File,
        FolderLink,
        FileLink,
        Unsupported
    };

    explicit FolderModel(QObject* parent = 0);

    virtual QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    QString path() const { return myPath; }
    void setPath(const QString& path);

    int minDepth() const { return myMinDepth; }
    void setMinDepth(int depth);

    QStringList breadcrumbs() const;

    QString name() const;
    bool isReadable() const { return myIsReadable; }

    /* Refreshes the current folder by reloading its contents.
     */
    Q_INVOKABLE void refresh();

    Q_INVOKABLE void open(const QString& name);
    Q_INVOKABLE void cdUp(int amount);

    /* Selects or unselects the given item.
     */
    Q_INVOKABLE void setSelected(int index, bool value);

    Q_INVOKABLE void unselectAll();

    Q_INVOKABLE void copySelected(FolderModel* dest);
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void linkSelected(FolderModel* dest);
    Q_INVOKABLE void newFolder(const QString& name);

    /* Return the basename of the file specified by path.
     */
    QString basename(const QString& path) const;
    /* Joins parts together to a path, using the right separator.
     */
    QString joinPath(const QStringList& parts) const;
    QString joinPath(const QString& p1, const QString& p2) const;
    /* Lists the contents of the given path.
     */
    QList<QString> list(const QString& path) const;
    /* Returns the type of the file specified by path.
     */
    ItemType type(const QString& path) const;
    /* Opens the given file and returns a QIODevice* to access it. The caller
     * is obliged to check for validity of the QIODevice* and to close and
     * delete it when finished.
     */
    QIODevice* openFile(const QString& path, QIODevice::OpenModeFlag mode);
    /* Creates the given directory. Returns true if successful.
     */
    bool makeDirectory(const QString& path);
    /* Creates a link pointing to the given source. Returns true if successful.
     */
    bool linkFile(const QString& path, const QString& source);

signals:
    void pathChanged();
    void minDepthChanged();
    void selectionChanged();
    void finished();
    void error(const QString& details);

private:
    enum
    {
        NameRole,
        PathRole,
        UriRole,
        TypeRole,
        MimeTypeRole,
        IconRole,
        SizeRole,
        MtimeRole,
        LinkTargetRole,
        SelectedRole
    };

    struct Item
    {
        typedef QSharedPointer<Item> Ptr;
        typedef QSharedPointer<const Item> ConstPtr;

        QString name;
        QString path;
        QString uri;
        ItemType type;
        QString mimeType;
        QString icon;
        qint64 size;
        QDateTime mtime;
        QString linkTarget;
        bool isSelected;
    };

    void loadDirectory(const QString& path);
    QList<Item::ConstPtr> selectedItems() const;

    int selected() const { return mySelectionCount; }

private:
    QHash<int, QByteArray> myRolenames;
    QMap<QString, QString> myIcons;
    QString myPath;
    int myMinDepth;
    int mySelectionCount;
    QList<Item::Ptr> myItems;
    bool myIsReadable;

    QIODevice* myCurrentWriteFile;
};

#endif // FOLDERMODEL_H
