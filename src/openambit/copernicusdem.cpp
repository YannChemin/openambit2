#include "copernicusdem.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QtEndian>

#include <cmath>
#include <cstring>

float DemGrid::sample(double lon, double lat, float outside) const
{
    if (!isValid())
        return outside;
    const double col = (lon - lon0) / dx;
    const double row = (lat0 - lat) / dy;
    if (col < -0.5 || row < -0.5 || col > width - 0.5 || row > height - 0.5)
        return outside;
    const double c = qBound(0.0, col, double(width - 1));
    const double r = qBound(0.0, row, double(height - 1));
    const int c0 = qMin(int(c), width - 1), r0 = qMin(int(r), height - 1);
    const int c1 = qMin(c0 + 1, width - 1), r1 = qMin(r0 + 1, height - 1);
    const double fc = c - c0, fr = r - r0;
    const float *h = heights.constData();
    const double top = h[r0 * width + c0] * (1 - fc) + h[r0 * width + c1] * fc;
    const double bottom = h[r1 * width + c0] * (1 - fc) + h[r1 * width + c1] * fc;
    return float(top * (1 - fr) + bottom * fr);
}

namespace GeoTiff {

namespace {

enum Tag {
    ImageWidth = 256,
    ImageLength = 257,
    BitsPerSample = 258,
    Compression = 259,
    StripOffsets = 273,
    SamplesPerPixel = 277,
    RowsPerStrip = 278,
    StripByteCounts = 279,
    Predictor = 317,
    TileWidth = 322,
    TileLength = 323,
    TileOffsets = 324,
    TileByteCounts = 325,
    SampleFormat = 339,
    ModelPixelScale = 33550,
    ModelTiepoint = 33922,
    GeoKeyDirectory = 34735,
};

constexpr int GT_MODEL_TYPE_KEY = 1024;
constexpr int GT_RASTER_TYPE_KEY = 1025;
constexpr int MODEL_TYPE_GEOGRAPHIC = 2;
constexpr int RASTER_PIXEL_IS_POINT = 2;

// Reads every numeric tag of the first IFD as doubles.
bool readTags(const QByteArray &tiff, QMap<int, QVector<double>> &tags, QString &error)
{
    const uchar *d = reinterpret_cast<const uchar *>(tiff.constData());
    const qint64 size = tiff.size();
    if (size < 8 || d[0] != 'I' || d[1] != 'I' || qFromLittleEndian<quint16>(d + 2) != 42) {
        error = QStringLiteral("not a little-endian classic TIFF");
        return false;
    }
    const quint32 ifd = qFromLittleEndian<quint32>(d + 4);
    if (ifd + 2 > size) {
        error = QStringLiteral("IFD offset out of range");
        return false;
    }
    const int count = qFromLittleEndian<quint16>(d + ifd);
    if (ifd + 2 + qint64(count) * 12 > size) {
        error = QStringLiteral("IFD truncated");
        return false;
    }
    for (int i = 0; i < count; i++) {
        const uchar *e = d + ifd + 2 + i * 12;
        const int tag = qFromLittleEndian<quint16>(e);
        const int type = qFromLittleEndian<quint16>(e + 2);
        const quint32 n = qFromLittleEndian<quint32>(e + 4);
        int typeSize;
        switch (type) {
        case 3: typeSize = 2; break;   // SHORT
        case 4: typeSize = 4; break;   // LONG
        case 12: typeSize = 8; break;  // DOUBLE
        default: continue;             // ASCII and others are not needed
        }
        const qint64 bytes = qint64(n) * typeSize;
        const uchar *v = e + 8;
        if (bytes > 4) {
            const quint32 off = qFromLittleEndian<quint32>(e + 8);
            if (off + bytes > size) {
                error = QStringLiteral("tag %1 out of range").arg(tag);
                return false;
            }
            v = d + off;
        }
        QVector<double> values(static_cast<int>(n));
        for (quint32 k = 0; k < n; k++) {
            switch (type) {
            case 3: values[k] = qFromLittleEndian<quint16>(v + 2 * k); break;
            case 4: values[k] = qFromLittleEndian<quint32>(v + 4 * k); break;
            default: {
                quint64 bits = qFromLittleEndian<quint64>(v + 8 * k);
                double x;
                std::memcpy(&x, &bits, 8);
                values[k] = x;
            }
            }
        }
        tags.insert(tag, values);
    }
    return true;
}

double tagValue(const QMap<int, QVector<double>> &tags, int tag, double fallback)
{
    const QVector<double> v = tags.value(tag);
    return v.isEmpty() ? fallback : v.first();
}

int geoKey(const QVector<double> &dir, int key, int fallback)
{
    // Header of 4 shorts, then entries of (key, location, count, value)
    for (int i = 4; i + 3 < dir.size(); i += 4) {
        if (int(dir[i]) == key && int(dir[i + 1]) == 0)
            return int(dir[i + 3]);
    }
    return fallback;
}

} // namespace

bool decode(const QByteArray &tiff, DemGrid &out, QString &error)
{
    QMap<int, QVector<double>> tags;
    if (!readTags(tiff, tags, error))
        return false;

    const int width = int(tagValue(tags, ImageWidth, 0));
    const int height = int(tagValue(tags, ImageLength, 0));
    const int bits = int(tagValue(tags, BitsPerSample, 0));
    const int compression = int(tagValue(tags, Compression, 1));
    const int predictor = int(tagValue(tags, Predictor, 1));
    const int format = int(tagValue(tags, SampleFormat, 1));
    const bool isFloat = (format == 3 && bits == 32);
    const bool isInt16 = ((format == 1 || format == 2) && bits == 16);

    if (width <= 0 || height <= 0) {
        error = QStringLiteral("missing image size");
        return false;
    }
    if (int(tagValue(tags, SamplesPerPixel, 1)) != 1) {
        error = QStringLiteral("only single-band rasters are supported");
        return false;
    }
    if (!isFloat && !isInt16) {
        error = QStringLiteral("unsupported sample type (%1 bits, format %2)").arg(bits).arg(format);
        return false;
    }
    if (compression != 1 && compression != 8 && compression != 32946) {
        error = QStringLiteral("unsupported compression %1").arg(compression);
        return false;
    }
    if (predictor != 1 && !(predictor == 2 && isInt16) && !(predictor == 3 && isFloat)) {
        error = QStringLiteral("unsupported predictor %1").arg(predictor);
        return false;
    }

    const QVector<double> scale = tags.value(ModelPixelScale);
    const QVector<double> tie = tags.value(ModelTiepoint);
    const QVector<double> keys = tags.value(GeoKeyDirectory);
    if (scale.size() < 2 || tie.size() < 6 || scale[0] <= 0 || scale[1] <= 0) {
        error = QStringLiteral("no ModelPixelScale/ModelTiepoint georeferencing");
        return false;
    }
    if (geoKey(keys, GT_MODEL_TYPE_KEY, 0) != MODEL_TYPE_GEOGRAPHIC) {
        error = QStringLiteral("not in geographic (latitude/longitude) coordinates");
        return false;
    }

    // Blocks are either tiles or strips spanning the full width
    const bool tiled = tags.contains(TileOffsets);
    const QVector<double> offsets = tags.value(tiled ? TileOffsets : StripOffsets);
    const QVector<double> counts = tags.value(tiled ? TileByteCounts : StripByteCounts);
    const int blockW = tiled ? int(tagValue(tags, TileWidth, 0)) : width;
    const int blockH = tiled ? int(tagValue(tags, TileLength, 0))
                             : int(qMin<double>(tagValue(tags, RowsPerStrip, height), height));
    if (blockW <= 0 || blockH <= 0 || offsets.isEmpty() || offsets.size() != counts.size()) {
        error = QStringLiteral("invalid tile or strip layout");
        return false;
    }
    const int across = (width + blockW - 1) / blockW;
    const int down = (height + blockH - 1) / blockH;
    if (offsets.size() < across * down) {
        error = QStringLiteral("missing tiles or strips");
        return false;
    }

    const int bytesPerSample = bits / 8;
    const int rowBytes = blockW * bytesPerSample;
    const int blockBytes = rowBytes * blockH;

    out = DemGrid();
    out.width = width;
    out.height = height;
    out.dx = scale[0];
    out.dy = scale[1];
    // The tiepoint maps raster (i, j) to model (x, y). For PixelIsArea, (0, 0)
    // is the outer corner of the first pixel; for PixelIsPoint, its centre.
    const double half = (geoKey(keys, GT_RASTER_TYPE_KEY, 1) == RASTER_PIXEL_IS_POINT) ? 0.0 : 0.5;
    out.lon0 = tie[3] + (half - tie[0]) * out.dx;
    out.lat0 = tie[4] - (half - tie[1]) * out.dy;
    out.heights.resize(width * height);

    for (int by = 0; by < down; by++) {
        for (int bx = 0; bx < across; bx++) {
            const int index = by * across + bx;
            const qint64 off = qint64(offsets[index]);
            const qint64 len = qint64(counts[index]);
            if (off < 0 || len < 0 || off + len > tiff.size()) {
                error = QStringLiteral("block %1 out of range").arg(index);
                return false;
            }
            QByteArray block = tiff.mid(int(off), int(len));
            if (compression != 1) {
                // qUncompress expects the zlib stream behind its decoded size
                QByteArray framed(4, 0);
                qToBigEndian<quint32>(quint32(blockBytes), reinterpret_cast<uchar *>(framed.data()));
                block = qUncompress(framed + block);
            }
            // The last strip may be shorter than the others
            const int rows = qMin(blockH, int(block.size() / rowBytes));
            if (rows <= 0) {
                error = QStringLiteral("block %1 could not be decompressed").arg(index);
                return false;
            }
            uchar *b = reinterpret_cast<uchar *>(block.data());
            QByteArray plane(rowBytes, 0);
            for (int r = 0; r < rows; r++) {
                uchar *row = b + r * rowBytes;
                if (predictor == 3) {
                    // Floating-point predictor: bytes are differenced, then
                    // stored as planes from the most to the least significant
                    for (int i = 1; i < rowBytes; i++)
                        row[i] = uchar(row[i] + row[i - 1]);
                    uchar *p = reinterpret_cast<uchar *>(plane.data());
                    for (int i = 0; i < blockW; i++) {
                        for (int k = 0; k < 4; k++)
                            p[i * 4 + (3 - k)] = row[k * blockW + i];
                    }
                    std::memcpy(row, p, rowBytes);
                } else if (predictor == 2) {
                    for (int i = 1; i < blockW; i++) {
                        quint16 v = quint16(qFromLittleEndian<quint16>(row + 2 * i)
                                            + qFromLittleEndian<quint16>(row + 2 * (i - 1)));
                        qToLittleEndian<quint16>(v, row + 2 * i);
                    }
                }
                const int y = by * blockH + r;
                if (y >= height)
                    break;
                for (int i = 0; i < blockW; i++) {
                    const int x = bx * blockW + i;
                    if (x >= width)
                        break;
                    float h;
                    if (isFloat) {
                        quint32 v = qFromLittleEndian<quint32>(row + 4 * i);
                        std::memcpy(&h, &v, 4);
                    } else if (format == 2) {
                        h = qFromLittleEndian<qint16>(row + 2 * i);
                    } else {
                        h = qFromLittleEndian<quint16>(row + 2 * i);
                    }
                    out.heights[y * width + x] = std::isfinite(h) ? h : 0.0f;
                }
            }
        }
    }
    return true;
}

} // namespace GeoTiff

