#include <openambit/activitycategory.h>
#include "doctest.h"

TEST_SUITE_BEGIN("ActivityCategory");

using namespace ActivityCategory;

TEST_CASE("paragliding activity type is an air sport") {
    CHECK(classify(ACTIVITY_PARAGLIDING, "Whatever") == Air);
    CHECK(prefers3D(classify(ACTIVITY_PARAGLIDING, "")));
}

TEST_CASE("aircraft recorded as unspecified sport are recognised by name") {
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Helicopter") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Heli") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Microlight ULM") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Powered Flight") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Skydiving") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Balloon") == Air);
}

TEST_CASE("names in other languages and with accents") {
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Parapente") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, QString::fromUtf8("Hélicoptère")) == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, QString::fromUtf8("Vol à voile")) == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Deltaplane") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Gleitschirm") == Air);
    CHECK(classify(ACTIVITY_UNSPECIFIED, QString::fromUtf8("Montgolfière")) == Air);
}

TEST_CASE("gliding wins over sailing, sailing stays water") {
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Sailplane") == Air);
    CHECK(classify(0x0d, "Sailing") == Water);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Voile") == Water);
}

TEST_CASE("ground, vertical and underwater activities") {
    CHECK(classify(0x03, "Running") == Ground);
    CHECK(classify(0x04, "Cycling") == Ground);
    CHECK(classify(0x10, "") == Vertical);
    CHECK(classify(ACTIVITY_UNSPECIFIED, "Via Ferrata") == Vertical);
    CHECK(classify(0x33, "") == Underwater);
    CHECK_FALSE(prefers3D(Vertical));
    CHECK_FALSE(prefers3D(Ground));
}

TEST_CASE("stable keys") {
    CHECK(key(Air) == "air");
    CHECK(key(Ground) == "ground");
}
