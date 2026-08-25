#include "apps_directory.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

#define ENTRY_HEADER_LEN 3  /* [u8 reserved=0][u8 activity_id][u8 marker] */
#define ENTRY_BLOCK_LEN (ENTRY_HEADER_LEN + APPS_DIR_NAME_LEN) /* 32: entry_offset -> magic */

static uint16_t rd_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd_u32(const uint8_t *p) { return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24)); }
static void wr_u16(uint8_t *p, uint16_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; }
static void wr_u32(uint8_t *p, uint32_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; p[2] = (v >> 16) & 0xff; p[3] = (v >> 24) & 0xff; }

uint8_t ambit_apps_dir_entry_marker(const uint8_t *binary, uint32_t binary_length)
{
    uint8_t checksum = 0;
    uint32_t i;

    for (i = 0; i < APPS_DIR_MAGIC_LEN; i++) {
        checksum ^= (uint8_t)APPS_DIR_MAGIC[i]; /* APPS_DIR_MAGIC is "IAMRULE\0" -> exactly 8 bytes with the NUL */
    }
    for (i = 0; i < binary_length; i++) {
        checksum ^= binary[i];
    }

    return checksum ^ (uint8_t)((APPS_DIR_MAGIC_LEN + binary_length) & 0xff);
}

int ambit_apps_dir_decode(const uint8_t *data, uint32_t data_len, ambit_apps_dir_region_t *out)
{
    uint16_t num_entries;
    uint32_t table_len;
    uint32_t i;

    memset(out, 0, sizeof(*out));

    if (data_len < 4) {
        LOG_ERROR("Apps region: too short to contain a directory header");
        return -1;
    }

    num_entries = rd_u16(data);
    /* data+2: "unknown2" - not used on decode, see apps_directory.h's own doc comment. */

    if (num_entries == 0 || num_entries > 1000) {
        LOG_ERROR("Apps region: implausible entry count %u, not a real directory", num_entries);
        return -1;
    }
    if (num_entries > APPS_DIR_MAX_ENTRIES) {
        LOG_ERROR("Apps region: %u entries exceeds this driver's internal limit of %d", num_entries, APPS_DIR_MAX_ENTRIES);
        return -1;
    }

    table_len = 4 + 4 * ((uint32_t)num_entries + 1);
    if (table_len > data_len) {
        LOG_ERROR("Apps region: directory table (%u bytes) exceeds buffer (%u bytes)", table_len, data_len);
        return -1;
    }

    {
        uint32_t first_offset = rd_u32(data + 4);
        if (first_offset != table_len) {
            LOG_ERROR("Apps region: first entry_offset (%u) does not match the directory's own size (%u) - not a real directory", first_offset, table_len);
            return -1;
        }
    }

    {
        uint32_t total_length = rd_u32(data + 4 + 4 * num_entries);
        if (total_length > data_len) {
            LOG_ERROR("Apps region: total_length (%u) exceeds buffer (%u bytes)", total_length, data_len);
            return -1;
        }

        for (i = 0; i < num_entries; i++) {
            uint32_t off = rd_u32(data + 4 + 4 * i);
            uint32_t magic_off = off + ENTRY_BLOCK_LEN;
            uint32_t bin_start = magic_off + APPS_DIR_MAGIC_LEN;
            uint32_t bin_end = (i + 1 < num_entries) ? rd_u32(data + 4 + 4 * (i + 1)) : total_length;
            ambit_apps_dir_entry_t *entry = &out->entries[i];

            if (bin_start > total_length || bin_end < bin_start || bin_end > total_length) {
                LOG_ERROR("Apps region: entry %u has an out-of-range binary span, region doesn't match this format", i);
                memset(out, 0, sizeof(*out));
                return -1;
            }
            if (memcmp(data + magic_off, APPS_DIR_MAGIC, APPS_DIR_MAGIC_LEN - 1) != 0) {
                LOG_ERROR("Apps region: entry %u's IAMRULE magic not found where the directory says it should be", i);
                memset(out, 0, sizeof(*out));
                return -1;
            }

            entry->activity_id = data[off + 1];
            /* data[off+2] is the 'marker' checksum - recomputed by the caller on encode, not trusted on decode. */
            {
                uint32_t name_len = 0;
                while (name_len < APPS_DIR_NAME_LEN && data[off + ENTRY_HEADER_LEN + name_len] != 0) {
                    name_len++;
                }
                memcpy(entry->name, data + off + ENTRY_HEADER_LEN, name_len);
                entry->name[name_len] = '\0';
            }
            entry->binary = (uint8_t *)(data + bin_start);
            entry->binary_length = bin_end - bin_start;
        }
    }

    out->entries_count = num_entries;
    return 0;
}

int ambit_apps_dir_encode(const ambit_apps_dir_region_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len)
{
    uint32_t table_len;
    uint32_t offsets[APPS_DIR_MAX_ENTRIES];
    uint32_t cursor;
    uint32_t total_length;
    uint16_t i;

    if (in->entries_count > APPS_DIR_MAX_ENTRIES) {
        LOG_ERROR("Apps region: %u entries exceeds this driver's internal limit of %d", in->entries_count, APPS_DIR_MAX_ENTRIES);
        return -1;
    }

    table_len = 4 + 4 * ((uint32_t)in->entries_count + 1);
    cursor = table_len;
    for (i = 0; i < in->entries_count; i++) {
        offsets[i] = cursor;
        cursor += ENTRY_BLOCK_LEN + APPS_DIR_MAGIC_LEN + in->entries[i].binary_length;
    }
    total_length = cursor;

    if (total_length > out_capacity) {
        LOG_ERROR("Apps region: encoded size %u exceeds region capacity %u", total_length, out_capacity);
        return -1;
    }

    wr_u16(out, in->entries_count);
    wr_u16(out + 2, in->entries_count ^ 0x02);
    for (i = 0; i < in->entries_count; i++) {
        wr_u32(out + 4 + 4 * i, offsets[i]);
    }
    wr_u32(out + 4 + 4 * in->entries_count, total_length);

    for (i = 0; i < in->entries_count; i++) {
        const ambit_apps_dir_entry_t *entry = &in->entries[i];
        uint8_t *block = out + offsets[i];
        uint32_t name_len;

        block[0] = 0; /* reserved */
        block[1] = entry->activity_id;
        block[2] = ambit_apps_dir_entry_marker(entry->binary, entry->binary_length);

        name_len = (uint32_t)strlen(entry->name);
        if (name_len > APPS_DIR_NAME_LEN) {
            LOG_WARNING("Apps region: entry name \"%s\" (%u bytes) exceeds the %d-byte field, truncating", entry->name, name_len, APPS_DIR_NAME_LEN);
            name_len = APPS_DIR_NAME_LEN;
        }
        memset(block + ENTRY_HEADER_LEN, 0, APPS_DIR_NAME_LEN);
        memcpy(block + ENTRY_HEADER_LEN, entry->name, name_len);

        memcpy(block + ENTRY_BLOCK_LEN, APPS_DIR_MAGIC, APPS_DIR_MAGIC_LEN);
        memcpy(block + ENTRY_BLOCK_LEN + APPS_DIR_MAGIC_LEN, entry->binary, entry->binary_length);
    }

    *out_len = total_length;
    return 0;
}
