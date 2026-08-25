#include "workout.h"

#include <QMap>
#include <QVariantList>

const QString WorkoutStepDuration::DURATION_NAME = "durationName";
const QString WorkoutStepDuration::VALUE = "value";

WorkoutStepDuration::WorkoutStepDuration(const QVariantMap &map)
{
    if (map.contains(DURATION_NAME)) {
        durationName = map[DURATION_NAME].toString();
    }
    value = map.value(VALUE, 0).toInt();
}

const QString WorkoutStepTarget::TARGET_NAME = "targetName";
const QString WorkoutStepTarget::VALUE_RANGE = "valueRange";
const QString WorkoutStepTarget::MIN = "min";
const QString WorkoutStepTarget::MAX = "max";

WorkoutStepTarget::WorkoutStepTarget(const QVariantMap &map)
{
    if (map.contains(TARGET_NAME)) {
        targetName = map[TARGET_NAME].toString();
    }
    QVariantMap range = map.value(VALUE_RANGE).toMap();
    rangeMin = range.value(MIN, 0).toInt();
    rangeMax = range.value(MAX, 0).toInt();
}

const QString WorkoutStepNotify::BEEP = "beep";
const QString WorkoutStepNotify::LIGHT = "light";

WorkoutStepNotify::WorkoutStepNotify(const QVariantMap &map)
{
    beep = map.value(BEEP, true).toBool();
    light = map.value(LIGHT, true).toBool();
}

const QString WorkoutStepType::TYPE_NAME = "typeName";
const QString WorkoutStepType::VALUE = "value";

WorkoutStepType::WorkoutStepType(const QVariantMap &map)
{
    if (map.contains(TYPE_NAME)) {
        typeName = map[TYPE_NAME].toString();
    }
    value = map.value(VALUE, 0).toInt();
}

const QString WorkoutStep::TYPE = "type";
const QString WorkoutStep::DURATION = "duration";
const QString WorkoutStep::TARGET = "target";
const QString WorkoutStep::NOTIFY = "notify";
const QString WorkoutStep::TEXT = "text";

WorkoutStep::WorkoutStep(const QVariantMap &map) :
    type(map.value(TYPE).toMap()),
    duration(map.value(DURATION).toMap()),
    target(map.value(TARGET).toMap()),
    notify(map.value(NOTIFY).toMap())
{
    text = map.value(TEXT).toString();
}

const QString Workout::NAME = "name";
const QString Workout::STEPS = "steps";

Workout::Workout(const QVariantMap &map)
{
    if (map.contains(NAME)) {
        name = map[NAME].toString();
    }
    for (const QVariant &stepVariant : map.value(STEPS).toList()) {
        steps.append(WorkoutStep(stepVariant.toMap()));
    }
}

/* --------------------------------------------------------------------------------------- */

// DURATION_VARS: which live App-Zone built-in each duration kind counts against.
static const QMap<QString, QString> &durationVars()
{
    static const QMap<QString, QString> vars = {
        {"time", "SUUNTO_DURATION"},
        {"distance", "SUUNTO_DISTANCE"},
        {"ascent", "SUUNTO_ASCENT"},
        {"energy", "SUUNTO_ENERGY"},
    };
    return vars;
}

// DURATION_POSTFIX: <=2 chars (compiler caps prefix+postfix at LABEL_BUDGET, postfix at 2).
static const QMap<QString, QString> &durationPostfixes()
{
    static const QMap<QString, QString> postfixes = {
        {"time", "s"}, {"distance", "m"}, {"ascent", "m"}, {"lap", ""},
        {"energy", "kc"}, {"hr_below", ""}, {"hr_above", ""},
    };
    return postfixes;
}

static const QMap<QString, QString> &targetVars()
{
    static const QMap<QString, QString> vars = {
        {"hr", "SUUNTO_HR"},
        {"pace", "SUUNTO_PACE"},
        {"speed", "SUUNTO_SPEED"},
        {"vertical_speed", "SUUNTO_VERTICAL_SPD"},
        {"power", "SUUNTO_BIKE_POWER"},
        {"cadence", "SUUNTO_CADENCE"},
    };
    return vars;
}

