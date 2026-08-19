extern "C" {
#include <libambit/libambit.h>
}
#include "doctest.h"

TEST_SUITE_BEGIN("sport_mode_validate");

TEST_CASE("empty device settings are valid") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();

    CHECK(libambit_sport_mode_validate(settings) == 0);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_CASE("NULL device settings are valid (nothing to check)") {
    CHECK(libambit_sport_mode_validate(NULL) == 0);
}

TEST_CASE("unique sport_mode_id values are valid") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();
    REQUIRE(libambit_malloc_sport_modes(19, settings));

    for (int i = 0; i < 19; i++) {
        settings->sport_modes[i].settings.sport_mode_id = i + 1;   // 1..19, like the factory defaults
    }

    CHECK(libambit_sport_mode_validate(settings) == 0);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_CASE("duplicate sport_mode_id is rejected") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();
    REQUIRE(libambit_malloc_sport_modes(20, settings));

    for (int i = 0; i < 19; i++) {
        settings->sport_modes[i].settings.sport_mode_id = i + 1;   // 1..19
    }
    // Reproduces the openambit GUI bug this guards against: a 20th mode
    // added with id = list size (19) collides with the last factory preset.
    settings->sport_modes[19].settings.sport_mode_id = 19;

    CHECK(libambit_sport_mode_validate(settings) == -1);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_CASE("unique sport_mode_group_id values are valid") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();
    REQUIRE(libambit_malloc_sport_mode_groups(3, settings));

    settings->sport_mode_groups[0].sport_mode_group_id = 1;
    settings->sport_mode_groups[1].sport_mode_group_id = 2;
    settings->sport_mode_groups[2].sport_mode_group_id = 3;

    CHECK(libambit_sport_mode_validate(settings) == 0);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_CASE("duplicate sport_mode_group_id is rejected") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();
    REQUIRE(libambit_malloc_sport_mode_groups(3, settings));

    settings->sport_mode_groups[0].sport_mode_group_id = 1;
    settings->sport_mode_groups[1].sport_mode_group_id = 2;
    settings->sport_mode_groups[2].sport_mode_group_id = 1;   // collides with index 0

    CHECK(libambit_sport_mode_validate(settings) == -1);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_CASE("duplicate group id does not mask a duplicate mode id, and vice versa") {
    ambit_sport_mode_device_settings_t *settings = libambit_malloc_sport_mode_device_settings();
    REQUIRE(libambit_malloc_sport_modes(2, settings));
    REQUIRE(libambit_malloc_sport_mode_groups(2, settings));

    settings->sport_modes[0].settings.sport_mode_id = 1;
    settings->sport_modes[1].settings.sport_mode_id = 2;
    settings->sport_mode_groups[0].sport_mode_group_id = 10;
    settings->sport_mode_groups[1].sport_mode_group_id = 10;   // duplicate group id only

    CHECK(libambit_sport_mode_validate(settings) == -1);

    libambit_sport_mode_device_settings_free(settings);
}

TEST_SUITE_END();
