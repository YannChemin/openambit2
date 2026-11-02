#include "geojsonexporter.h"
#include "activitycategory.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {

// Collects track points; the altitude of a point is the last one logged
// before it, since the watch records positions and altitudes separately.
struct TrackBuilder
{
    QJsonArray coords;
    QList<double> pendingLon, pendingLat;  // points logged before the first altitude
    double alt = 0.0;
    bool hasAlt = false;

    void setAltitude(double meters)
    {
        alt = meters;
        if (!hasAlt) {
            // Give the points logged before the first altitude that altitude
            hasAlt = true;
            for (int i = 0; i < pendingLon.size(); i++)
                coords.append(QJsonArray{ pendingLon[i], pendingLat[i], alt });
            pendingLon.clear();
            pendingLat.clear();
        }
    }

    // lat/lon are in 1e-7 degrees
    void append(int32_t lat, int32_t lon)
    {
        if (lat == 0 && lon == 0)
            return;
        if (hasAlt) {
            coords.append(QJsonArray{ lon / 1e7, lat / 1e7, alt });
        } else {
            pendingLon << lon / 1e7;
            pendingLat << lat / 1e7;
        }
    }

    // Track without any altitude: 2D coordinates
    void finish()
    {
        for (int i = 0; i < pendingLon.size(); i++)
            coords.append(QJsonArray{ pendingLon[i], pendingLat[i] });
        pendingLon.clear();
        pendingLat.clear();
    }
};

} // namespace

QString GeoJSONExporter::exportTrack(LogEntry *entry)
{
    if (!entry || !entry->logEntry) return QString();

    ambit_log_entry_t *log = entry->logEntry;
    TrackBuilder track;
    int32_t lastLat = 0, lastLon = 0;
    bool hasLat = false, hasLon = false;
    // The pressure altitude (what the watch shows in Altitude mode) is preferred
    // over the GPS one, which is noisier, as soon as the log has one.
    bool hasBaroAlt = false;

    for (uint32_t i = 0; i < log->samples_count; i++) {
        ambit_log_sample_t &s = log->samples[i];
        switch (s.type) {
        case ambit_log_sample_type_gps_base:
            if (!hasBaroAlt && (s.u.gps_base.latitude != 0 || s.u.gps_base.longitude != 0))
                track.setAltitude(s.u.gps_base.altitude / 100.0);
            track.append(s.u.gps_base.latitude, s.u.gps_base.longitude);
            break;
        case ambit_log_sample_type_gps_small:
            track.append(s.u.gps_small.latitude, s.u.gps_small.longitude);
            break;
        case ambit_log_sample_type_gps_tiny:
            track.append(s.u.gps_tiny.latitude, s.u.gps_tiny.longitude);
            break;
        case ambit_log_sample_type_position:
            track.append(s.u.position.latitude, s.u.position.longitude);
            break;
        case ambit_log_sample_type_periodic:
            hasLat = false; hasLon = false;
            for (uint8_t j = 0; j < s.u.periodic.value_count; j++) {
                auto &v = s.u.periodic.values[j];
                if (v.type == ambit_log_sample_periodic_type_latitude) {
                    lastLat = v.u.latitude; hasLat = true;
                } else if (v.type == ambit_log_sample_periodic_type_longitude) {
                    lastLon = v.u.longitude; hasLon = true;
                } else if (v.type == ambit_log_sample_periodic_type_altitude) {
                    hasBaroAlt = true;
                    track.setAltitude(v.u.altitude);
                } else if (v.type == ambit_log_sample_periodic_type_gpsaltitude && !hasBaroAlt) {
                    track.setAltitude(v.u.gpsaltitude / 100.0);
                }
            }
            if (hasLat && hasLon)
                track.append(lastLat, lastLon);
            break;
        default:
            break;
        }
    }
    track.finish();

    if (track.coords.isEmpty()) return QString();

    QJsonObject geometry;
    geometry["type"] = "LineString";
    geometry["coordinates"] = track.coords;

    QString activityName = log->header.activity_name ? QString::fromUtf8(log->header.activity_name) : QString();
    ActivityCategory::Kind kind = ActivityCategory::classify(log->header.activity_type, activityName);

    QJsonObject props;
    props["name"] = entry->time.toString("yyyy-MM-dd HH:mm");
    if (!activityName.isEmpty())
        props["activity"] = activityName;
    props["activity_type"] = (int)log->header.activity_type;
    props["category"] = ActivityCategory::key(kind);
    props["prefers_3d"] = ActivityCategory::prefers3D(kind);
    props["distance_m"] = (double)log->header.distance;
    props["duration_s"] = (double)(log->header.duration / 1000);
    props["altitude_min_m"] = (double)log->header.altitude_min;
    props["altitude_max_m"] = (double)log->header.altitude_max;

    QJsonObject feature;
    feature["type"] = "Feature";
    feature["geometry"] = geometry;
    feature["properties"] = props;

    QJsonObject fc;
    fc["type"] = "FeatureCollection";
    fc["features"] = QJsonArray{ feature };

    return QJsonDocument(fc).toJson(QJsonDocument::Compact);
}

QString GeoJSONExporter::exportWaypoints(LogEntry *entry)
{
    if (!entry || !entry->logEntry) return QString();

    // Waypoints come from routes/navigation data embedded in log entries.
    // The current libambit representation stores route navigation marks
    // separately (not in log samples). Return empty for now — waypoints
    // will be populated when route support is added.
    QJsonObject fc;
    fc["type"] = "FeatureCollection";
    fc["features"] = QJsonArray();
    return QJsonDocument(fc).toJson(QJsonDocument::Compact);
}
