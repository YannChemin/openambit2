#ifndef ACTIVITYCATEGORY_H
#define ACTIVITYCATEGORY_H

#include <QString>

// Groups activities by how they move through space, so that the map can pick
// a suitable view. "Air sports" is the FAI umbrella term for every flying
// discipline (free flight, gliding, microlights, powered aircraft, rotorcraft,
// ballooning, parachuting); "3D activities" covers them plus the vertical and
// underwater ones, whose height or depth matters as much as their ground track.
namespace ActivityCategory {

enum Kind {
    Ground,      // Running, cycling, hiking, skiing...
    Water,       // Sailing, kayaking, swimming...
    Air,         // Air sports: the track is flown, its altitude is above the terrain
    Vertical,    // Climbing, mountaineering, via ferrata
    Underwater   // Scuba and free diving
};

// Suunto/Movescount activity type of paragliding (0x5c), the only air sport
// the Ambit firmware knows. Other foot-launched wings reuse it.
constexpr int ACTIVITY_PARAGLIDING = 92;
// "Unspecified sport" (0x01): used for aircraft the firmware has no type for.
constexpr int ACTIVITY_UNSPECIFIED = 1;

// Classifies a log or sport mode from its activity type (log header
// activity_type or mode ActivityID) and its name. The name decides when the
// type alone cannot, e.g. a "Helicopter" mode recorded as "Unspecified sport".
Kind classify(int activityType, const QString &name);

// True for the kinds the map shows in 3D by default (air sports only, for now:
// a climbing route is too small for the terrain models that are available).
bool prefers3D(Kind kind);

// Stable, untranslated key ("ground", "air"...) for GeoJSON properties.
QString key(Kind kind);

// Translated, user-visible name of the kind.
QString displayName(Kind kind);

} // namespace ActivityCategory

#endif // ACTIVITYCATEGORY_H
