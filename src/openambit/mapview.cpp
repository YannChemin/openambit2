#include "mapview.h"
#include "copernicusdem.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

#ifdef HAVE_WEBENGINE
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

namespace {

// The page talks to C++ through console messages carrying this prefix and a
// JSON object: unlike the window title, every message is delivered.
const QString BRIDGE_PREFIX = QStringLiteral("__openambit__");

class MapPage : public QWebEnginePage
{
public:
    MapPage(std::function<void(const QString &)> handler, QObject *parent)
        : QWebEnginePage(parent), m_handler(std::move(handler)) {}

protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override
    {
        if (message.startsWith(BRIDGE_PREFIX)) {
            m_handler(message.mid(BRIDGE_PREFIX.length()));
            return;
        }
        QWebEnginePage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
    }

private:
    std::function<void(const QString &)> m_handler;
};

} // namespace
#endif

// Default home: Plumergat, Bretagne, France
static constexpr double HOME_LAT  = 47.817;
static constexpr double HOME_LON  = -2.817;
static constexpr int    HOME_ZOOM = 10;

MapView::MapView(QWidget *parent)
    : MapViewBase(parent)
{
#ifdef HAVE_WEBENGINE
    setPage(new MapPage([this](const QString &json) { onPageMessage(json); }, this));
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    // Lets tile servers answer in the interface language where they can
    page()->profile()->setHttpAcceptLanguage(QLocale().uiLanguages().join(','));

    m_dem = new CopernicusDem(this);
    connect(m_dem, &CopernicusDem::ready, this, &MapView::sendDem);
    connect(m_dem, &CopernicusDem::failed, this, [this](const QString &reason) {
        QString safe = QString(reason).replace("\\", "\\\\").replace("'", "\\'");
        runJS(QString("fallbackDemFailed('%1');").arg(safe));
    });
    connect(m_dem, &CopernicusDem::progress, this, [this](int done, int total) {
        runJS(QString("fallbackDemProgress(%1, %2);").arg(done).arg(total));
    });
#else
    setText(tr("Map requires Qt5WebEngine (not installed)"));
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
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../src/openambit/map.html"),
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../src/openambit/map.html")
    };
    for (const QString &c : candidates) {
        if (QFile::exists(c)) { htmlPath = c; break; }
    }
    if (htmlPath.isEmpty()) {
        setHtml("<html><body style='background:#1e1e2e;color:#6e7681;font-family:sans-serif;"
                "display:flex;align-items:center;justify-content:center;height:100%'>"
                "<p>" + tr("map.html not found").toHtmlEscaped() + "</p></body></html>");
        return;
    }
    connect(this, &QWebEngineView::loadFinished, this, [this](bool ok) {
        m_ready = ok;
        if (!ok)
            return;
        sendStrings();
        if (m_pendingInitialView) {
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
    if (QSettings().value("mapView/mode").toString() == "3d")
        setMode("3d");
#else
    Q_UNUSED(hasGeojsonFiles)
#endif
}

void MapView::onPageMessage(const QString &json)
{
    QJsonObject msg = QJsonDocument::fromJson(json.toUtf8()).object();
    const QString type = msg.value("type").toString();

    if (type == "view") {
        double lat = msg.value("lat").toDouble();
        double lon = msg.value("lon").toDouble();
        int zoom = msg.value("zoom").toInt();

        // Persist for next session
        QSettings s;
        s.beginGroup("mapView");
        s.setValue("lat",  lat);
        s.setValue("lon",  lon);
        s.setValue("zoom", zoom);
        s.endGroup();
        emit viewChanged(lat, lon, zoom);
    } else if (type == "mode") {
        const QString mode = msg.value("mode").toString();
        if (msg.value("user").toBool())
            QSettings().setValue("mapView/mode", mode);
        emit modeChanged(mode);
    } else if (type == "demRequest" && m_dem) {
        m_dem->request(msg.value("west").toDouble(), msg.value("south").toDouble(),
                       msg.value("east").toDouble(), msg.value("north").toDouble());
    }
}

void MapView::sendStrings()
{
    // Everything the page shows, translated here so that one catalogue covers it
    QJsonObject s;
    s["view2d"]          = tr("2D");
    s["view3d"]          = tr("3D");
    s["view2dTip"]       = tr("Flat map (OpenStreetMap)");
    s["view3dTip"]       = tr("Relief with satellite imagery");
    s["loading3d"]       = tr("Loading the 3D globe...");
    s["noWebGL"]         = tr("3D view unavailable: this system has no WebGL support.");
    s["no3dLibrary"]     = tr("3D view unavailable: the CesiumJS library could not be loaded (offline?).");
    s["imagery"]         = tr("Imagery");
    s["terrain"]         = tr("Terrain");
    s["terrainTiles"]    = tr("Terrain Tiles (AWS Open Data)");
    s["terrainCopernicus"] = tr("Copernicus GLO-90 (around the tracks)");
    s["terrainCopernicusLoading"] = tr("Downloading Copernicus GLO-90 elevation tiles: %1 of %2...");
    s["terrainNone"]     = tr("none (flat)");
    s["terrainNoTracks"] = tr("none: load a track to fetch the Copernicus elevation model around it");
    s["imageryEsri"]     = tr("Esri World Imagery");
    s["imageryS2"]       = tr("Sentinel-2 cloudless (EOX)");
    s["imageryOsm"]      = tr("OpenStreetMap");
    s["exaggeration"]    = tr("Relief exaggeration");
    s["curtain"]         = tr("Altitude curtain");
    s["distance"]        = tr("Distance");
    s["duration"]        = tr("Duration");
    s["altitude"]        = tr("Altitude");
    s["minutesUnit"]     = tr("min");
    s["categoryAir"]     = tr("Air sports");
    runJS(QString("setStrings(%1);").arg(QString::fromUtf8(QJsonDocument(s).toJson(QJsonDocument::Compact))));
}

void MapView::sendDem(const DemGrid &grid)
{
#ifdef HAVE_WEBENGINE
    // Heights travel as base64 little-endian float32, the layout of a JS Float32Array
    QByteArray raw(reinterpret_cast<const char *>(grid.heights.constData()),
                   grid.heights.size() * int(sizeof(float)));
    runJS(QString("setFallbackDem({lon0: %1, lat0: %2, dx: %3, dy: %4, width: %5, height: %6, data: '%7'});")
          .arg(grid.lon0, 0, 'f', 9).arg(grid.lat0, 0, 'f', 9)
          .arg(grid.dx, 0, 'g', 12).arg(grid.dy, 0, 'g', 12)
          .arg(grid.width).arg(grid.height)
          .arg(QString::fromLatin1(raw.toBase64())));
#else
    Q_UNUSED(grid)
#endif
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

void MapView::setMode(const QString &mode)
{
#ifdef HAVE_WEBENGINE
    if (!m_ready) return;
    runJS(QString("setMode('%1');").arg(mode == "3d" ? "3d" : "2d"));
#else
    Q_UNUSED(mode)
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
