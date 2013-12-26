#ifndef DEVELOPERMODE_H
#define DEVELOPERMODE_H

#include <QFile>
#include <QObject>

class DeveloperMode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled CONSTANT)
public:
    bool enabled() const
    {
        return QFile::exists("/usr/bin/devel-su");
    }
};

#endif // DEVELOPERMODE_H