CopernicusDem::CopernicusDem(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

QString CopernicusDem::tileName(int lat, int lon)
{
    return QString("Copernicus_DSM_COG_30_%1%2_00_%3%4_00_DEM")
        .arg(lat < 0 ? 'S' : 'N').arg(std::abs(lat), 2, 10, QChar('0'))
        .arg(lon < 0 ? 'W' : 'E').arg(std::abs(lon), 3, 10, QChar('0'));
}

DemGrid CopernicusDem::mosaic(const QList<DemGrid> &tiles, double west, double south,
                              double east, double north, int maxSize)
{
    double nativeDx = 1.0 / 1200, nativeDy = 1.0 / 1200;
    for (const DemGrid &t : tiles) {
        nativeDx = qMin(nativeDx, t.dx);
        nativeDy = qMin(nativeDy, t.dy);
    }
    const double stepX = qMax(nativeDx, (east - west) / (maxSize - 1));
    const double stepY = qMax(nativeDy, (north - south) / (maxSize - 1));

    DemGrid g;
    g.lon0 = west;
    g.lat0 = north;
    g.dx = stepX;
    g.dy = stepY;
    g.width = qBound(2, int(std::ceil((east - west) / stepX)) + 1, maxSize);
    g.height = qBound(2, int(std::ceil((north - south) / stepY)) + 1, maxSize);
    g.heights.resize(g.width * g.height);
    for (int r = 0; r < g.height; r++) {
        const double lat = g.lat0 - r * g.dy;
        for (int c = 0; c < g.width; c++) {
            const double lon = g.lon0 + c * g.dx;
            float h = 0.0f;  // Sea level where no tile exists
            for (const DemGrid &t : tiles) {
                const float v = t.sample(lon, lat, NAN);
                if (!std::isnan(v)) {
                    h = v;
                    break;
                }
            }
            g.heights[r * g.width + c] = h;
        }
    }
    return g;
}

QString CopernicusDem::cachePath(const QString &name) const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
        + "/Openambit/dem/" + name + ".tif";
}

