#include <openambit/copernicusdem.h>
#include "doctest.h"

#include <QtEndian>
#include <cmath>
#include <cstring>

TEST_SUITE_BEGIN("CopernicusDem");

namespace {

// Builds a GeoTIFF laid out like a Copernicus DEM tile: float32, one
// Deflate-compressed tile with the floating-point predictor, PixelIsPoint.
QByteArray makeTiff(int width, int height, int tile, double west, double north, double step,
                    float (*value)(int x, int y), bool pixelIsPoint = true)
{
    // Pixels: predictor 3 encoding, row by row
    QByteArray tileData;
    for (int y = 0; y < tile; y++) {
        QByteArray planes(tile * 4, 0);
        for (int x = 0; x < tile; x++) {
            float v = (x < width && y < height) ? value(x, y) : 0.0f;
            quint32 bits;
            std::memcpy(&bits, &v, 4);
            for (int k = 0; k < 4; k++)
                planes[k * tile + x] = char((bits >> (8 * (3 - k))) & 0xff);
        }
        for (int i = planes.size() - 1; i > 0; i--)
            planes[i] = char(uchar(planes[i]) - uchar(planes[i - 1]));
        tileData += planes;
    }
    // qCompress prepends a 4-byte size; TIFF wants the bare zlib stream
    QByteArray compressed = qCompress(tileData).mid(4);

    struct Entry { quint16 tag, type; quint32 count; QByteArray data; };
    auto shorts = [](std::initializer_list<int> v) {
        QByteArray b; for (int x : v) { char c[2]; qToLittleEndian<quint16>(quint16(x), c); b.append(c, 2); } return b; };
    auto longs = [](std::initializer_list<quint32> v) {
        QByteArray b; for (quint32 x : v) { char c[4]; qToLittleEndian<quint32>(x, c); b.append(c, 4); } return b; };
    auto doubles = [](std::initializer_list<double> v) {
        QByteArray b; for (double x : v) { quint64 u; std::memcpy(&u, &x, 8); char c[8]; qToLittleEndian<quint64>(u, c); b.append(c, 8); } return b; };

    const quint32 dataOffset = 8;
    QList<Entry> entries = {
        {256, 3, 1, shorts({width})}, {257, 3, 1, shorts({height})}, {258, 3, 1, shorts({32})},
        {259, 3, 1, shorts({8})}, {277, 3, 1, shorts({1})}, {317, 3, 1, shorts({3})},
        {322, 3, 1, shorts({tile})}, {323, 3, 1, shorts({tile})},
        {324, 4, 1, longs({dataOffset})}, {325, 4, 1, longs({quint32(compressed.size())})},
        {339, 3, 1, shorts({3})},
        {33550, 12, 3, doubles({step, step, 0})},
        {33922, 12, 6, doubles({0, 0, 0, west, north, 0})},
        {34735, 3, 12, shorts({1, 1, 0, 2, 1024, 0, 1, 2, 1025, 0, 1, pixelIsPoint ? 2 : 1})},
    };

    QByteArray out = QByteArray("II") + shorts({42}) + longs({0});
    out += compressed;
    if (out.size() % 2) out += char(0);
    // Out-of-line values follow the pixels, then the IFD
    QList<quint32> valueOffsets;
    for (const Entry &e : entries) {
        if (e.data.size() > 4) {
            valueOffsets << quint32(out.size());
            out += e.data;
        } else {
            valueOffsets << 0;
        }
    }
    const quint32 ifd = quint32(out.size());
    qToLittleEndian<quint32>(ifd, out.data() + 4);
    out += shorts({int(entries.size())});
    for (int i = 0; i < entries.size(); i++) {
        const Entry &e = entries[i];
        out += shorts({e.tag, e.type}) + longs({e.count});
        QByteArray v = e.data.size() > 4 ? longs({valueOffsets[i]}) : e.data;
        v.append(QByteArray(4 - v.size(), 0));
        out += v;
    }
    out += longs({0});
    return out;
}

float ramp(int x, int y) { return float(100 * y + x) + 0.25f; }
float constant(int, int) { return 50.0f; }

} // namespace

TEST_CASE("decodes a Deflate float32 tile with predictor 3") {
    const QByteArray tiff = makeTiff(10, 7, 16, -3.0, 48.0, 0.1, ramp);
    DemGrid g;
    QString error;
    REQUIRE(GeoTiff::decode(tiff, g, error));
    CHECK(error.isEmpty());
    CHECK(g.width == 10);
    CHECK(g.height == 7);
    CHECK(g.heights[0] == doctest::Approx(0.25));
    CHECK(g.heights[3 * 10 + 4] == doctest::Approx(304.25));
    CHECK(g.heights[6 * 10 + 9] == doctest::Approx(609.25));
    // PixelIsPoint: the tiepoint is the centre of the first pixel
    CHECK(g.lon0 == doctest::Approx(-3.0));
    CHECK(g.lat0 == doctest::Approx(48.0));
    CHECK(g.dx == doctest::Approx(0.1));
}

TEST_CASE("PixelIsArea tiepoint is the corner of the first pixel") {
    DemGrid g;
    QString error;
    REQUIRE(GeoTiff::decode(makeTiff(4, 4, 16, 10.0, 20.0, 0.5, ramp, false), g, error));
    CHECK(g.lon0 == doctest::Approx(10.25));
    CHECK(g.lat0 == doctest::Approx(19.75));
}

TEST_CASE("rejects what it cannot read") {
    DemGrid g;
    QString error;
    CHECK_FALSE(GeoTiff::decode(QByteArray("MM\0*", 4), g, error));
    CHECK_FALSE(error.isEmpty());
    CHECK_FALSE(GeoTiff::decode(QByteArray(), g, error));
}

TEST_CASE("bilinear sampling and outside value") {
    DemGrid g;
    QString error;
    REQUIRE(GeoTiff::decode(makeTiff(10, 7, 16, 0.0, 1.0, 0.1, ramp), g, error));
    // Halfway between columns 2 and 3 of row 1: (102.25 + 103.25) / 2
    CHECK(g.sample(0.25, 0.9) == doctest::Approx(102.75));
    CHECK(std::isnan(g.sample(5.0, 0.5, NAN)));
}

TEST_CASE("tile names follow the Copernicus bucket") {
    CHECK(CopernicusDem::tileName(47, -3) == "Copernicus_DSM_COG_30_N47_00_W003_00_DEM");
    CHECK(CopernicusDem::tileName(-1, 0) == "Copernicus_DSM_COG_30_S01_00_E000_00_DEM");
    CHECK(CopernicusDem::tileName(5, 120) == "Copernicus_DSM_COG_30_N05_00_E120_00_DEM");
}

TEST_CASE("mosaic fills gaps with sea level and limits the size") {
    DemGrid g;
    QString error;
    REQUIRE(GeoTiff::decode(makeTiff(11, 11, 16, 0.0, 1.0, 0.1, constant), g, error));
    DemGrid m = CopernicusDem::mosaic({g}, 0.0, 0.0, 2.0, 1.0, 64);
    CHECK(m.width <= 64);
    CHECK(m.height <= 64);
    CHECK(m.sample(0.5, 0.5) == doctest::Approx(50.0));
    CHECK(m.sample(1.8, 0.5) == doctest::Approx(0.0));
}