static const QMap<QString, QString> &typeLabels()
{
    static const QMap<QString, QString> labels = {
        {"warmup", "Warm"}, {"interval", "Fast"}, {"recovery", "Rec"}, {"cooldown", "Cool"},
    };
    return labels;
}

QString AppZoneGenerator::durationVar(const QString &durationName)
{
    return durationVars().value(durationName, "SUUNTO_DURATION");
}

QString AppZoneGenerator::durationPostfix(const QString &durationName)
{
    return durationPostfixes().value(durationName, "").left(2);
}

bool AppZoneGenerator::isCounterLessDuration(const QString &durationName)
{
    return durationName == "lap" || durationName == "hr_below" || durationName == "hr_above";
}

QString AppZoneGenerator::targetVar(const QString &targetName)
{
    return targetVars().value(targetName, QString());
}

QList<WorkoutStep> AppZoneGenerator::expandSteps(const Workout &workout, QString &errorString)
{
    QList<WorkoutStep> flat;
    const QList<WorkoutStep> &steps = workout.steps;
    int i = 0;

    while (i < steps.size()) {
        const WorkoutStep &step = steps[i];
        const QString &typeName = step.type.typeName;

        if (typeName == "repeatStart") {
            int count = step.type.value;
            QList<WorkoutStep> block;
            i++;
            while (i < steps.size() && steps[i].type.typeName != "repeatEnd") {
                if (steps[i].type.typeName == "repeatStart") {
                    errorString = "nested repeat blocks aren't supported";
                    return QList<WorkoutStep>();
                }
                block.append(steps[i]);
                i++;
            }
            if (i >= steps.size()) {
                errorString = "repeatStart without a matching repeatEnd";
                return QList<WorkoutStep>();
            }
            for (int r = 0; r < count; r++) {
                flat.append(block);
            }
        } else if (typeName == "repeatEnd") {
            errorString = "repeatEnd without a matching repeatStart";
            return QList<WorkoutStep>();
        } else {
            flat.append(step);
        }
        i++;
    }

    return flat;
}

QString AppZoneGenerator::phaseCondition(const WorkoutStep &step)
{
    const QString &kind = step.duration.durationName;
    if (kind == "lap") {
        return "SUUNTO_LAP_NUMBER > CURRENT_LAP_NUMBER";
    }
    if (kind == "hr_below") {
        return QString("SUUNTO_HR < %1").arg(step.duration.value);
    }
    if (kind == "hr_above") {
        return QString("SUUNTO_HR > %1").arg(step.duration.value);
    }
    return QString("(%1 - START_COUNTER) >= %2").arg(durationVar(kind)).arg(step.duration.value);
}

QString AppZoneGenerator::phaseLabel(const WorkoutStep &step, int maxLen)
{
    QString text = step.text.trimmed();
    if (!text.isEmpty()) {
        // Live compiler rejects any string literal containing a digit (confirmed against
        // the real service, see workout.h's provenance note) - strip digits, not just truncate.
        QString clean;
        for (QChar c : text) {
            if (!c.isDigit()) {
                clean.append(c);
            }
        }
        clean = clean.trimmed();
        if (!clean.isEmpty()) {
            return clean.left(maxLen);
        }
    }
    QString label = typeLabels().value(step.type.typeName);
    if (label.isEmpty()) {
        label = step.type.typeName.left(4);
        if (!label.isEmpty()) {
            label[0] = label[0].toUpper();
        }
    }
    return label.left(maxLen);
}

