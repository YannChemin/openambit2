#include "trainingplanstorage.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QString TrainingPlanStorage::defaultPath()
{
    return QString(getenv("HOME")) + "/.openambit/training_plans.json";
}

QString TrainingPlanStorage::compiledAppsDir(const QString &planName)
{
    QString safeName = planName;
    safeName.replace(QRegExp("[^A-Za-z0-9_ -]"), "_");
    return QString(getenv("HOME")) + "/.openambit/training_plan_apps/" + safeName;
}

bool TrainingPlanStorage::save(const QList<TrainingPlan> &plans, const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QJsonArray arr;
    for (const TrainingPlan &plan : plans) {
        arr.append(QJsonObject::fromVariantMap(plan.toMap()));
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    f.write(QJsonDocument(arr).toJson());
    return true;
}

QList<TrainingPlan> TrainingPlanStorage::load(const QString &path)
{
    QList<TrainingPlan> result;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return result;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        return result;
    }
    for (const QJsonValue &v : doc.array()) {
        if (v.isObject()) {
            result.append(TrainingPlan(v.toObject().toVariantMap()));
        }
    }
    return result;
}
