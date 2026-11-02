#include "activitycategory.h"

#include <QCoreApplication>
#include <QStringList>

namespace ActivityCategory {

// Lower-cases and strips accents, so that "Hélicoptère" matches "helicopt".
static QString fold(const QString &s)
{
    QString d = s.toLower().normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(d.size());
    for (const QChar c : d) {
        if (c.category() != QChar::Mark_NonSpacing)
            out.append(c);
    }
    return out;
}

static bool containsAny(const QString &folded, const QStringList &words)
{
    for (const QString &w : words) {
        if (folded.contains(w))
            return true;
    }
    return false;
}

Kind classify(int activityType, const QString &name)
{
    // Mode names are typed by the user, often in their own language, so the
    // keywords cover the usual English, French, German, Italian and Spanish
    // spellings. They are matched as substrings of the accent-free name.
    static const QStringList airWords = {
        "paraglid", "parapent", "gleitschirm", "parapendio",
        "hang glid", "hangegleit", "hanggleit", "deltaplan", "drachen",
        "speed fly", "speedfly", "speed rid", "speedrid",
        "paramot", "ppg",
        "glider", "gliding", "sailplane", "soaring", "planeur", "vol a voile",
        "segelflug", "volo a vela", "vuelo a vela", "velero",
        "ulm", "microlight", "ultralight", "ultraleger", "ultraleicht",
        "gyrocopt", "autogir", "autogyr", "tragschrauber",
        "helicopt", "helico", "heli ", "hubschrauber", "elicottero",
        "powered flight", "aeroplane", "airplane", "aircraft", "avion",
        "flugzeug", "aereo", "aviation", "flight", "flug", "vol moteur",
        "vol libre", "free flight",
        "balloon", "montgolf", "ballon", "mongolfiera", "globo",
        "skydiv", "parachut", "chute libre", "fallschirm", "paracaid", "paracadut",
        "wingsuit", "base jump",
    };
    static const QStringList verticalWords = {
        "climb", "escalad", "klettern", "arrampic", "bloc", "boulder",
        "via ferrata", "klettersteig", "alpinis", "mountaineer", "bergsteig",
    };
    static const QStringList underwaterWords = {
        "scuba", "dive", "diving", "plongee", "apnea", "apnee", "tauchen", "buceo", "immersion",
    };
    static const QStringList waterWords = {
        "swim", "natation", "kayak", "canoe", "row", "aviron", "sail", "voile",
        "surf", "paddl", "stand up",
    };

    // The trailing space lets short words such as "heli " match at the end.
    const QString n = fold(name) + QLatin1Char(' ');

    switch (activityType) {
    case 0x5c:                    // Paragliding
        return Air;
    case 0x10:                    // Climbing
    case 0x4a:                    // Mountaineering
        return Vertical;
    case 0x33: case 0x34: case 0x35: // Scuba diving, free diving, snorkeling
        return Underwater;
    default:
        break;
    }

    // Names are checked from the most to the least specific kind: "vol a voile"
    // (gliding) must win over "voile" (sailing).
    if (containsAny(n, airWords))
        return Air;
    if (containsAny(n, underwaterWords))
        return Underwater;
    if (containsAny(n, verticalWords))
        return Vertical;

    switch (activityType) {
    case 0x06: case 0x0d: case 0x0e: case 0x0f: case 0x36: case 0x48:
    case 0x53: case 0x56: case 0x59: case 0x5b:
        return Water;
    default:
        break;
    }
    if (containsAny(n, waterWords))
        return Water;
    return Ground;
}

bool prefers3D(Kind kind)
{
    return kind == Air;
}

QString key(Kind kind)
{
    switch (kind) {
    case Water:      return "water";
    case Air:        return "air";
    case Vertical:   return "vertical";
    case Underwater: return "underwater";
    case Ground:
    default:         return "ground";
    }
}

QString displayName(Kind kind)
{
    switch (kind) {
    case Water:      return QCoreApplication::translate("ActivityCategory", "Water sports");
    case Air:        return QCoreApplication::translate("ActivityCategory", "Air sports");
    case Vertical:   return QCoreApplication::translate("ActivityCategory", "Climbing & mountaineering");
    case Underwater: return QCoreApplication::translate("ActivityCategory", "Underwater");
    case Ground:
    default:         return QCoreApplication::translate("ActivityCategory", "Ground sports");
    }
}

} // namespace ActivityCategory