bool AppZoneGenerator::generateSource(const Workout &workout, QString &source, QStringList &ownVars, QString &errorString)
{
    QList<WorkoutStep> steps = expandSteps(workout, errorString);
    if (!errorString.isEmpty()) {
        return false;
    }
    if (steps.isEmpty()) {
        errorString = "workout has no real steps";
        return false;
    }

    QStringList lines;
    lines << "if (PHASE <= 0) {"
          << "\tPHASE = 1;"
          << QString("\tSTART_COUNTER = %1;").arg(durationVar(steps[0].duration.durationName))
          << "\tCURRENT_LAP_NUMBER = SUUNTO_LAP_NUMBER;"
          << "} else {";

    for (int idx = 0; idx < steps.size(); idx++) {
        const WorkoutStep &step = steps[idx];
        int phaseNum = idx + 1;
        QString keyword = (idx == 0) ? "\tif" : "\t} else if";
        lines << QString("%1 (PHASE == %2 && %3) {").arg(keyword).arg(phaseNum).arg(phaseCondition(step));
        lines << QString("\t\tPHASE = %1;").arg(phaseNum + 1);

        if (idx + 1 < steps.size()) {
            const QString &nextKind = steps[idx + 1].duration.durationName;
            if (!isCounterLessDuration(nextKind)) {
                lines << QString("\t\tSTART_COUNTER = %1;").arg(durationVar(nextKind));
            }
        }
        lines << "\t\tCURRENT_LAP_NUMBER = SUUNTO_LAP_NUMBER;";

        // notify() governs the alert fired on *entering* the next phase; defaults both on.
        WorkoutStepNotify notify = (idx + 1 < steps.size()) ? steps[idx + 1].notify : WorkoutStepNotify();
        if (notify.beep) {
            lines << "\t\tSuunto.alarmBeep();";
        }
        if (notify.light) {
            lines << "\t\tSuunto.light();";
        }
    }
    lines << "\t}" << "}" << "";

    lines << "if (PHASE < 1) {"
          << "\tprefix = \"\";"
          << "\tRESULT = 0;";
    for (int idx = 0; idx < steps.size(); idx++) {
        const WorkoutStep &step = steps[idx];
        int phaseNum = idx + 1;
        const QString &kind = step.duration.durationName;
        QString postfix = durationPostfix(kind);
        QString label = phaseLabel(step, LABEL_BUDGET - postfix.length());

        lines << QString("} else if (PHASE == %1) {").arg(phaseNum);
        lines << QString("\tprefix = \"%1\";").arg(label);
        lines << QString("\tpostfix = \"%1\";").arg(postfix);
        if (kind == "lap") {
            lines << "\tRESULT = SUUNTO_LAP_NUMBER - CURRENT_LAP_NUMBER;";
        } else if (kind == "hr_below" || kind == "hr_above") {
            lines << "\tRESULT = SUUNTO_HR;";
        } else {
            lines << QString("\tRESULT = %1 - (%2 - START_COUNTER);").arg(step.duration.value).arg(durationVar(kind));
        }
    }
    lines << "} else {"
          << "\tprefix = \"Done\";"
          << "\tpostfix = \"\";"
          << "\tRESULT = 0;"
          << "}";

    ownVars = QStringList{"PHASE", "START_COUNTER", "CURRENT_LAP_NUMBER"};

    bool hasTargets = false;
    for (const WorkoutStep &step : steps) {
        if (targetVars().contains(step.target.targetName)) {
            hasTargets = true;
            break;
        }
    }

    if (hasTargets) {
        lines << "" << "if (PHASE < 1) {" << "\tOUT_OF_RANGE = 0;";
        for (int idx = 0; idx < steps.size(); idx++) {
            const WorkoutStep &step = steps[idx];
            int phaseNum = idx + 1;
            lines << QString("} else if (PHASE == %1) {").arg(phaseNum);

            QString var = targetVar(step.target.targetName);
            if (var.isEmpty()) {
                lines << "\tOUT_OF_RANGE = 0;";
                continue;
            }
            lines << QString("\tif (%1 < %2 || %1 > %3) {").arg(var).arg(step.target.rangeMin).arg(step.target.rangeMax);
            lines << "\t\tif (OUT_OF_RANGE == 0) {";
            lines << "\t\t\tSuunto.alarmBeep();";
            lines << "\t\t\tOUT_OF_RANGE = 1;";
            lines << "\t\t}";
            lines << "\t} else {";
            lines << "\t\tOUT_OF_RANGE = 0;";
            lines << "\t}";
        }
        lines << "} else {" << "\tOUT_OF_RANGE = 0;" << "}";
        ownVars << "OUT_OF_RANGE";
    }

    source = lines.join("\n") + "\n";
    return true;
}

QString AppZoneGenerator::buildCompileRequest(const QString &source, const QStringList &ownVars)
{
    QStringList headerLines;
    for (const QString &v : ownVars) {
        headerLines << QString("%1 = 0").arg(v);
    }
    QString header = "/***HEADER***/\n" + headerLines.join("\n") + "\n/***ENDHEADER***/\n";
    return header + source;
}
