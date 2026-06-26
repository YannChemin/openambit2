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
    static QString defaultPath();
};

#endif // SPORTMODESTORAGE_H
