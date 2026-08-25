extern "C" {
#include <libambit/apps_directory.h>
}
#include "doctest.h"

#include <cstring>
#include <string>

TEST_SUITE_BEGIN("apps_directory");

TEST_CASE("entry_marker matches a hand-computed XOR checksum") {
    uint8_t binary[] = {0x01, 0x02, 0x03, 0x04};
    // payload = "IAMRULE\0" + binary, XOR all bytes, then XOR with (payload length & 0xff).
    uint8_t expected = 0;
    const char *magic = "IAMRULE";
    for (int i = 0; i < 8; i++) {
        expected ^= (uint8_t)magic[i]; // includes the trailing NUL, 8 bytes total
    }
    for (uint8_t b : binary) {
        expected ^= b;
    }
    expected ^= (uint8_t)(12 & 0xff); // 8-byte magic + 4-byte binary = 12
    CHECK(ambit_apps_dir_entry_marker(binary, sizeof(binary)) == expected);
}

TEST_CASE("a single-entry region round-trips byte-exact") {
    ambit_apps_dir_region_t region;
    memset(&region, 0, sizeof(region));
    region.entries_count = 1;

    uint8_t binary[] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x02};
    strncpy(region.entries[0].name, "My Workout", APPS_DIR_NAME_LEN);
    region.entries[0].activity_id = 3;
    region.entries[0].binary = binary;
    region.entries[0].binary_length = sizeof(binary);

    uint8_t buffer[256];
    uint32_t written = 0;
    REQUIRE(ambit_apps_dir_encode(&region, buffer, sizeof(buffer), &written) == 0);

    // Directory layout sanity: [u16 num_entries=1][u16 1^2][u32 offset0=12][u32 total_length]
    CHECK(buffer[0] == 1);
    CHECK(buffer[1] == 0);
    CHECK(buffer[2] == (1 ^ 0x02));
    uint32_t offset0 = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | ((uint32_t)buffer[7] << 24);
    CHECK(offset0 == 12); // table_len for 1 entry: 4 + 4*(1+1)

    ambit_apps_dir_region_t decoded;
    REQUIRE(ambit_apps_dir_decode(buffer, written, &decoded) == 0);
    REQUIRE(decoded.entries_count == 1);
    CHECK(std::string(decoded.entries[0].name) == "My Workout");
    CHECK(decoded.entries[0].activity_id == 3);
    REQUIRE(decoded.entries[0].binary_length == sizeof(binary));
    CHECK(memcmp(decoded.entries[0].binary, binary, sizeof(binary)) == 0);

    // Re-encoding the decoded struct must reproduce the exact same bytes.
    uint8_t buffer2[256];
    uint32_t written2 = 0;
    REQUIRE(ambit_apps_dir_encode(&decoded, buffer2, sizeof(buffer2), &written2) == 0);
    CHECK(written2 == written);
    CHECK(memcmp(buffer, buffer2, written) == 0);
}

