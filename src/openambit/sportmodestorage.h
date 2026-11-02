#ifndef SPORTMODESTORAGE_H
#define SPORTMODESTORAGE_H

#include <QString>
#include <QList>
#include <QVariantMap>

class SportModeStorage
{
public:
    static bool save(const QList<QVariantMap> &modes, const QString &path = defaultPath());
    static QList<QVariantMap> load(const QString &path = defaultPath());
    static QList<QVariantMap> factoryDefaults();
    // Templates for air sports (free flight, gliding, microlights, powered
    // aircraft, rotorcraft, ballooning, parachuting), offered when adding a
    // mode rather than included in the defaults: the watch holds 16 modes.
    static QList<QVariantMap> airSportPresets();
    static QString defaultPath();
};

#endif // SPORTMODESTORAGE_H
