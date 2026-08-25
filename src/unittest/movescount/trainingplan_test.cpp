#include <movescount/trainingplan.h>
#include "doctest.h"

static QVariantMap simpleWorkout(const QString &name, int durationSeconds)
{
    QVariantMap type;
    type.insert(WorkoutStepType::TYPE_NAME, "interval");
    QVariantMap duration;
    duration.insert(WorkoutStepDuration::DURATION_NAME, "time");
    duration.insert(WorkoutStepDuration::VALUE, durationSeconds);
    QVariantMap step;
    step.insert(WorkoutStep::TYPE, type);
    step.insert(WorkoutStep::DURATION, duration);

    QVariantMap workout;
    workout.insert(Workout::NAME, name);
    workout.insert(Workout::STEPS, QVariantList{step});
    return workout;
}

static QVariantMap entry(const QString &date, const QVariantMap &workout)
{
    QVariantMap e;
    e.insert(TrainingPlanEntry::DATE, date);
    e.insert(TrainingPlanEntry::WORKOUT, workout);
    return e;
}

TEST_SUITE_BEGIN("TrainingPlan");

TEST_CASE("dateToDays2000 is 0-based on 2000-01-01") {
    CHECK(TrainingPlanScheduler::dateToDays2000("2000-01-01") == 0);
    CHECK(TrainingPlanScheduler::dateToDays2000("2000-01-02") == 1);
    CHECK(TrainingPlanScheduler::dateToDays2000("1999-12-31") == -1);
}

TEST_CASE("TrainingPlan/TrainingPlanEntry round-trip through toMap") {
    QVariantMap map;
    map.insert(TrainingPlan::NAME, "Marathon prep");
    map.insert(TrainingPlan::ENTRIES, QVariantList{
        entry("2026-08-14", simpleWorkout("5x3min intervals", 180)),
        entry("2026-08-16", simpleWorkout("Long run", 3600)),
    });

    TrainingPlan plan(map);
    CHECK(plan.name == "Marathon prep");
    REQUIRE(plan.entries.size() == 2);
    CHECK(plan.entries[0].date == "2026-08-14");
    CHECK(plan.entries[0].workout.name == "5x3min intervals");
    CHECK(plan.entries[0].workout.steps[0].duration.value == 180);

    TrainingPlan roundTripped(plan.toMap());
    CHECK(roundTripped.name == plan.name);
    REQUIRE(roundTripped.entries.size() == 2);
    CHECK(roundTripped.entries[1].date == "2026-08-16");
    CHECK(roundTripped.entries[1].workout.name == "Long run");
    CHECK(roundTripped.entries[1].workout.steps[0].duration.value == 3600);
}

TEST_CASE("buildAppSource gates each entry on its own SUUNTO_DAYS_AFTER_1_1_2000 and adds a countdown") {
    QList<TrainingPlanEntry> entries;
    entries.append(TrainingPlanEntry(entry("2026-08-16", simpleWorkout("Long run", 3600))));
    entries.append(TrainingPlanEntry(entry("2026-08-14", simpleWorkout("Short run", 600))));

    QString source, error;
    QStringList ownVars;
    REQUIRE(TrainingPlanScheduler::buildAppSource(entries, source, ownVars, error) == true);
    CHECK(error.isEmpty());

    qint64 day14 = TrainingPlanScheduler::dateToDays2000("2026-08-14");
    qint64 day16 = TrainingPlanScheduler::dateToDays2000("2026-08-16");

    CHECK(source.contains(QString("if (SUUNTO_DAYS_AFTER_1_1_2000 == %1)").arg(day14)));
    CHECK(source.contains(QString("if (SUUNTO_DAYS_AFTER_1_1_2000 == %1)").arg(day16)));
    // Countdown branches are emitted latest-date-first, so the earliest upcoming date wins.
    CHECK(source.contains(QString("if (SUUNTO_DAYS_AFTER_1_1_2000 < %1)").arg(day16)));
    CHECK(source.contains(QString("if (SUUNTO_DAYS_AFTER_1_1_2000 < %1)").arg(day14)));
    CHECK(source.contains(QString("if (SUUNTO_DAYS_AFTER_1_1_2000 > %1)").arg(day16)));
    CHECK(source.contains("prefix = \"Done\";"));

    // Own-vars from each entry are merged/deduped.
    CHECK(ownVars.contains("PHASE"));
    CHECK(ownVars.contains("START_COUNTER"));
    CHECK(ownVars.contains("CURRENT_LAP_NUMBER"));
    CHECK(ownVars.count("PHASE") == 1);
}

TEST_CASE("buildAppSource fails cleanly on an empty entry list") {
    QString source, error;
    QStringList ownVars;
    CHECK(TrainingPlanScheduler::buildAppSource(QList<TrainingPlanEntry>(), source, ownVars, error) == false);
    CHECK(!error.isEmpty());
}

TEST_CASE("packPlan packs at most MAX_WORKOUTS_PER_APP_GUESS entries per app, in date order") {
    QVariantMap map;
    map.insert(TrainingPlan::NAME, "Marathon prep");
    map.insert(TrainingPlan::ENTRIES, QVariantList{
        entry("2026-08-20", simpleWorkout("C", 600)),
        entry("2026-08-14", simpleWorkout("A", 600)),
        entry("2026-08-16", simpleWorkout("B", 600)),
    });
    TrainingPlan plan(map);

    QString error;
    QList<TrainingPlanPack> packs = TrainingPlanScheduler::packPlan(plan, error);
    CHECK(error.isEmpty());
    REQUIRE(packs.size() == 2); // 3 entries, guess=2 per app -> packs of 2 and 1

    CHECK(packs[0].dates == QStringList{"2026-08-14", "2026-08-16"});
    CHECK(packs[0].name == "Marathon prep 1");
    CHECK(packs[1].dates == QStringList{"2026-08-20"});
    CHECK(packs[1].name == "Marathon prep 2");

    for (const TrainingPlanPack &pack : packs) {
        CHECK(pack.compileRequest.startsWith("/***HEADER***/"));
        CHECK(pack.compileRequest.contains("SUUNTO_DAYS_AFTER_1_1_2000"));
    }
}

TEST_CASE("packPlan gives a single untagged app name when everything fits in one pack") {
    QVariantMap map;
    map.insert(TrainingPlan::NAME, "Short plan");
    map.insert(TrainingPlan::ENTRIES, QVariantList{
        entry("2026-08-14", simpleWorkout("A", 600)),
    });
    TrainingPlan plan(map);

    QString error;
    QList<TrainingPlanPack> packs = TrainingPlanScheduler::packPlan(plan, error);
    REQUIRE(packs.size() == 1);
    CHECK(packs[0].name == "Short plan");
}

TEST_CASE("packPlan fails cleanly on a plan with no entries") {
    TrainingPlan plan;
    QString error;
    QList<TrainingPlanPack> packs = TrainingPlanScheduler::packPlan(plan, error);
    CHECK(packs.isEmpty());
    CHECK(!error.isEmpty());
}

TEST_SUITE_END();
