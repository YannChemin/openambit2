#include <movescount/workout.h>
#include "doctest.h"

static QVariantMap step(const QString &typeName, int typeValue = 0)
{
    QVariantMap type;
    type.insert(WorkoutStepType::TYPE_NAME, typeName);
    if (typeValue) {
        type.insert(WorkoutStepType::VALUE, typeValue);
    }
    QVariantMap step;
    step.insert(WorkoutStep::TYPE, type);
    return step;
}

static void addDuration(QVariantMap &step, const QString &durationName, int value)
{
    QVariantMap duration;
    duration.insert(WorkoutStepDuration::DURATION_NAME, durationName);
    duration.insert(WorkoutStepDuration::VALUE, value);
    step.insert(WorkoutStep::DURATION, duration);
}

static void addTarget(QVariantMap &step, const QString &targetName, int min = 0, int max = 0)
{
    QVariantMap target;
    target.insert(WorkoutStepTarget::TARGET_NAME, targetName);
    if (targetName != "none") {
        QVariantMap range;
        range.insert(WorkoutStepTarget::MIN, min);
        range.insert(WorkoutStepTarget::MAX, max);
        target.insert(WorkoutStepTarget::VALUE_RANGE, range);
    }
    step.insert(WorkoutStep::TARGET, target);
}

TEST_SUITE_BEGIN("Workout");

TEST_CASE("parses the schema from workout.h's own docstring example") {
    QVariantMap warmup = step("warmup");
    addDuration(warmup, "time", 600);
    addTarget(warmup, "hr", 120, 140);

    QVariantMap repeatStart = step("repeatStart", 5);
    QVariantMap interval = step("interval");
    addDuration(interval, "time", 180);
    addTarget(interval, "power", 200, 220);
    QVariantMap recovery = step("recovery");
    addDuration(recovery, "time", 90);
    addTarget(recovery, "none");
    QVariantMap repeatEnd = step("repeatEnd");

    QVariantMap cooldown = step("cooldown");
    addDuration(cooldown, "time", 300);
    addTarget(cooldown, "none");

    QVariantMap map;
    map.insert(Workout::NAME, "5x3min power intervals");
    map.insert(Workout::STEPS, QVariantList{warmup, repeatStart, interval, recovery, repeatEnd, cooldown});

    Workout workout(map);
    CHECK(workout.name == "5x3min power intervals");
    REQUIRE(workout.steps.size() == 6);
    CHECK(workout.steps[0].type.typeName == "warmup");
    CHECK(workout.steps[0].duration.value == 600);
    CHECK(workout.steps[0].target.targetName == "hr");
    CHECK(workout.steps[0].target.rangeMin == 120);
    CHECK(workout.steps[1].type.typeName == "repeatStart");
    CHECK(workout.steps[1].type.value == 5);
}

TEST_CASE("expandSteps unrolls a repeat block") {
    QVariantMap interval = step("interval");
    addDuration(interval, "time", 60);
    QVariantMap recovery = step("recovery");
    addDuration(recovery, "time", 30);

    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{step("repeatStart", 3), interval, recovery, step("repeatEnd")});
    Workout workout(map);

    QString error;
    QList<WorkoutStep> expanded = AppZoneGenerator::expandSteps(workout, error);
    CHECK(error.isEmpty());
    REQUIRE(expanded.size() == 6);
    CHECK(expanded[0].type.typeName == "interval");
    CHECK(expanded[1].type.typeName == "recovery");
    CHECK(expanded[4].type.typeName == "interval");
    CHECK(expanded[5].type.typeName == "recovery");
}

TEST_CASE("expandSteps rejects nested repeat blocks") {
    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{step("repeatStart", 2), step("repeatStart", 2), step("repeatEnd"), step("repeatEnd")});
    Workout workout(map);

    QString error;
    QList<WorkoutStep> expanded = AppZoneGenerator::expandSteps(workout, error);
    CHECK(expanded.isEmpty());
    CHECK(!error.isEmpty());
}

