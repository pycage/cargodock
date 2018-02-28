#ifndef DEVELOPERMODE_H
#define DEVELOPERMODE_H

#include <QFile>
#include <QObject>
#include <unistd.h>

class DeveloperMode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled CONSTANT)
    Q_PROPERTY(bool isRoot READ isRoot CONSTANT)
public:
    bool enabled() const
    {
        return inDebug||QFile::exists("/usr/bin/devel-su");
    }
    bool isRoot() const
    {
        return (geteuid() == 0);
    }
    static bool inDebug;
};

#endif // DEVELOPERMODE_H
