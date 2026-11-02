#include "sportmodestorage.h"
#include "activitycategory.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QString SportModeStorage::defaultPath()
{
    return QString(getenv("HOME")) + "/.openambit/sport_modes.json";
}

bool SportModeStorage::save(const QList<QVariantMap> &modes, const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QJsonArray arr;
    for (const QVariantMap &m : modes)
        arr.append(QJsonObject::fromVariantMap(m));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(QJsonDocument(arr).toJson());
    return true;
}

QList<QVariantMap> SportModeStorage::load(const QString &path)
{
    QList<QVariantMap> result;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return factoryDefaults();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
        return factoryDefaults();
    for (const QJsonValue &v : doc.array()) {
        if (v.isObject())
            result.append(v.toObject().toVariantMap());
    }
    return result.isEmpty() ? factoryDefaults() : result;
}

// AltiBaroMode: 0=FusedAlti, 1=Altitude(baro), 2=Barometer
// GPSInterval:  0=Best(~1s), 1=1s, 2=2s, 5=5s, 10=10s, 15=15s, 30=30s, 60=60s
static QVariantMap makeMode(const QString &name, int activityId, int customModeId,
                             int gpsInterval, int recordingInterval,
                             int altiBaroMode,
                             bool hrBelt, bool footPod, bool bikePod,
                             bool cadencePod, bool accelerometer,
                             int autolap = 0)
{
    QVariantMap m;
    m["Name"]              = name;
    m["ActivityID"]        = activityId;
    m["CustomModeID"]      = customModeId;
    m["GPSInterval"]       = gpsInterval;
    m["RecordingInterval"] = recordingInterval;
    m["AltiBaroMode"]      = altiBaroMode;
    m["UseHRBelt"]         = hrBelt;
    m["UseFootPOD"]        = footPod;
    m["UseBikePOD"]        = bikePod;
    m["UseCadencePOD"]     = cadencePod;
    m["UseAccelerometer"]  = accelerometer;
    m["UsePowerPOD"]       = false;
    m["UseAutolap"]        = (autolap > 0);
    m["AutolapDistance"]   = autolap;
    m["HRLimitHigh"]       = 180;
    m["HRLimitLow"]        = 120;
    m["UseHRLimits"]       = false;
    m["AutoPauseSpeed"]    = 0.0;
    m["AutoScrolling"]     = 0;
    m["BacklightMode"]     = 0;
    m["DisplayIsNegative"] = 0;
    m["ShowNavigationSelection"] = 0;
    m["UseIntervals"]      = false;
    m["IntervalRepetitions"] = 0;
    m["Interval1Time"]     = 0;
    m["Interval2Time"]     = 0;
    m["Interval1Distance"] = 0;
    m["Interval2Distance"] = 0;
    m["Displays"]          = QVariantList();
    m["DisplayedRuleIDs"]  = QVariantList();
    m["LoggedRuleIDs"]     = QVariantList();
    return m;
}

QList<QVariantMap> SportModeStorage::factoryDefaults()
{
    // ActivityID values follow Suunto/Movescount conventions for the Ambit series.
    // GPSInterval 1=1s, 2=2s, 5=5s, 10=10s, 30=30s, 60=60s; AltiBaroMode 0=FusedAlti, 1=Altitude, 2=Baro
    //                        name            actId  modeId  gpsInt recInt alti  hrBelt foot  bike  cad   accel autolap(m)
    return {
        makeMode("Running",         10,  1,    1,    1,    0,  true,  false, false, false, false, 1000),
        makeMode("Trail Running",   13,  2,    1,    1,    1,  true,  false, false, false, false, 5000),
        makeMode("Cycling",         11,  3,    1,    1,    0,  true,  false, true,  true,  false, 5000),
        makeMode("Mountain Biking", 3,   4,    1,    1,    1,  true,  false, true,  false, false, 0),
        makeMode("Hiking",          8,   5,    5,    5,    1,  false, false, false, false, false, 0),
        makeMode("Trekking",        9,   6,    10,   10,   1,  false, false, false, false, false, 0),
        makeMode("Nordic Walking",  14,  7,    5,    5,    0,  true,  true,  false, false, false, 0),
        makeMode("Rock Climbing",   25,  8,    30,   15,   1,  true,  false, false, false, false, 0),
        makeMode("Ski (Downhill)",  26,  9,    2,    2,    1,  true,  false, false, false, false, 0),
        makeMode("Ski Touring",     27,  10,   5,    5,    1,  true,  false, false, false, false, 0),
        makeMode("Snowshoeing",     28,  11,   10,   10,   1,  false, false, false, false, false, 0),
        makeMode("XC Skiing",       29,  12,   2,    2,    0,  true,  false, false, false, false, 0),
        makeMode("Kayaking",        18,  13,   2,    2,    0,  false, false, false, false, false, 0),
        makeMode("Rowing",          17,  14,   2,    2,    0,  true,  false, false, false, false, 500),
        makeMode("Surfing",         21,  15,   1,    1,    2,  false, false, false, false, false, 0),
        makeMode("Windsurfing",     20,  16,   1,    1,    2,  false, false, false, false, false, 0),
        makeMode("Sailing",         19,  17,   5,    5,    2,  false, false, false, false, false, 0),
        makeMode("Horse Riding",    23,  18,   5,    5,    0,  false, false, false, false, false, 0),
        makeMode("Other",           0,   19,   30,   30,   0,  false, false, false, false, false, 0),
    };
}

QList<QVariantMap> SportModeStorage::airSportPresets()
{
    // Flying needs a dense track: 1 s GPS fixes and recording catch thermal
    // circles, and the pressure altimeter is locked in "Altitude" mode (1)
    // because FusedAlti would read climbs and descents as weather changes.
    // Paragliding (92) is the only flying activity the firmware knows; the
    // other wings reuse it, aircraft use "Unspecified sport" (1) and are
    // recognised as air sports by their name. Names fit the watch's 15
    // characters; translations must too, and are cut otherwise.
    struct Preset {
        const char *name;
        int activityId;
        int gpsInterval;
        int recordingInterval;
        bool hrBelt;
    };
    static const Preset presets[] = {
        //                                                        name              activity                                   gps rec hrBelt
        { QT_TRANSLATE_NOOP("SportModeStorage", "Paragliding"),    ActivityCategory::ACTIVITY_PARAGLIDING, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Hang Gliding"),   ActivityCategory::ACTIVITY_PARAGLIDING, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Speed Flying"),   ActivityCategory::ACTIVITY_PARAGLIDING, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Paramotor"),      ActivityCategory::ACTIVITY_PARAGLIDING, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Gliding"),        ActivityCategory::ACTIVITY_PARAGLIDING, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Microlight ULM"), ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Gyrocopter"),     ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Powered Flight"), ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Helicopter"),     ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Balloon"),        ActivityCategory::ACTIVITY_UNSPECIFIED, 5, 5, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Skydiving"),      ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
        { QT_TRANSLATE_NOOP("SportModeStorage", "Wingsuit"),       ActivityCategory::ACTIVITY_UNSPECIFIED, 1, 1, false },
    };

    QList<QVariantMap> result;
    for (const Preset &p : presets) {
        QString name = QCoreApplication::translate("SportModeStorage", p.name).left(15);
        result << makeMode(name, p.activityId, 0, p.gpsInterval, p.recordingInterval,
                           1, p.hrBelt, false, false, false, false, 0);
    }
    return result;
}
