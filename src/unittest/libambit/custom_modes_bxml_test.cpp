extern "C" {
#include <libambit/custom_modes_bxml.h>
}
#include "doctest.h"

#include <cstring>
#include <string>

TEST_SUITE_BEGIN("custom_modes_bxml");

static void fill_minimal_settings(ambit_custom_mode_settings_t *settings, const char *name, uint16_t activity_id, uint32_t custom_mode_id)
{
    memset(settings, 0, sizeof(*settings));
    strncpy(settings->name, name, CUSTOM_MODES_BXML_NAME_SIZE);
    settings->activity_id = activity_id;
    settings->custom_mode_id = custom_mode_id;
    settings->use_hw = 3;
    settings->alti_baro_mode = 1;
    settings->recording_interval = 1;
    settings->hr_high = 165;
    settings->hr_low = 125;
    settings->interval_slots[5].flags = 0;       // BacklightMode
    settings->interval_slots[5].max_limit = 255; // DisplayMode
    settings->interval_slots[5].min_limit = 255; // QuickNavigation
}

TEST_CASE("empty region round-trips") {
    ambit_custom_modes_region_t region;
    memset(&region, 0, sizeof(region));
    region.format_type = 2;

    uint8_t buffer[512];
    uint32_t written = 0;
    REQUIRE(ambit_custom_modes_encode(&region, buffer, sizeof(buffer), &written) == 0);
    CHECK(written == sizeof(buffer));

    ambit_custom_modes_region_t decoded;
    REQUIRE(ambit_custom_modes_decode(buffer, sizeof(buffer), &decoded) == 0);
    CHECK(decoded.format_type == 2);
    CHECK(decoded.modes_count == 0);
    CHECK(decoded.sport_modes_count == 0);
}

TEST_CASE("a single mode with one display round-trips byte-exact") {
    ambit_custom_modes_region_t region;
    memset(&region, 0, sizeof(region));
    region.format_type = 2;
    region.modes_count = 1;

    fill_minimal_settings(&region.modes[0].settings, "Running", 3, 100);

    region.modes[0].displays_count = 1;
    ambit_custom_mode_display_t *disp = &region.modes[0].displays[0];
    disp->template_id = 0x0104; // TRIPLE_ROWS
    disp->type = 10;
    disp->fields_count = 3;
    disp->fields[0].index = 31;
    disp->fields[1].index = 21;
    disp->fields[2].index = 0;
    disp->fields[2].shortcuts_count = 4;
    disp->fields[2].shortcuts[0] = 1;
    disp->fields[2].shortcuts[1] = 2;
    disp->fields[2].shortcuts[2] = 3;
    disp->fields[2].shortcuts[3] = 4;

    region.sport_modes_count = 1;
    strncpy(region.sport_modes[0].name, "Running", CUSTOM_MODES_BXML_NAME_SIZE);
    region.sport_modes[0].activity_id = 3;
    region.sport_modes[0].exercises_count = 1;
    region.sport_modes[0].exercises[0] = 0;

    uint8_t buffer[4096];
    uint32_t written = 0;
    REQUIRE(ambit_custom_modes_encode(&region, buffer, sizeof(buffer), &written) == 0);

    ambit_custom_modes_region_t decoded;
    REQUIRE(ambit_custom_modes_decode(buffer, sizeof(buffer), &decoded) == 0);

    CHECK(decoded.modes_count == 1);
    CHECK(std::string(decoded.modes[0].settings.name) == "Running");
    CHECK(decoded.modes[0].settings.activity_id == 3);
    CHECK(decoded.modes[0].settings.custom_mode_id == 100);
    CHECK(decoded.modes[0].settings.hr_high == 165);
    CHECK(decoded.modes[0].settings.hr_low == 125);
    CHECK(decoded.modes[0].displays_count == 1);
    CHECK(decoded.modes[0].displays[0].template_id == 0x0104);
    CHECK(decoded.modes[0].displays[0].fields_count == 3);
    CHECK(decoded.modes[0].displays[0].fields[0].index == 31);
    CHECK(decoded.modes[0].displays[0].fields[2].shortcuts_count == 4);
    CHECK(decoded.modes[0].displays[0].fields[2].shortcuts[3] == 4);

    CHECK(decoded.sport_modes_count == 1);
    CHECK(std::string(decoded.sport_modes[0].name) == "Running");
    CHECK(decoded.sport_modes[0].exercises_count == 1);
    CHECK(decoded.sport_modes[0].exercises[0] == 0);

    // Re-encoding the decoded struct must reproduce the exact same bytes (the actual
    // property this project relied on before ever writing to real hardware).
    uint8_t buffer2[4096];
    uint32_t written2 = 0;
    REQUIRE(ambit_custom_modes_encode(&decoded, buffer2, sizeof(buffer2), &written2) == 0);
    CHECK(written2 == written);
    CHECK(memcmp(buffer, buffer2, written) == 0);
}

TEST_CASE("a multisport slot with several exercise legs (Triathlon-shaped) round-trips") {
    ambit_custom_modes_region_t region;
    memset(&region, 0, sizeof(region));
    region.format_type = 2;
    region.modes_count = 4;
    fill_minimal_settings(&region.modes[0].settings, "Openwater swim", 83, 1);
    fill_minimal_settings(&region.modes[1].settings, "Transition", 1, 2);
    fill_minimal_settings(&region.modes[2].settings, "Cycling", 4, 3);
    fill_minimal_settings(&region.modes[3].settings, "Running", 3, 4);

    region.sport_modes_count = 1;
    ambit_multisport_slot_t *slot = &region.sport_modes[0];
    strncpy(slot->name, "Triathlon", CUSTOM_MODES_BXML_NAME_SIZE);
    slot->activity_id = 19;
    slot->exercises_count = 5;
    uint16_t legs[5] = {0, 1, 2, 1, 3};
    memcpy(slot->exercises, legs, sizeof(legs));
    slot->has_order = true;
    slot->order = 10;

    uint8_t buffer[4096];
    uint32_t written = 0;
    REQUIRE(ambit_custom_modes_encode(&region, buffer, sizeof(buffer), &written) == 0);

    ambit_custom_modes_region_t decoded;
    REQUIRE(ambit_custom_modes_decode(buffer, sizeof(buffer), &decoded) == 0);

    REQUIRE(decoded.sport_modes_count == 1);
    CHECK(decoded.sport_modes[0].exercises_count == 5);
    for (int i = 0; i < 5; i++) {
        CHECK(decoded.sport_modes[0].exercises[i] == legs[i]);
    }
    CHECK(decoded.sport_modes[0].has_order == true);
    CHECK(decoded.sport_modes[0].order == 10);
}

TEST_CASE("decode rejects a buffer that doesn't start with the DEVICE_CUSTOM tag") {
    uint8_t garbage[16] = {0};
    garbage[0] = 0xff;
    garbage[1] = 0xff;
    ambit_custom_modes_region_t decoded;
    CHECK(ambit_custom_modes_decode(garbage, sizeof(garbage), &decoded) == -1);
}

TEST_CASE("encode refuses a region that would not fit in the given capacity") {
    ambit_custom_modes_region_t region;
    memset(&region, 0, sizeof(region));
    region.modes_count = 1;
    fill_minimal_settings(&region.modes[0].settings, "Running", 3, 1);

    uint8_t buffer[8]; // far too small
    uint32_t written = 0;
    CHECK(ambit_custom_modes_encode(&region, buffer, sizeof(buffer), &written) == -1);
}

TEST_SUITE_END();