void CopernicusDem::request(double west, double south, double east, double north)
{
    m_generation++;
    m_queue.clear();
    m_tiles.clear();
    m_networkErrors = 0;

    m_west = qBound(-180.0, west, 180.0);
    m_east = qBound(-180.0, east, 180.0);
    m_south = qBound(-90.0, south, 90.0);
    m_north = qBound(-90.0, north, 90.0);
    if (!(m_east > m_west) || !(m_north > m_south)) {
        emit failed(tr("Invalid area for the elevation model."));
        return;
    }

    const int lat0 = int(std::floor(m_south)), lat1 = int(std::floor(m_north - 1e-9));
    const int lon0 = int(std::floor(m_west)), lon1 = int(std::floor(m_east - 1e-9));
    m_total = (lat1 - lat0 + 1) * (lon1 - lon0 + 1);
    if (m_total > MAX_TILES) {
        emit failed(tr("Area too large for the elevation model (%1 tiles of 1 degree, at most %2).")
                        .arg(m_total).arg(MAX_TILES));
        return;
    }
    for (int lat = lat0; lat <= lat1; lat++) {
        for (int lon = lon0; lon <= lon1; lon++)
            m_queue << tileName(lat, lon);
    }
    QDir().mkpath(QFileInfo(cachePath("x")).absolutePath());
    fetchNext();
}

