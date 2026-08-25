/*
 * (C) Copyright 2026 Yann Chemin
 *
 * This file is part of Openambit.
 *
 * Openambit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 * Structured-workout data model and App Zone source generator ("Training Plan" App-Zone
 * workaround, Phase 2 of the openambit2 training-plans plan). Ported from the sibling
 * sommet project's tools/workout.py (https://github.com/skinnie/sommet), which reverse-
 * engineered the JSON step schema from openambitproject/openambit#257 and independently
 * confirmed it against wanarun.net and Suunto's own tutorial. This is a from-scratch C++
 * reimplementation of that documented schema/algorithm, not a copy of their Python source.
 *
 * Deliberately NOT ported: workout.py's compile_source() (the live network call to the
 * community App-Zone compiler). Per this project's plan, the GUI hands the generated source
 * to the user to paste into the compiler website themselves rather than calling it on their
 * behalf automatically - see AppZoneGenerator::buildCompileRequest()'s own doc comment.
 *
 * IMPORTANT / unverified: whether a freshly-compiled App Zone app installed this way actually
 * *executes* its logic on real Ambit3-family firmware (as opposed to merely being accepted and
 * displayed) has not been confirmed on real hardware by this project. Treat any output of this
 * generator as experimental until that is checked against a real watch.
 */
#ifndef WORKOUT_H
#define WORKOUT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QList>

class WorkoutStepDuration
{
public:
    explicit WorkoutStepDuration(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString DURATION_NAME;
    static const QString VALUE;

    QString durationName = "time"; // time/distance/ascent/lap/energy/hr_below/hr_above
    int value = 0;                 // seconds/meters/meters/(unused)/kcal/bpm/bpm
};

class WorkoutStepTarget
{
public:
    explicit WorkoutStepTarget(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString TARGET_NAME;
    static const QString VALUE_RANGE;
    static const QString MIN;
    static const QString MAX;

    QString targetName = "none"; // none/hr/pace/speed/vertical_speed/power/cadence
    int rangeMin = 0;
    int rangeMax = 0;
};

class WorkoutStepNotify
{
public:
    explicit WorkoutStepNotify(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString BEEP;
    static const QString LIGHT;

    bool beep = true;
    bool light = true;
};

class WorkoutStepType
{
public:
    explicit WorkoutStepType(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString TYPE_NAME;
    static const QString VALUE;

    QString typeName = "interval"; // warmup/interval/recovery/cooldown/repeatStart/repeatEnd
    int value = 0;                  // repeatStart's repeat count
};

class WorkoutStep
{
public:
    explicit WorkoutStep(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString TYPE;
    static const QString DURATION;
    static const QString TARGET;
    static const QString NOTIFY;
    static const QString TEXT;

    WorkoutStepType type;
    WorkoutStepDuration duration;
    WorkoutStepTarget target;
    WorkoutStepNotify notify;
    QString text; // optional label shown as this step's field prefix, see phaseLabel()
};

class Workout
{
public:
    explicit Workout(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString NAME;
    static const QString STEPS;

    QString name = "Workout";
    QList<WorkoutStep> steps;
};

/**
 * Pure text generator: turns a Workout into App Zone source, exactly mirroring sommet's
 * workout.py::generate_source() (see that file's own extensive derivation comments for why
 * each constraint below exists - they come from live behavior of the community compiler and
 * of App Zone's own bytecode limits, not from this project's own guesswork).
 */
class AppZoneGenerator
{
public:
    /**
     * Flattens repeatStart/repeatEnd blocks into a plain step list (repeat blocks are
     * unrolled at generation time - App Zone has no loop construct over data). Nested
     * repeats are not supported. Returns an empty list and sets \a errorString on a
     * malformed repeat structure.
     */
    static QList<WorkoutStep> expandSteps(const Workout &workout, QString &errorString);

    /**
     * Generates App Zone source for \a workout. Returns false (with \a errorString set) if
     * the workout has no real steps or its repeat structure is malformed. On success,
     * \a source holds the generated script body and \a ownVars the script's own top-level
     * variables (PHASE, START_COUNTER, ... - needed by buildCompileRequest()'s header block).
     */
    static bool generateSource(const Workout &workout, QString &source, QStringList &ownVars, QString &errorString);

    /**
     * Wraps \a source in the compiler's HEADER block (declaring \a ownVars), ready to hand
     * to a human to paste into the community App-Zone compiler website. This project does
     * not call that compiler automatically - see this class's own header comment for why.
     */
    static QString buildCompileRequest(const QString &source, const QStringList &ownVars);

    static const int LABEL_BUDGET = 6; // compiler limit: prefix + postfix per phase, in chars

private:
    static QString phaseCondition(const WorkoutStep &step);
    static QString phaseLabel(const WorkoutStep &step, int maxLen);
    static QString durationVar(const QString &durationName);
    static QString durationPostfix(const QString &durationName);
    static bool isCounterLessDuration(const QString &durationName);
    static QString targetVar(const QString &targetName);
};

#endif // WORKOUT_H
