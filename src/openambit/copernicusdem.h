#ifndef COPERNICUSDEM_H
#define COPERNICUSDEM_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;

// Regular latitude/longitude elevation grid. Coordinates are those of pixel
// centres; row 0 is the northernmost row.
struct DemGrid
{
    double lon0 = 0.0;    // Longitude of the centre of column 0
    double lat0 = 0.0;    // Latitude of the centre of row 0
    double dx = 0.0;      // Column spacing, degrees east
    double dy = 0.0;      // Row spacing, degrees south (positive)
    int width = 0;
    int height = 0;
    QVector<float> heights;  // Metres, row-major

    bool isValid() const { return width > 0 && height > 0 && heights.size() == width * height; }
    // Bilinear height at a point, or `outside` when the point is off the grid.
    float sample(double lon, double lat, float outside = 0.0f) const;
};

// Minimal GeoTIFF reader for single-band elevation rasters such as the
// Copernicus DEM tiles: little-endian classic TIFF, tiled or striped, float32
// or int16, uncompressed or Deflate, with predictor 1, 2 or 3, in geographic
// coordinates (ModelPixelScale + ModelTiepoint). Anything else is rejected
// with an error rather than read wrongly.
namespace GeoTiff {
bool decode(const QByteArray &tiff, DemGrid &out, QString &error);
}

// Fetches the Copernicus GLO-90 DEM (3 arc-second, about 90 m, global, free,
// no account) for a bounding box from its public AWS Open Data bucket, the
// same source as the r.in.dem GRASS addon. Tiles are cached under
// ~/.cache/Openambit/dem; ocean tiles do not exist and read as sea level.
//
// This is the terrain fallback of the 3D map, used when the browser cannot
// reach the terrain tiles directly: the bucket sends no CORS headers, so the
// page cannot read it itself.
class CopernicusDem : public QObject
{
    Q_OBJECT
public:
    explicit CopernicusDem(QObject *parent = nullptr);

    // Largest number of 1-degree tiles a single request may need.
    static constexpr int MAX_TILES = 25;
    // Largest side of the returned grid, in pixels.
    static constexpr int MAX_GRID_SIZE = 1024;

    // Name of the tile holding the 1-degree cell whose south-west corner is
    // (lat, lon), e.g. "Copernicus_DSM_COG_30_N47_00_W003_00_DEM".
    static QString tileName(int lat, int lon);

    // Mosaics `tiles` (missing ones read as 0 m) over the box, at the native
    // resolution or coarser so that neither side exceeds `maxSize` pixels.
    static DemGrid mosaic(const QList<DemGrid> &tiles, double west, double south,
                          double east, double north, int maxSize = MAX_GRID_SIZE);

public slots:
    // Starts fetching the box; emits ready() or failed() once done. A new
    // request replaces the one in progress.
    void request(double west, double south, double east, double north);

signals:
    void ready(const DemGrid &grid);
    void failed(const QString &reason);
    void progress(int done, int total);

private:
    void fetchNext();
    void finishTile(QNetworkReply *reply);
    void finish();
    QString cachePath(const QString &name) const;

    QNetworkAccessManager *m_nam;
    double m_west = 0, m_south = 0, m_east = 0, m_north = 0;
    QStringList m_queue;
    QList<DemGrid> m_tiles;
    int m_total = 0;
    int m_networkErrors = 0;
    int m_generation = 0;
};

#endif // COPERNICUSDEM_H
