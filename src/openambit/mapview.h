#ifndef MAPVIEW_H
#define MAPVIEW_H

#ifdef HAVE_WEBENGINE
#include <QWebEngineView>
typedef QWebEngineView MapViewBase;
#else
#include <QLabel>
typedef QLabel MapViewBase;
#endif

#include <QWidget>
#include <QSettings>

class CopernicusDem;
struct DemGrid;

// Map of the logged tracks, in 2D (Leaflet, OpenStreetMap) or 3D (CesiumJS:
// satellite imagery draped over a terrain model, flown tracks drawn at their
// recorded altitude). See map.html for the imagery and terrain sources and
// their fallbacks.
class MapView : public MapViewBase
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

    // Load the HTML map; call once after construction
    void loadMap();

    // Restore the last saved view from QSettings, or fall back to Plumergat if no
    // GeoJSON files exist yet, or fit all layers if files exist.
    void applyInitialView(bool hasGeojsonFiles);

public slots:
    void loadGeoJSON(const QString &id, const QString &geojson, const QString &color = QString());
    void showLayer(const QString &id, bool visible);
    void clearLayer(const QString &id);
    void fitAll();
    // "2d" or "3d"
    void setMode(const QString &mode);

signals:
    // Emitted whenever the JS side reports a view change (lat, lon, zoom)
    void viewChanged(double lat, double lon, int zoom);
    // Emitted when the map switches between "2d" and "3d"
    void modeChanged(const QString &mode);

private:
    void onPageMessage(const QString &json);
    void sendStrings();
    void sendDem(const DemGrid &grid);
    void runJS(const QString &js);
    bool m_ready = false;
    bool m_pendingInitialView = false;
    bool m_pendingHasFiles = false;
    CopernicusDem *m_dem = nullptr;
};

#endif // MAPVIEW_H
