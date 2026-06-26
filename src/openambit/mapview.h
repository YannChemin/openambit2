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

signals:
    // Emitted whenever the JS side reports a view change (lat, lon, zoom)
    void viewChanged(double lat, double lon, int zoom);

private slots:
    void onTitleChanged(const QString &title);

private:
    void runJS(const QString &js);
    bool m_ready = false;
    bool m_pendingInitialView = false;
    bool m_pendingHasFiles = false;
};

#endif // MAPVIEW_H
