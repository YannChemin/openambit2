#include "geojsonexporter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static void appendCoord(QJsonArray &coords, int32_t lat, int32_t lon)
{
    // lat/lon are in 1e-7 degrees
    coords.append(QJsonArray{ lon / 1e7, lat / 1e7 });
}

QString GeoJSONExporter::exportTrack(LogEntry *entry)
{
    if (!entry || !entry->logEntry) return QString();

    ambit_log_entry_t *log = entry->logEntry;
    QJsonArray coords;
    int32_t lastLat = 0, lastLon = 0;
    bool hasLat = false, hasLon = false;

    for (uint32_t i = 0; i < log->samples_count; i++) {
        ambit_log_sample_t &s = log->samples[i];
        switch (s.type) {
        case ambit_log_sample_type_gps_base:
            if (s.u.gps_base.latitude != 0 || s.u.gps_base.longitude != 0)
                appendCoord(coords, s.u.gps_base.latitude, s.u.gps_base.longitude);
            break;
        case ambit_log_sample_type_gps_small:
            if (s.u.gps_small.latitude != 0 || s.u.gps_small.longitude != 0)
                appendCoord(coords, s.u.gps_small.latitude, s.u.gps_small.longitude);
            break;
        case ambit_log_sample_type_gps_tiny:
            if (s.u.gps_tiny.latitude != 0 || s.u.gps_tiny.longitude != 0)
                appendCoord(coords, s.u.gps_tiny.latitude, s.u.gps_tiny.longitude);
            break;
        case ambit_log_sample_type_position:
            if (s.u.position.latitude != 0 || s.u.position.longitude != 0)
                appendCoord(coords, s.u.position.latitude, s.u.position.longitude);
            break;
        case ambit_log_sample_type_periodic:
            hasLat = false; hasLon = false;
            for (uint8_t j = 0; j < s.u.periodic.value_count; j++) {
                auto &v = s.u.periodic.values[j];
                if (v.type == ambit_log_sample_periodic_type_latitude) {
                    lastLat = v.u.latitude; hasLat = true;
                } else if (v.type == ambit_log_sample_periodic_type_longitude) {
                    lastLon = v.u.longitude; hasLon = true;
                }
            }
            if (hasLat && hasLon && (lastLat != 0 || lastLon != 0))
                appendCoord(coords, lastLat, lastLon);
            break;
        default:
            break;
        }
    }

    if (coords.isEmpty()) return QString();

    QJsonObject geometry;
    geometry["type"] = "LineString";
    geometry["coordinates"] = coords;

    QJsonObject props;
    props["name"] = entry->time.toString("yyyy-MM-dd HH:mm");
    if (log->header.activity_name)
        props["activity"] = QString::fromUtf8(log->header.activity_name);
    props["distance_m"] = (double)log->header.distance;
    props["duration_s"] = (double)(log->header.duration / 1000);

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
