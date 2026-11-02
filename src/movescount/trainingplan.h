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
 * Training Plan scheduler (Phase 3 of the openambit2 training-plans plan): schedules real
 * workouts (workout.h's Workout) on real calendar dates, as date-gated Suunto Apps. Ported
 * from the sibling sommet project's tools/training_plan.py - see that file's own extensive
 * derivation comments (App Zone's SUUNTO_DAYS_AFTER_1_1_2000 built-in, the label-length and
 * binary-size limits found by live compiler probing) for why this design looks the way it
 * does. From-scratch C++ reimplementation of the documented algorithm, not a copy of their
 * Python source.
 *
 * IMPORTANT SCOPE NOTE: sommet's own packer verifies each pack actually compiles (backing
 * off on the live compiler's BINARY_TOO_LARGE error) because it calls the compiler itself.
 * This project deliberately does not call the compiler automatically (see AppZoneGenerator's
 * own header comment) - so packPlan() below can only use the same static pack-size guess
 * sommet uses as its *first* guess, with no ability to verify or back off. If a pack is too
 * large, the compiler website will report COMPILATION_FAILED/BINARY_TOO_LARGE when the user
 * pastes it in, and they'll need to manually split that entry into its own plan.
 *
 * TWO WAYS TO SCHEDULE A PLAN, pick per use case - see buildNativeProgram() vs. packPlan():
 *
 * 1. NATIVE (buildNativeProgram(), training_program.h) - hardware-confirmed by sommet
 *    2026-09-03, no App Zone bytecode or external compiler at all. Writes a flat list of
 *    planned-move records (name/duration/distance/activityId/date) that the watch's own
 *    firmware reads directly and shows on its native "Today" screen. Prefer this for the
 *    common case of "remind me I have a run planned today" - it is simpler, needs no manual
 *    compile step, and libambit_training_program_write() can write it straight to the watch.
 *    Limitation: one flat record per entry, no multi-step interval structure, no live
 *    target-range alerting, no countdown display of its own, max 60 entries.
 *
 * 2. APP ZONE (packPlan()) - the only way to schedule a REAL multi-phase interval workout
 *    (warmup/interval/recovery/cooldown with target ranges), but needs the manual compiler
 *    hand-off below, and whether a compiled app's logic actually executes on real firmware
 *    (as opposed to being accepted/displayed) is still unverified - see AppZoneGenerator's
 *    own header comment. Also still missing: wiring an installed app into a sport mode's
 *    display (sommet's tools/workout_install.py) needs a second binary format - the real
 *    Apps flash-region directory (already ported, see apps_directory.h) - PLUS the
 *    CustomModes-side RULES/DISP_FIELD_SHORTCUT wiring to actually attach it to a display,
 *    which is not built here; this file only builds and saves the packed App Zone sources/
 *    compiled-app files, see TrainingPlanStorage for where those are kept.
 */
#ifndef TRAININGPLAN_H
#define TRAININGPLAN_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

#include "workout.h"
#include "training_program.h"

class TrainingPlanEntry
{
public:
    explicit TrainingPlanEntry(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString DATE;
    static const QString WORKOUT;

    QString date; // ISO YYYY-MM-DD
    Workout workout;
};

class TrainingPlan
{
public:
    explicit TrainingPlan(const QVariantMap &map = QVariantMap());
    QVariantMap toMap() const;

    static const QString NAME;
    static const QString ENTRIES;

    QString name = "Training Plan";
    QList<TrainingPlanEntry> entries;
};

/**
 * One app's worth of packed, date-gated workout(s): the App Zone source ready to hand to
 * the compiler, plus the metadata needed to save/track the result once compiled.
 */
class TrainingPlanPack
{
public:
    QString name;             // on-watch app name, e.g. "Marathon prep 1"
    QStringList dates;        // ISO dates this pack covers, sorted
    QString compileRequest;   // HEADER-wrapped source, ready to paste into the compiler
};

class TrainingPlanScheduler
{
public:
    // The community compiler's own hard cap on compiled binary size was found (by sommet's
    // live probing) to reject anything past ~4KB; two 12-phase interval workouts compiled to
    // 3332 bytes, three did not. Used only as a first-guess pack size - see this file's own
    // header comment for why there's no verify-and-back-off loop here.
    static const int MAX_WORKOUTS_PER_APP_GUESS = 2;

    /** ISO date -> SUUNTO_DAYS_AFTER_1_1_2000 (0-based on 2000-01-01, a Saturday). */
    static qint64 dateToDays2000(const QString &isoDate);

    /**
     * Builds one app's source: each entry's workout body gated on its planned day, plus a
     * countdown ("In Nd") for every other day and a "Done" state after the last planned date.
     * Returns false (with \a errorString set) if any entry's workout fails to generate, or if
     * \a entries is empty.
     */
    static bool buildAppSource(const QList<TrainingPlanEntry> &entries, QString &source, QStringList &ownVars, QString &errorString);

    /**
     * Packs \a plan's entries into groups of up to MAX_WORKOUTS_PER_APP_GUESS (greedy,
     * date-sorted, no live-compiler verification - see this file's header comment), and
     * generates each pack's App Zone source. Returns an empty list (with \a errorString set)
     * if the plan has no entries or any entry's workout fails to generate.
     */
    static QList<TrainingPlanPack> packPlan(const TrainingPlan &plan, QString &errorString);

    /**
     * Builds a native ambit_training_program_t from \a plan - see this file's own header
     * comment for how this differs from packPlan()/the App Zone path. Each entry becomes one
     * flat item: duration_minutes is the sum of its workout's expanded "time" steps (repeats
     * unrolled, seconds rounded up to whole minutes), distance the sum of its "distance"
     * steps, intensity a fixed default (3 - this project's Workout model has no per-entry
     * intensity field), and activityId resolved from the workout's name against a small table
     * of common sports (falling back to 3/Running) - deliberately simple, unlike sommet's own
     * resolution against a live watch's real CustomModes names, which needs a connected device
     * this function doesn't take. base_date is the earliest entry's calendar date; every
     * item's day_offset is its own date minus that base date, in days.
     * \return false (with \a errorString set) if the plan has no entries, if it has more than
     *         TRAINING_PROGRAM_MAX_ITEMS entries, or if any entry's workout fails to expand
     */
    static bool buildNativeProgram(const TrainingPlan &plan, ambit_training_program_t &out, QString &errorString);

private:
    static QString indent(const QString &source, const QString &prefix = "\t");
    static int resolveActivityId(const QString &workoutName);
};

#endif // TRAININGPLAN_H
