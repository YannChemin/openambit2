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

TEST_CASE("dateToDays2000 matches real firmware: 2000-01-02 is day 0, not 2000-01-01") {
    // Hardware-verified by sommet (commit b343442): the firmware counts days STRICTLY AFTER
    // 2000-01-01, so 2000-01-02 = 0 (not 2000-01-01 = 0, which was this port's original,
    // never-hardware-checked assumption).
    CHECK(TrainingPlanScheduler::dateToDays2000("2000-01-01") == -1);
    CHECK(TrainingPlanScheduler::dateToDays2000("2000-01-02") == 0);
    CHECK(TrainingPlanScheduler::dateToDays2000("2000-01-03") == 1);
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

TEST_CASE("buildNativeProgram sums expanded step durations/distances per entry") {
    QVariantMap map;
    map.insert(TrainingPlan::NAME, "Marathon prep");
    // Entry 1: a repeat block (3 x 90s) + a plain 600s step = 3*90+600 = 870s -> 15 min (ceil).
    QVariantMap repeatStart, interval, plainStep, repeatEnd;
    {
        QVariantMap type; type.insert(WorkoutStepType::TYPE_NAME, "repeatStart"); type.insert(WorkoutStepType::VALUE, 3);
        repeatStart.insert(WorkoutStep::TYPE, type);
    }
    {
        QVariantMap type; type.insert(WorkoutStepType::TYPE_NAME, "interval");
        QVariantMap duration; duration.insert(WorkoutStepDuration::DURATION_NAME, "time"); duration.insert(WorkoutStepDuration::VALUE, 90);
        interval.insert(WorkoutStep::TYPE, type);
        interval.insert(WorkoutStep::DURATION, duration);
    }
    {
        QVariantMap type; type.insert(WorkoutStepType::TYPE_NAME, "repeatEnd");
        repeatEnd.insert(WorkoutStep::TYPE, type);
    }
    {
        QVariantMap type; type.insert(WorkoutStepType::TYPE_NAME, "cooldown");
        QVariantMap duration; duration.insert(WorkoutStepDuration::DURATION_NAME, "time"); duration.insert(WorkoutStepDuration::VALUE, 600);
        plainStep.insert(WorkoutStep::TYPE, type);
        plainStep.insert(WorkoutStep::DURATION, duration);
    }
    QVariantMap workout1;
    workout1.insert(Workout::NAME, "Running");
    workout1.insert(Workout::STEPS, QVariantList{repeatStart, interval, repeatEnd, plainStep});

    QVariantMap e1;
    e1.insert(TrainingPlanEntry::DATE, "2026-08-14");
    e1.insert(TrainingPlanEntry::WORKOUT, workout1);

    QVariantMap e2;
    e2.insert(TrainingPlanEntry::DATE, "2026-08-20");
    e2.insert(TrainingPlanEntry::WORKOUT, simpleWorkout("Cycling", 3600)); // 3600s -> 60 min

    map.insert(TrainingPlan::ENTRIES, QVariantList{e1, e2});
    TrainingPlan plan(map);

    ambit_training_program_t program;
    QString error;
    REQUIRE(TrainingPlanScheduler::buildNativeProgram(plan, program, error) == true);
    CHECK(error.isEmpty());

    CHECK(program.base_year == 2026);
    CHECK(program.base_month == 8);
    CHECK(program.base_day == 14);
    REQUIRE(program.items_count == 2);

    CHECK(program.items[0].day_offset == 0);
    CHECK(std::string(program.items[0].name) == "Running");
    CHECK(program.items[0].duration_minutes == 15); // ceil(870/60)
    CHECK(program.items[0].activity_id == 3); // resolved from "Running"

    CHECK(program.items[1].day_offset == 6); // 2026-08-20 is 6 days after 2026-08-14
    CHECK(std::string(program.items[1].name) == "Cycling");
    CHECK(program.items[1].duration_minutes == 60);
    CHECK(program.items[1].activity_id == 4); // resolved from "Cycling"
}

TEST_CASE("buildNativeProgram falls back to activityId 3 for an unrecognized workout name") {
    QVariantMap map;
    map.insert(TrainingPlan::ENTRIES, QVariantList{entry("2026-08-14", simpleWorkout("Something Exotic", 60))});
    TrainingPlan plan(map);

    ambit_training_program_t program;
    QString error;
    REQUIRE(TrainingPlanScheduler::buildNativeProgram(plan, program, error) == true);
    REQUIRE(program.items_count == 1);
    CHECK(program.items[0].activity_id == 3);
}

TEST_CASE("buildNativeProgram fails cleanly on a plan with no entries") {
    TrainingPlan plan;
    ambit_training_program_t program;
    QString error;
    CHECK(TrainingPlanScheduler::buildNativeProgram(plan, program, error) == false);
    CHECK(!error.isEmpty());
}

TEST_CASE("buildNativeProgram fails cleanly on more entries than the firmware supports") {
    QVariantMap map;
    QVariantList entries;
    for (int i = 0; i < TRAINING_PROGRAM_MAX_ITEMS + 1; i++) {
        entries.append(entry(QString("2026-01-%1").arg(i % 28 + 1, 2, 10, QChar('0')), simpleWorkout("Running", 60)));
    }
    map.insert(TrainingPlan::ENTRIES, entries);
    TrainingPlan plan(map);

    ambit_training_program_t program;
    QString error;
    CHECK(TrainingPlanScheduler::buildNativeProgram(plan, program, error) == false);
    CHECK(!error.isEmpty());
}

TEST_SUITE_END();
