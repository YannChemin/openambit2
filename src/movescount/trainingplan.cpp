#include "trainingplan.h"

#include <QDate>
#include <QVariantList>
#include <QMap>
#include <algorithm>
#include <cstring>

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

// Hardware-verified by sommet on a real Ambit3 Sport (2026-09-02, commit b343442): the
// firmware's actual SUUNTO_DAYS_AFTER_1_1_2000 for a given calendar date is ONE LESS than
// (date - 2000-01-01).days - a date-gated plan entry dated "today" showed "In 1 d" with the
// watch clock confirmed correct. The built-in counts days STRICTLY AFTER 2000-01-01 (so
// 2000-01-02 = 0), not 0-based ON 2000-01-01 as this project's port originally assumed
// (that assumption came from weekday math alone, never date-verified against real hardware).
static const qint64 FIRMWARE_DAYS_OFFSET = 1;

qint64 TrainingPlanScheduler::dateToDays2000(const QString &isoDate)
{
    QDate d = QDate::fromString(isoDate, "yyyy-MM-dd");
    return EPOCH_2000.daysTo(d) - FIRMWARE_DAYS_OFFSET;
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

int TrainingPlanScheduler::resolveActivityId(const QString &workoutName)
{
    // Deliberately small and conservative: only ActivityID values this project has actually
    // confirmed against real Ambit3 hardware this session (openambit2's device_driver_ambit3.c
    // CustomModes work), not sommet's own live-CustomModes-name resolution (which needs a
    // connected watch this function doesn't take). Falls back to 3 (Running) otherwise.
    static const QMap<QString, int> table = {
        {"running", 3}, {"run", 3},
        {"cycling", 4}, {"bike", 4}, {"biking", 4},
        {"pool swimming", 6}, {"swimming", 6}, {"swim", 6},
        {"openwater swim", 83}, {"open water swim", 83},
        {"indoor training", 95},
        {"windsurfing", 20}, {"windsurf", 20},
    };
    return table.value(workoutName.trimmed().toLower(), 3);
}

bool TrainingPlanScheduler::buildNativeProgram(const TrainingPlan &plan, ambit_training_program_t &out, QString &errorString)
{
    QList<TrainingPlanEntry> entries = plan.entries;
    if (entries.isEmpty()) {
        errorString = "plan has no entries";
        return false;
    }
    if (entries.size() > TRAINING_PROGRAM_MAX_ITEMS) {
        errorString = QString("plan has %1 entries, the watch firmware supports at most %2")
                          .arg(entries.size()).arg(TRAINING_PROGRAM_MAX_ITEMS);
        return false;
    }
    std::sort(entries.begin(), entries.end(), [](const TrainingPlanEntry &a, const TrainingPlanEntry &b) {
        return a.date < b.date;
    });

    memset(&out, 0, sizeof(out));
    memcpy(out.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);

    QDate baseDate = QDate::fromString(entries.first().date, "yyyy-MM-dd");
    if (!baseDate.isValid()) {
        errorString = QString("entry has an invalid date: %1").arg(entries.first().date);
        return false;
    }
    out.base_year = (uint16_t)baseDate.year();
    out.base_month = (uint8_t)baseDate.month();
    out.base_day = (uint8_t)baseDate.day();

    out.items_count = (uint16_t)entries.size();
    for (int i = 0; i < entries.size(); i++) {
        const TrainingPlanEntry &entry = entries[i];
        QDate entryDate = QDate::fromString(entry.date, "yyyy-MM-dd");
        if (!entryDate.isValid()) {
            errorString = QString("entry has an invalid date: %1").arg(entry.date);
            return false;
        }

        QList<WorkoutStep> steps = AppZoneGenerator::expandSteps(entry.workout, errorString);
        if (!errorString.isEmpty()) {
            errorString = QString("entry %1: %2").arg(entry.date, errorString);
            return false;
        }

        int totalSeconds = 0;
        int totalDistance = 0;
        for (const WorkoutStep &step : steps) {
            if (step.duration.durationName == "time") {
                totalSeconds += step.duration.value;
            } else if (step.duration.durationName == "distance") {
                totalDistance += step.duration.value;
            }
        }

        qint64 dayOffset = baseDate.daysTo(entryDate);
        if (dayOffset > 255) {
            errorString = QString("plan spans more than 255 days (entry %1 is %2 days after the earliest entry) - "
                                   "the watch firmware's day_offset field is a single byte").arg(entry.date).arg(dayOffset);
            return false;
        }

        ambit_training_program_item_t &item = out.items[i];
        memset(&item, 0, sizeof(item));
        item.day_offset = (uint8_t)dayOffset;
        item.activity_id = (uint16_t)resolveActivityId(entry.workout.name);
        item.duration_minutes = (uint16_t)((totalSeconds + 59) / 60); // round up to whole minutes
        item.distance = (uint32_t)totalDistance;
        item.intensity = 3; // fixed default - see this method's own doc comment
        QByteArray nameUtf8 = (entry.workout.name.isEmpty() ? QStringLiteral("Workout") : entry.workout.name).toUtf8();
        strncpy(item.name, nameUtf8.constData(), TRAINING_PROGRAM_NAME_LEN);
        item.name[TRAINING_PROGRAM_NAME_LEN] = '\0';
    }

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
