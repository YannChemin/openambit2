#ifndef GEOJSONEXPORTER_H
#define GEOJSONEXPORTER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <movescount/logentry.h>

class GeoJSONExporter
{
public:
    // Returns a GeoJSON FeatureCollection with a single LineString for the GPS track,
    // or empty string if no GPS data found.
    static QString exportTrack(LogEntry *entry);

    // Returns a GeoJSON FeatureCollection where each waypoint/POI is a Point Feature.
    static QString exportWaypoints(LogEntry *entry);

    // Returns true if any .geojson files exist in ~/.openambit/geojson/
    static bool hasAnyFiles()
    {
        QString base = QString(getenv("HOME")) + "/.openambit/geojson";
        QStringList subdirs = { "tracks", "waypoints" };
        for (const QString &sub : subdirs) {
            QDir d(base + "/" + sub);
            if (!d.entryList({"*.geojson"}, QDir::Files).isEmpty())
                return true;
        }
        return false;
    }

    // Return all .geojson file paths grouped by subdirectory name
    static QMap<QString, QStringList> scanFiles()
    {
        QMap<QString, QStringList> result;
        QString base = QString(getenv("HOME")) + "/.openambit/geojson";
        for (const QString &sub : QStringList{"tracks", "waypoints"}) {
            QDir d(base + "/" + sub);
            QStringList files = d.entryList({"*.geojson"}, QDir::Files, QDir::Time);
            if (!files.isEmpty()) {
                QStringList paths;
                for (const QString &f : files)
                    paths << d.absoluteFilePath(f);
                result[sub] = paths;
            }
        }
        return result;
    }
};

#endif // GEOJSONEXPORTER_H
