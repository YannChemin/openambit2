#include "training_program.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

const uint8_t TRAINING_PROGRAM_DEFAULT_HR_ZONES[4] = {60, 70, 80, 90};

static uint16_t rd_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd_u32(const uint8_t *p) { return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24)); }
static void wr_u16(uint8_t *p, uint16_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; }
static void wr_u32(uint8_t *p, uint32_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; p[2] = (v >> 16) & 0xff; p[3] = (v >> 24) & 0xff; }

static bool hr_zones_valid(const uint8_t zones[4])
{
    return zones[0] >= 1 && zones[0] < zones[1] && zones[1] < zones[2] && zones[2] < zones[3] && zones[3] < 101;
}

int ambit_training_program_encode(const ambit_training_program_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len)
{
    uint32_t total_length;
    uint16_t i;

    if (!hr_zones_valid(in->hr_zones)) {
        LOG_ERROR("TrainingProgram: HR zones {%u,%u,%u,%u} must be strictly increasing with the last < 101 "
                   "(the watch firmware rejects the whole header otherwise)",
                   in->hr_zones[0], in->hr_zones[1], in->hr_zones[2], in->hr_zones[3]);
        return -1;
    }
    if (in->items_count > TRAINING_PROGRAM_MAX_ITEMS) {
        LOG_ERROR("TrainingProgram: %u items exceeds the firmware's own limit of %d", in->items_count, TRAINING_PROGRAM_MAX_ITEMS);
        return -1;
    }

    total_length = TRAINING_PROGRAM_HEADER_SIZE + (uint32_t)in->items_count * TRAINING_PROGRAM_ITEM_SIZE;
    if (total_length > out_capacity) {
        LOG_ERROR("TrainingProgram: encoded size %u exceeds region capacity %u", total_length, out_capacity);
        return -1;
    }

    wr_u16(out, in->base_year);
    out[2] = in->base_month;
    out[3] = in->base_day;
    memcpy(out + 4, in->hr_zones, 4);
    wr_u16(out + 8, in->items_count);
    wr_u16(out + 10, 0xffff);

    for (i = 0; i < in->items_count; i++) {
        const ambit_training_program_item_t *item = &in->items[i];
        uint8_t *block = out + TRAINING_PROGRAM_HEADER_SIZE + (uint32_t)i * TRAINING_PROGRAM_ITEM_SIZE;
        uint32_t name_len;

        block[0] = item->day_offset;
        block[1] = item->completed ? 1 : 0;
        wr_u16(block + 2, item->activity_id);
        wr_u32(block + 4, item->move_id);
        wr_u32(block + 8, item->distance);
        wr_u16(block + 12, item->duration_minutes);
        block[14] = item->intensity;
        block[15] = 0; /* padding */

        name_len = (uint32_t)strlen(item->name);
        if (name_len > TRAINING_PROGRAM_NAME_LEN) {
            LOG_WARNING("TrainingProgram: item name \"%s\" (%u bytes) exceeds the %d-byte field, truncating",
                        item->name, name_len, TRAINING_PROGRAM_NAME_LEN);
            name_len = TRAINING_PROGRAM_NAME_LEN;
        }
        memset(block + 16, 0, TRAINING_PROGRAM_NAME_LEN);
        memcpy(block + 16, item->name, name_len);

        block[39] = 0; /* padding */
    }

    *out_len = total_length;
    return 0;
}

int ambit_training_program_decode(const uint8_t *data, uint32_t data_len, ambit_training_program_t *out)
{
    uint16_t items_count;
    uint16_t i;

    memset(out, 0, sizeof(*out));

    if (data_len < TRAINING_PROGRAM_HEADER_SIZE) {
        LOG_ERROR("TrainingProgram: region too short to contain a header");
        return -1;
    }

    out->base_year = rd_u16(data);
    out->base_month = data[2];
    out->base_day = data[3];
    memcpy(out->hr_zones, data + 4, 4);

    items_count = rd_u16(data + 8);
    if (items_count > TRAINING_PROGRAM_MAX_ITEMS) {
        /* An erased/never-written region reads back as 0xFFFF here - a normal, valid "no
         * program set" state, not a decode error. Any other implausible count is treated the
         * same way rather than guessed at further (see this file's own header comment). */
        LOG_INFO("TrainingProgram: item count %u looks like an empty/erased region, treating as no program", items_count);
        out->items_count = 0;
        return 0;
    }

    if (TRAINING_PROGRAM_HEADER_SIZE + (uint32_t)items_count * TRAINING_PROGRAM_ITEM_SIZE > data_len) {
        LOG_ERROR("TrainingProgram: %u items exceeds buffer (%u bytes)", items_count, data_len);
        memset(out, 0, sizeof(*out));
        return -1;
    }

    for (i = 0; i < items_count; i++) {
        const uint8_t *block = data + TRAINING_PROGRAM_HEADER_SIZE + (uint32_t)i * TRAINING_PROGRAM_ITEM_SIZE;
        ambit_training_program_item_t *item = &out->items[i];
        uint32_t name_len = 0;

        item->day_offset = block[0];
        item->completed = block[1] != 0;
        item->activity_id = rd_u16(block + 2);
        item->move_id = rd_u32(block + 4);
        item->distance = rd_u32(block + 8);
        item->duration_minutes = rd_u16(block + 12);
        item->intensity = block[14];

        while (name_len < TRAINING_PROGRAM_NAME_LEN && block[16 + name_len] != 0) {
            name_len++;
        }
        memcpy(item->name, block + 16, name_len);
        item->name[name_len] = '\0';
    }

    out->items_count = items_count;
    return 0;
}