void CopernicusDem::fetchNext()
{
    while (!m_queue.isEmpty()) {
        const QString name = m_queue.first();
        emit progress(m_total - m_queue.size(), m_total);
        const QString path = cachePath(name);

        // A marker remembers that the tile does not exist (open sea)
        if (QFile::exists(path + ".none")) {
            m_queue.removeFirst();
            continue;
        }
        QFile cached(path);
        if (cached.open(QIODevice::ReadOnly)) {
            DemGrid tile;
            QString error;
            if (GeoTiff::decode(cached.readAll(), tile, error)) {
                m_tiles << tile;
                m_queue.removeFirst();
                continue;
            }
            // A damaged cache file is downloaded again
            cached.close();
            QFile::remove(path);
        }

        QNetworkRequest req(QUrl(QString("https://copernicus-dem-90m.s3.amazonaws.com/%1/%1.tif").arg(name)));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply *reply = m_nam->get(req);
        const int generation = m_generation;
        connect(reply, &QNetworkReply::finished, this, [this, reply, generation]() {
            reply->deleteLater();
            if (generation == m_generation)
                finishTile(reply);
        });
        return;
    }
    finish();
}

void CopernicusDem::finishTile(QNetworkReply *reply)
{
    const QString name = m_queue.takeFirst();
    const QString path = cachePath(name);
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (status == 404 || status == 403) {
        QFile marker(path + ".none");
        marker.open(QIODevice::WriteOnly);
    } else if (reply->error() != QNetworkReply::NoError) {
        m_networkErrors++;
    } else {
        const QByteArray data = reply->readAll();
        DemGrid tile;
        QString error;
        if (GeoTiff::decode(data, tile, error)) {
            m_tiles << tile;
            QFile f(path);
            if (f.open(QIODevice::WriteOnly))
                f.write(data);
        } else {
            m_networkErrors++;
        }
    }
    fetchNext();
}

void CopernicusDem::finish()
{
    emit progress(m_total, m_total);
    // Only a box made entirely of unreachable tiles is a failure: a partial
    // model is still better than a flat globe, and sea tiles are legitimately 0
    if (m_tiles.isEmpty() && m_networkErrors > 0) {
        emit failed(tr("The Copernicus elevation model could not be downloaded."));
        return;
    }
    emit ready(mosaic(m_tiles, m_west, m_south, m_east, m_north));
}
