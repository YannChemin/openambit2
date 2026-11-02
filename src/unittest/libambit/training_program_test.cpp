extern "C" {
#include <libambit/training_program.h>
}
#include "doctest.h"

#include <cstring>
#include <string>

TEST_SUITE_BEGIN("training_program");

static ambit_training_program_item_t makeItem(const char *name, uint16_t activityId, uint16_t durationMinutes, uint8_t dayOffset)
{
    ambit_training_program_item_t item;
    memset(&item, 0, sizeof(item));
    strncpy(item.name, name, TRAINING_PROGRAM_NAME_LEN);
    item.activity_id = activityId;
    item.duration_minutes = durationMinutes;
    item.intensity = 3;
    item.day_offset = dayOffset;
    return item;
}

TEST_CASE("default HR zones (60/70/80/90) are valid and encode to the historically-observed bytes") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    program.base_year = 2026;
    program.base_month = 8;
    program.base_day = 20;
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = 0;

    uint8_t buffer[64];
    uint32_t written = 0;
    REQUIRE(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == 0);
    CHECK(written == TRAINING_PROGRAM_HEADER_SIZE);

    // 60/70/80/90 decimal = 0x3C/0x46/0x50/0x5A - the bytes once mistaken for a fixed signature.
    CHECK(buffer[4] == 0x3c);
    CHECK(buffer[5] == 0x46);
    CHECK(buffer[6] == 0x50);
    CHECK(buffer[7] == 0x5a);
}

TEST_CASE("encode refuses HR zones that aren't strictly increasing") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    program.hr_zones[0] = 70;
    program.hr_zones[1] = 60; // out of order
    program.hr_zones[2] = 80;
    program.hr_zones[3] = 90;

    uint8_t buffer[64];
    uint32_t written = 0;
    CHECK(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == -1);
}

TEST_CASE("encode refuses an HR zone >= 101") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    program.hr_zones[0] = 60;
    program.hr_zones[1] = 70;
    program.hr_zones[2] = 80;
    program.hr_zones[3] = 101; // must be < 101

    uint8_t buffer[64];
    uint32_t written = 0;
    CHECK(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == -1);
}

TEST_CASE("a single-item program round-trips byte-exact") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    program.base_year = 2026;
    program.base_month = 8;
    program.base_day = 20;
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = 1;
    program.items[0] = makeItem("Long run", 3, 60, 0);
    program.items[0].distance = 10000;

    uint8_t buffer[256];
    uint32_t written = 0;
    REQUIRE(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == 0);
    CHECK(written == TRAINING_PROGRAM_HEADER_SIZE + TRAINING_PROGRAM_ITEM_SIZE);

    ambit_training_program_t decoded;
    REQUIRE(ambit_training_program_decode(buffer, written, &decoded) == 0);
    CHECK(decoded.base_year == 2026);
    CHECK(decoded.base_month == 8);
    CHECK(decoded.base_day == 20);
    CHECK(memcmp(decoded.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4) == 0);
    REQUIRE(decoded.items_count == 1);
    CHECK(std::string(decoded.items[0].name) == "Long run");
    CHECK(decoded.items[0].activity_id == 3);
    CHECK(decoded.items[0].duration_minutes == 60);
    CHECK(decoded.items[0].distance == 10000);
    CHECK(decoded.items[0].day_offset == 0);
    CHECK(decoded.items[0].completed == false);

    uint8_t buffer2[256];
    uint32_t written2 = 0;
    REQUIRE(ambit_training_program_encode(&decoded, buffer2, sizeof(buffer2), &written2) == 0);
    CHECK(written2 == written);
    CHECK(memcmp(buffer, buffer2, written) == 0);
}

TEST_CASE("multiple items with different day_offsets round-trip in order") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    program.base_year = 2026;
    program.base_month = 8;
    program.base_day = 20;
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = 3;
    program.items[0] = makeItem("Day 0", 3, 30, 0);
    program.items[1] = makeItem("Day 2", 4, 45, 2);
    program.items[2] = makeItem("Day 5", 6, 20, 5);

    uint8_t buffer[512];
    uint32_t written = 0;
    REQUIRE(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == 0);

    ambit_training_program_t decoded;
    REQUIRE(ambit_training_program_decode(buffer, written, &decoded) == 0);
    REQUIRE(decoded.items_count == 3);
    CHECK(decoded.items[0].day_offset == 0);
    CHECK(decoded.items[1].day_offset == 2);
    CHECK(decoded.items[2].day_offset == 5);
    CHECK(std::string(decoded.items[1].name) == "Day 2");
    CHECK(decoded.items[1].activity_id == 4);
}

TEST_CASE("a name longer than the 23-byte field is truncated on encode, not overflowed") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = 1;
    program.items[0] = makeItem("This name is much longer than 23 characters", 3, 30, 0);

    uint8_t buffer[256];
    uint32_t written = 0;
    REQUIRE(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == 0);

    ambit_training_program_t decoded;
    REQUIRE(ambit_training_program_decode(buffer, written, &decoded) == 0);
    CHECK(strlen(decoded.items[0].name) <= TRAINING_PROGRAM_NAME_LEN);
}

TEST_CASE("decode treats an erased/empty region as zero items, not an error") {
    uint8_t buffer[TRAINING_PROGRAM_HEADER_SIZE];
    memset(buffer, 0xff, sizeof(buffer));

    ambit_training_program_t decoded;
    REQUIRE(ambit_training_program_decode(buffer, sizeof(buffer), &decoded) == 0);
    CHECK(decoded.items_count == 0);
}

TEST_CASE("decode rejects a buffer too short for the declared item count") {
    uint8_t buffer[TRAINING_PROGRAM_HEADER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    buffer[8] = 5; // claims 5 items but the buffer has no item bytes at all

    ambit_training_program_t decoded;
    CHECK(ambit_training_program_decode(buffer, sizeof(buffer), &decoded) == -1);
}

TEST_CASE("encode refuses more items than the firmware's own limit") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = TRAINING_PROGRAM_MAX_ITEMS + 1;

    uint8_t buffer[4096];
    uint32_t written = 0;
    CHECK(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == -1);
}

TEST_CASE("encode refuses a program that would not fit in the given capacity") {
    ambit_training_program_t program;
    memset(&program, 0, sizeof(program));
    memcpy(program.hr_zones, TRAINING_PROGRAM_DEFAULT_HR_ZONES, 4);
    program.items_count = 1;
    program.items[0] = makeItem("Run", 3, 30, 0);

    uint8_t buffer[8]; // far too small
    uint32_t written = 0;
    CHECK(ambit_training_program_encode(&program, buffer, sizeof(buffer), &written) == -1);
}

TEST_SUITE_END();