TEST_CASE("expandSteps rejects an unterminated repeat block") {
    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{step("repeatStart", 2), step("interval")});
    Workout workout(map);

    QString error;
    QList<WorkoutStep> expanded = AppZoneGenerator::expandSteps(workout, error);
    CHECK(expanded.isEmpty());
    CHECK(!error.isEmpty());
}

TEST_CASE("generateSource fails cleanly on a workout with no real steps") {
    Workout workout;
    QString source, error;
    QStringList ownVars;
    CHECK(AppZoneGenerator::generateSource(workout, source, ownVars, error) == false);
    CHECK(!error.isEmpty());
}

TEST_CASE("generateSource produces well-formed phase-chain source for a simple two-step workout") {
    QVariantMap warmup = step("warmup");
    addDuration(warmup, "time", 60);
    QVariantMap cooldown = step("cooldown");
    addDuration(cooldown, "time", 30);

    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{warmup, cooldown});
    Workout workout(map);

    QString source, error;
    QStringList ownVars;
    REQUIRE(AppZoneGenerator::generateSource(workout, source, ownVars, error) == true);
    CHECK(error.isEmpty());

    CHECK(source.contains("PHASE = 1;"));
    CHECK(source.contains("PHASE == 1 && (SUUNTO_DURATION - START_COUNTER) >= 60"));
    CHECK(source.contains("PHASE == 2 && (SUUNTO_DURATION - START_COUNTER) >= 30"));
    CHECK(source.contains("prefix = \"Warm\";"));
    CHECK(source.contains("prefix = \"Done\";"));
    CHECK(source.contains("Suunto.alarmBeep();"));
    CHECK(source.contains("Suunto.light();"));

    CHECK(ownVars == QStringList{"PHASE", "START_COUNTER", "CURRENT_LAP_NUMBER"});
    CHECK(!source.contains("OUT_OF_RANGE")); // no step has a real target, so no target-watch block
}

TEST_CASE("generateSource adds an OUT_OF_RANGE target-watch block only when a step has a real target") {
    QVariantMap interval = step("interval");
    addDuration(interval, "time", 60);
    addTarget(interval, "hr", 120, 140);

    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{interval});
    Workout workout(map);

    QString source, error;
    QStringList ownVars;
    REQUIRE(AppZoneGenerator::generateSource(workout, source, ownVars, error) == true);

    CHECK(source.contains("SUUNTO_HR < 120 || SUUNTO_HR > 140"));
    CHECK(ownVars.contains("OUT_OF_RANGE"));
}

TEST_CASE("phase labels never contain a digit, matching the live compiler's constraint") {
    QVariantMap interval = step("interval");
    addDuration(interval, "time", 60);
    interval[WorkoutStep::TEXT] = "Push 120";

    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{interval});
    Workout workout(map);

    QString source, error;
    QStringList ownVars;
    REQUIRE(AppZoneGenerator::generateSource(workout, source, ownVars, error) == true);

    CHECK(source.contains("prefix = \"Push\";"));
    CHECK(!source.contains("120"));
}

TEST_CASE("a lap-based duration uses SUUNTO_LAP_NUMBER, not a counter") {
    QVariantMap lapStep = step("interval");
    addDuration(lapStep, "lap", 0);

    QVariantMap map;
    map.insert(Workout::STEPS, QVariantList{lapStep});
    Workout workout(map);

    QString source, error;
    QStringList ownVars;
    REQUIRE(AppZoneGenerator::generateSource(workout, source, ownVars, error) == true);

    CHECK(source.contains("SUUNTO_LAP_NUMBER > CURRENT_LAP_NUMBER"));
    CHECK(source.contains("RESULT = SUUNTO_LAP_NUMBER - CURRENT_LAP_NUMBER;"));
}

TEST_CASE("buildCompileRequest wraps source in a HEADER block declaring own vars") {
    QString request = AppZoneGenerator::buildCompileRequest("PHASE = 1;\n", QStringList{"PHASE", "START_COUNTER"});
    CHECK(request.startsWith("/***HEADER***/\nPHASE = 0\nSTART_COUNTER = 0\n/***ENDHEADER***/\n"));
    CHECK(request.endsWith("PHASE = 1;\n"));
}

TEST_SUITE_END();
