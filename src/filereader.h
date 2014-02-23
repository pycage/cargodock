#ifndef FILEREADER_H
#define FILEREADER_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QUrl>

/* Very simple file reader element for QML.
 */
class FileReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString data READ data NOTIFY sourceChanged)
public:

signals:
    void sourceChanged();

private:
    QString source() const
    {
        return mySource;
    }

    void setSource(const QString& source)
    {
        mySource = source;
        emit sourceChanged();
    }

    QString data()
    {
        if (mySource.isEmpty())
        {
            return QString();
        }

        QFile f(QUrl(mySource).path());
        if (f.open(QIODevice::ReadOnly))
        {
            return QString::fromUtf8(f.readAll());
        }
        else
        {
            return QString();
        }
    }

private:
    QString mySource;
};

#endif // FILEREADER_H
