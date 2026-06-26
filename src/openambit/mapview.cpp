#include "mapview.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>

#ifdef HAVE_WEBENGINE
#include <QWebEngineSettings>
#endif

// Default home: Plumergat, Bretagne, France
static constexpr double HOME_LAT  = 47.817;
static constexpr double HOME_LON  = -2.817;
static constexpr int    HOME_ZOOM = 10;

MapView::MapView(QWidget *parent)
    : MapViewBase(parent)
{
#ifdef HAVE_WEBENGINE
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    connect(this, &QWebEngineView::titleChanged, this, &MapView::onTitleChanged);
#else
    setText("Map requires Qt5WebEngine (not installed)");
    setAlignment(Qt::AlignCenter);
    setStyleSheet("color: #6e7681; background: #1e1e2e;");
#endif
}

void MapView::loadMap()
{
#ifdef HAVE_WEBENGINE
    QString htmlPath;
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/map.html",
        QCoreApplication::applicationDirPath() + "/../share/openambit/map.html",
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../src/openambit/map.html")
    };
    for (const QString &c : candidates) {
        if (QFile::exists(c)) { htmlPath = c; break; }
    }
    if (htmlPath.isEmpty()) {
        setHtml("<html><body style='background:#1e1e2e;color:#6e7681;font-family:sans-serif;"
                "display:flex;align-items:center;justify-content:center;height:100%'>"
                "<p>map.html not found</p></body></html>");
        return;
    }
    connect(this, &QWebEngineView::loadFinished, this, [this](bool ok) {
        m_ready = ok;
        if (ok && m_pendingInitialView) {
            m_pendingInitialView = false;
            applyInitialView(m_pendingHasFiles);
        }
    });
    load(QUrl::fromLocalFile(htmlPath));
#endif
}

void MapView::applyInitialView(bool hasGeojsonFiles)
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) {
        // Defer until loadFinished
        m_pendingInitialView = true;
        m_pendingHasFiles = hasGeojsonFiles;
        return;
    }

    if (hasGeojsonFiles) {
        // Try to restore the last saved view from settings
        QSettings s;
        s.beginGroup("mapView");
        double lat  = s.value("lat",  HOME_LAT).toDouble();
        double lon  = s.value("lon",  HOME_LON).toDouble();
        int    zoom = s.value("zoom", HOME_ZOOM).toInt();
        s.endGroup();
        runJS(QString("restoreView(%1, %2, %3);")
              .arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6).arg(zoom));
    } else {
        // First run — go home
        runJS(QString("setHomeView(%1, %2, %3);")
              .arg(HOME_LAT, 0, 'f', 6).arg(HOME_LON, 0, 'f', 6).arg(HOME_ZOOM));
    }
#else
    Q_UNUSED(hasGeojsonFiles)
#endif
}

void MapView::onTitleChanged(const QString &title)
{
    // The JS side encodes view state as: __mapview__lat,lon,zoom
    if (!title.startsWith("__mapview__"))
        return;
    QStringList parts = title.mid(11).split(',');
    if (parts.size() != 3)
        return;
    bool ok1, ok2, ok3;
    double lat  = parts[0].toDouble(&ok1);
    double lon  = parts[1].toDouble(&ok2);
    int    zoom = parts[2].toInt(&ok3);
    if (!ok1 || !ok2 || !ok3)
        return;

    // Persist for next session
    QSettings s;
    s.beginGroup("mapView");
    s.setValue("lat",  lat);
    s.setValue("lon",  lon);
    s.setValue("zoom", zoom);
    s.endGroup();

    emit viewChanged(lat, lon, zoom);
}

void MapView::loadGeoJSON(const QString &id, const QString &geojson, const QString &color)
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) return;
    QString safeJson = QString(geojson).replace("\\", "\\\\").replace("'", "\\'").replace("\n", "");
    QString col = color.isEmpty() ? "#3584e4" : color;
    runJS(QString("loadGeoJSON('%1', '%2', '%3');").arg(id, safeJson, col));
#else
    Q_UNUSED(id) Q_UNUSED(geojson) Q_UNUSED(color)
#endif
}

void MapView::showLayer(const QString &id, bool visible)
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) return;
    runJS(QString("showLayer('%1', %2);").arg(id, visible ? "true" : "false"));
#else
    Q_UNUSED(id) Q_UNUSED(visible)
#endif
}

void MapView::clearLayer(const QString &id)
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) return;
    runJS(QString("clearLayer('%1');").arg(id));
#else
    Q_UNUSED(id)
#endif
}

void MapView::fitAll()
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) return;
    runJS("fitAll();");
#endif
}

void MapView::runJS(const QString &js)
{
#ifdef HAVE_WEBENGINE
    page()->runJavaScript(js);
#else
    Q_UNUSED(js)
#endif
}