TEST_CASE("a multi-entry region round-trips and each entry's binary span is correct") {
    ambit_apps_dir_region_t region;
    memset(&region, 0, sizeof(region));
    region.entries_count = 3;

    uint8_t bin0[] = {0x01};
    uint8_t bin1[] = {0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t bin2[] = {0x07, 0x08, 0x09};

    strncpy(region.entries[0].name, "App A", APPS_DIR_NAME_LEN);
    region.entries[0].activity_id = 1;
    region.entries[0].binary = bin0;
    region.entries[0].binary_length = sizeof(bin0);

    strncpy(region.entries[1].name, "App B", APPS_DIR_NAME_LEN);
    region.entries[1].activity_id = 2;
    region.entries[1].binary = bin1;
    region.entries[1].binary_length = sizeof(bin1);

    strncpy(region.entries[2].name, "App C", APPS_DIR_NAME_LEN);
    region.entries[2].activity_id = 3;
    region.entries[2].binary = bin2;
    region.entries[2].binary_length = sizeof(bin2);

    uint8_t buffer[512];
    uint32_t written = 0;
    REQUIRE(ambit_apps_dir_encode(&region, buffer, sizeof(buffer), &written) == 0);

    ambit_apps_dir_region_t decoded;
    REQUIRE(ambit_apps_dir_decode(buffer, written, &decoded) == 0);
    REQUIRE(decoded.entries_count == 3);

    CHECK(std::string(decoded.entries[0].name) == "App A");
    REQUIRE(decoded.entries[0].binary_length == sizeof(bin0));
    CHECK(memcmp(decoded.entries[0].binary, bin0, sizeof(bin0)) == 0);

    CHECK(std::string(decoded.entries[1].name) == "App B");
    REQUIRE(decoded.entries[1].binary_length == sizeof(bin1));
    CHECK(memcmp(decoded.entries[1].binary, bin1, sizeof(bin1)) == 0);

    CHECK(std::string(decoded.entries[2].name) == "App C");
    REQUIRE(decoded.entries[2].binary_length == sizeof(bin2));
    CHECK(memcmp(decoded.entries[2].binary, bin2, sizeof(bin2)) == 0);
}

TEST_CASE("a name longer than the 29-byte field is truncated on encode, not overflowed") {
    ambit_apps_dir_region_t region;
    memset(&region, 0, sizeof(region));
    region.entries_count = 1;

    uint8_t binary[] = {0x01};
    // 38 chars, matching the real edge case apps_directory.h's provenance comment describes.
    strncpy(region.entries[0].name, "This name is much longer than 29 chars", APPS_DIR_NAME_LEN);
    region.entries[0].name[APPS_DIR_NAME_LEN] = '\0';
    region.entries[0].binary = binary;
    region.entries[0].binary_length = sizeof(binary);

    uint8_t buffer[128];
    uint32_t written = 0;
    REQUIRE(ambit_apps_dir_encode(&region, buffer, sizeof(buffer), &written) == 0);

    ambit_apps_dir_region_t decoded;
    REQUIRE(ambit_apps_dir_decode(buffer, written, &decoded) == 0);
    CHECK(strlen(decoded.entries[0].name) <= APPS_DIR_NAME_LEN);
}

TEST_CASE("decode rejects a region with zero entries (an empty/default directory)") {
    // This is the real shape found on a live device with no apps installed:
    // [u16 num_entries=0][u16 2][u32 total_length=8].
    uint8_t buffer[8] = {0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00};
    ambit_apps_dir_region_t decoded;
    CHECK(ambit_apps_dir_decode(buffer, sizeof(buffer), &decoded) == -1);
}

TEST_CASE("decode rejects a buffer that's too short for even the directory header") {
    uint8_t buffer[2] = {0x01, 0x00};
    ambit_apps_dir_region_t decoded;
    CHECK(ambit_apps_dir_decode(buffer, sizeof(buffer), &decoded) == -1);
}

TEST_CASE("decode rejects a region whose first entry_offset doesn't match the directory size") {
    uint8_t buffer[16] = {0};
    buffer[0] = 1; // num_entries = 1
    buffer[4] = 99; // bogus first entry_offset, should be 12 for 1 entry
    ambit_apps_dir_region_t decoded;
    CHECK(ambit_apps_dir_decode(buffer, sizeof(buffer), &decoded) == -1);
}

TEST_CASE("encode refuses a region that would not fit in the given capacity") {
    ambit_apps_dir_region_t region;
    memset(&region, 0, sizeof(region));
    region.entries_count = 1;
    uint8_t binary[100] = {0};
    strncpy(region.entries[0].name, "Big", APPS_DIR_NAME_LEN);
    region.entries[0].binary = binary;
    region.entries[0].binary_length = sizeof(binary);

    uint8_t buffer[8]; // far too small
    uint32_t written = 0;
    CHECK(ambit_apps_dir_encode(&region, buffer, sizeof(buffer), &written) == -1);
}

TEST_SUITE_END();
