#ifndef FOLDERACTION_H
#define FOLDERACTION_H

#include <QObject>
#include <QString>

class FolderAction : public QObject
{
    Q_OBJECT
public:
    virtual void start() = 0;

signals:
    void progress(const QString& name, double amount);
    void finished();
    void error(const QString& details);
};

#endif // FOLDERACTION_H
