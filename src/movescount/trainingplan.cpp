#include "trainingplan.h"

#include <QDate>
#include <QVariantList>
#include <algorithm>

const QString TrainingPlanEntry::DATE = "date";
const QString TrainingPlanEntry::WORKOUT = "workout";

TrainingPlanEntry::TrainingPlanEntry(const QVariantMap &map) :
    workout(map.value(WORKOUT).toMap())
{
    date = map.value(DATE).toString();
}

QVariantMap TrainingPlanEntry::toMap() const
{
    QVariantMap map;
    map.insert(DATE, date);
    map.insert(WORKOUT, workout.toMap());
    return map;
}

const QString TrainingPlan::NAME = "name";
const QString TrainingPlan::ENTRIES = "entries";

TrainingPlan::TrainingPlan(const QVariantMap &map)
{
    if (map.contains(NAME)) {
        name = map[NAME].toString();
    }
    for (const QVariant &entryVariant : map.value(ENTRIES).toList()) {
        entries.append(TrainingPlanEntry(entryVariant.toMap()));
    }
}

QVariantMap TrainingPlan::toMap() const
{
    QVariantMap map;
    map.insert(NAME, name);
    QVariantList entryMaps;
    for (const TrainingPlanEntry &entry : entries) {
        entryMaps.append(entry.toMap());
    }
    map.insert(ENTRIES, entryMaps);
    return map;
}

/* --------------------------------------------------------------------------------------- */

const int TrainingPlanScheduler::MAX_WORKOUTS_PER_APP_GUESS;

static const QDate EPOCH_2000(2000, 1, 1);

qint64 TrainingPlanScheduler::dateToDays2000(const QString &isoDate)
{
    QDate d = QDate::fromString(isoDate, "yyyy-MM-dd");
    return EPOCH_2000.daysTo(d);
}

QString TrainingPlanScheduler::indent(const QString &source, const QString &prefix)
{
    QStringList out;
    for (const QString &line : source.split('\n')) {
        out << (line.isEmpty() ? line : prefix + line);
    }
    return out.join('\n');
}

bool TrainingPlanScheduler::buildAppSource(const QList<TrainingPlanEntry> &entries, QString &source, QStringList &ownVars, QString &errorString)
{
    if (entries.isEmpty()) {
        errorString = "pack has no entries";
        return false;
    }

    QList<TrainingPlanEntry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(), [](const TrainingPlanEntry &a, const TrainingPlanEntry &b) {
        return a.date < b.date;
    });

    QStringList mergedVars;
    QStringList gates;

    for (const TrainingPlanEntry &entry : sorted) {
        QString body;
        QStringList entryVars;
        if (!AppZoneGenerator::generateSource(entry.workout, body, entryVars, errorString)) {
            errorString = QString("entry %1: %2").arg(entry.date, errorString);
            return false;
        }
        for (const QString &v : entryVars) {
            if (!mergedVars.contains(v)) {
                mergedVars << v;
            }
        }
        qint64 day = dateToDays2000(entry.date);
        gates << QString("if (SUUNTO_DAYS_AFTER_1_1_2000 == %1) {\n%2\n}\n").arg(day).arg(indent(body));
    }

    QList<qint64> days;
    for (const TrainingPlanEntry &entry : sorted) {
        days << dateToDays2000(entry.date);
    }
    std::sort(days.begin(), days.end());

    QStringList countdown;
    for (auto it = days.rbegin(); it != days.rend(); ++it) {
        countdown << QString("if (SUUNTO_DAYS_AFTER_1_1_2000 < %1) {\n"
                              "\tRESULT = %1 - SUUNTO_DAYS_AFTER_1_1_2000;\n"
                              "\tprefix = \"In\";\n"
                              "\tpostfix = \"d\";\n"
                              "}\n").arg(*it);
    }

    QString done = QString("if (SUUNTO_DAYS_AFTER_1_1_2000 > %1) {\n"
                            "\tRESULT = 0;\n"
                            "\tprefix = \"Done\";\n"
                            "}\n").arg(days.last());

    source = gates.join("") + countdown.join("") + done;
    ownVars = mergedVars;
    return true;
}

QList<TrainingPlanPack> TrainingPlanScheduler::packPlan(const TrainingPlan &plan, QString &errorString)
{
    QList<TrainingPlanPack> result;

    QList<TrainingPlanEntry> entries = plan.entries;
    if (entries.isEmpty()) {
        errorString = "plan has no entries";
        return result;
    }
    std::sort(entries.begin(), entries.end(), [](const TrainingPlanEntry &a, const TrainingPlanEntry &b) {
        return a.date < b.date;
    });

    QString planName = plan.name.trimmed();
    if (planName.isEmpty()) {
        planName = "Program";
    }

    int i = 0;
    int part = 1;
    while (i < entries.size()) {
        int take = qMin(MAX_WORKOUTS_PER_APP_GUESS, entries.size() - i);
        QList<TrainingPlanEntry> pack = entries.mid(i, take);

        QString tag = (entries.size() > take || part > 1) ? QString(" %1").arg(part) : QString();
        QString name = planName.left(32 - tag.length()) + tag;

        QString source;
        QStringList ownVars;
        if (!buildAppSource(pack, source, ownVars, errorString)) {
            result.clear();
            return result;
        }

        TrainingPlanPack out;
        out.name = name;
        for (const TrainingPlanEntry &e : pack) {
            out.dates << e.date;
        }
        out.compileRequest = AppZoneGenerator::buildCompileRequest(source, ownVars);
        result.append(out);

        i += take;
        part++;
    }

    return result;
}
