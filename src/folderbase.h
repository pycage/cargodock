#ifndef FOLDERBASE_H
#define FOLDERBASE_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QIODevice>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>

/* Base class for file system models.
 * In order to create a new file system model, derive from this class and
 * implement / override the virtual methods.
 */
class FolderBase : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(ItemType)
    Q_ENUMS(Permissions)
    Q_ENUMS(Capabilities)
    Q_PROPERTY(QString uid WRITE setUid CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY pathChanged)
    Q_PROPERTY(bool isReadable READ isReadable NOTIFY pathChanged)
    Q_PROPERTY(bool isWritable READ isWritable NOTIFY pathChanged)
    Q_PROPERTY(int capabilities READ capabilities NOTIFY selectionChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QStringList breadcrumbs READ breadcrumbs NOTIFY pathChanged)
    Q_PROPERTY(int selected READ selected NOTIFY selectionChanged)
    Q_PROPERTY(QStringList selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
public:
    enum ItemType
    {
        Folder,
        File,
        FolderLink,
        FileLink,
        Unsupported
    };

    enum Permissions
    {
        // values as used by QFile::Permission
        ReadOwner = 0x4000,
        WriteOwner = 0x2000,
        ExecOwner = 0x1000,
        ReadGroup = 0x40,
        WriteGroup = 0x20,
        ExecGroup = 0x10,
        ReadOther = 0x4,
        WriteOther = 0x2,
        ExecOther = 0x1
    };

    enum Capabilities
    {
        NoCapabilities = 0,

        // folder capabilities
        CanCopy = 1,
        AcceptCopy = 2,
        CanLink = 4,
        AcceptLink = 8,
        CanBookmark = 16,
        AcceptBookmark = 32,
        CanDelete = 64,

        // file capabilities
        CanOpen = 128,
        HasPermissions = 256
    };

    FolderBase(QObject* parent = 0);

    QString uid() const { return myUid; }

    virtual QHash<int, QByteArray> roleNames() const { return myRolenames; }
    virtual int rowCount(const QModelIndex&) const { return 0; }
    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual QString name() const { return userBasename(myPath); }
    virtual bool isReadable() const { return true; }
    virtual bool isWritable() const { return false; }

    /* Returns the capabilities that are currently available.
     * Capabilities targetting selected items may depend on the selection.
     */
    virtual int capabilities() const { return NoCapabilities; }

    QString path() const { return myPath; }
    void setPath(const QString& path);

    QStringList breadcrumbs() const;

    int selected() const { return mySelection.size(); }
    QStringList selection() const;

    Q_INVOKABLE void open(const QString& name);
    Q_INVOKABLE void copySelected(FolderBase* dest);
    Q_INVOKABLE void deleteItems(const QStringList& items);
    Q_INVOKABLE void linkSelected(FolderBase* dest);
    Q_INVOKABLE void newFolder(const QString& name);

    Q_INVOKABLE virtual void setPermissions(const QString& name, int permissions);
    Q_INVOKABLE virtual void rename(const QString& name, const QString& newName);
    Q_INVOKABLE virtual QString readFile(const QString& name) const;

    /* Refreshes the current folder by reloading its contents.
     */
    Q_INVOKABLE void refresh();

    /* Goes up the given amount of directories in the hierarchy.
     */
    Q_INVOKABLE void cdUp(int amount);

    /* Selects or unselects the given item.
     */
    Q_INVOKABLE void setSelected(int index, bool value);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void unselectAll();
    Q_INVOKABLE void invertSelection();


    /* Returns the parent directory of the given path. If there is no parent,
     * the path is returned.
     */
    virtual QString parentPath(const QString& path) const = 0;

    /* Returns the basename of the file specified by path.
     */
    virtual QString basename(const QString& path) const = 0;

    /* Returns a user-presentable basename of the file specified by path.
     */
    virtual QString userBasename(const QString& path) const
    {
        return basename(path);
    }

    /* Joins the given parts of a path.
     */
    virtual QString joinPath(const QStringList& parts) const = 0;

    /* Lists the contents of the given path.
     */
    virtual QStringList list(const QString& path) const;

    /* Returns the type of the file specified by path.
     */
    virtual ItemType type(const QString& path) const;

    /* Opens the given file and returns a QIODevice* to access it. The caller
     * is obliged to check for validity of the QIODevice* and to close and
     * delete it when finished.
     */
    virtual QIODevice* openFile(const QString& path,
                                QIODevice::OpenModeFlag mode);

    /* Creates the given directory. Returns true if successful.
     */
    virtual bool makeDirectory(const QString& path);

    /* Creates a link pointing to the given source. Returns true if successful.
     */
    virtual bool linkFile(const QString& path,
                          const QString& source,
                          const FolderBase* sourceModel);

    /* Deletes the given file or directory. Returns true if successful.
     */
    virtual bool deleteFile(const QString& path);

    /* Runs the given file.
     */
    virtual void runFile(const QString& path);

signals:
    void pathChanged();
    void selectionChanged();
    void finished();
    void error(const QString& details);
    void loadingChanged();

protected:
    enum
    {
        NameRole,
        SectionRole,
        PathRole,
        UriRole,
        PreviewRole,
        TypeRole,
        MimeTypeRole,
        IconRole,
        SizeRole,
        MtimeRole,
        OwnerRole,
        GroupRole,
        PermissionsRole,
        LinkTargetRole,
        ModelTargetRole,
        SelectableRole,
        SelectedRole,
        CapabilitiesRole
    };

    virtual void init() { }

    void setConfigValue(const QString& key,
                        const QVariant& value);
    QVariant configValue(const QString& key) const;

    void setConfigValue(const QString& uid,
                        const QString& key,
                        const QVariant& value);
    QVariant configValue(const QString& uid,
                         const QString& key) const;
    void removeConfigValues(const QString& uid);
    void cloneConfigValues(const QString& uid, const QString& cloneUid);

    virtual bool loading() const { return false; }

    virtual void loadDirectory(const QString& path) = 0;
    virtual QString itemName(int idx) const = 0;
    virtual bool isSelected(int idx) const;

    virtual QString mimeTypeIcon(const QString& mimeType) const;

private:
    void setUid(const QString& uid);

private:
    QHash<int, QByteArray> myRolenames;
    QMap<QString, QString> myMimeTypeIcons;

    QString myUid;
    QString myPath;
    int myMinDepth;
    QSet<int> mySelection;
};

#endif // FOLDERBASE_H
