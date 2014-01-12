#ifndef DROPBOXTHUMBPROVIDER_H
#define DROPBOXTHUMBPROVIDER_H

#include <QByteArray>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QQuickImageProvider>
#include <QSharedPointer>
#include <QSize>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

/* QML image provider for Dropbox thumbnails.
 * This image provider is blocking, so the Image element MUST have the
 * 'asynchronous' property set to 'true'.
 */
class DropboxThumbProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    DropboxThumbProvider();

    virtual QImage requestImage(const QString& id,
                                QSize* size,
                                const QSize& requestedSize);

signals:
    void loadImage(const QString& accessToken, const QString& path);

private slots:
    void slotLoadImage(const QString& accessToken, const QString& path);
    void slotImageLoaded(QNetworkReply* reply);

private:
    QMutex myMutex;
    QSharedPointer<QNetworkAccessManager> myNetworkAccessManager;
    QMap<QString, QByteArray> myImages;
};

#endif // DROPBOXTHUMBPROVIDER_H
