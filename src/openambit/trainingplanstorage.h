#ifndef TRAININGPLANSTORAGE_H
#define TRAININGPLANSTORAGE_H

#include <QString>
#include <QList>

#include <movescount/trainingplan.h>

class TrainingPlanStorage
{
public:
    static bool save(const QList<TrainingPlan> &plans, const QString &path = defaultPath());
    static QList<TrainingPlan> load(const QString &path = defaultPath());
    static QString defaultPath();

    // Where a plan's compiled app JSON files (pasted back from the compiler website) get
    // saved, one subdirectory per plan name.
    static QString compiledAppsDir(const QString &planName);
};

#endif // TRAININGPLANSTORAGE_H
