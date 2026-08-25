/*
 * (C) Copyright 2014 Emil Ljungdahl
 *
 * This file is part of libambit.
 *
 * libambit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 */
#include "device_driver.h"
#include "device_driver_common.h"
#include "device_support.h"
#include "libambit_int.h"
#include "protocol.h"
#include "pmem20.h"
#include "personal.h"
#include "sbem0102.h"
#include "sport_mode_serialize.h"
#include "custom_modes_bxml.h"
#include "apps_directory.h"
#include "utils.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Local definitions
 */

typedef struct memory_map_entry_s {
    uint32_t start;
    uint32_t size;
    uint8_t hash[32];
} memory_map_entry_t;

enum ambit3_fw_gen {
    AMBIT3_FW_GEN1,
    AMBIT3_FW_GEN2,
    AMBIT3_FW_GEN3,
    AMBIT3_FW_GEN4,
    AMBIT3_VERT_FW_GEN1,
    AMBIT3_VERT_FW_GEN2,
    AMBIT3_VERT_FW_GEN3,
    TRAVERSE_FW_GEN1,
    TRAVERSE_FW_GEN2,
};

typedef struct ambit3_driver_params_s {
    uint8_t mm_legacy_format;
    uint8_t mm_entry_data_id;
    uint8_t log_header_request_data_id;
    uint8_t log_entries_total_data_id;
    uint8_t log_entries_notsynced_data_id;
    uint8_t log_header_data_id;
    uint8_t log_header_tail_length;
    uint8_t log_synced_data_id;
} ambit3_driver_params_t;

struct ambit_device_driver_data_s {
    libambit_pmem20_t pmem20;
    libambit_sbem0102_t sbem0102;
    struct {
        uint8_t initialized;
        memory_map_entry_t waypoints;
        memory_map_entry_t routes;
        memory_map_entry_t rules;
        memory_map_entry_t gps;
        memory_map_entry_t sport_modes;
        memory_map_entry_t training_program;
        memory_map_entry_t exercise_log;
        memory_map_entry_t event_log;
        memory_map_entry_t ble_pairing;
        memory_map_entry_t apps;
        memory_map_entry_t glonass; // Ambit3 Vertical / Traverse
        memory_map_entry_t track_log; // Traverse
    } memory_maps;
    enum ambit3_fw_gen fw_gen;
    ambit3_driver_params_t driver_params;
};

typedef struct ambit3_log_header_s {
    ambit_log_header_t header;
    uint32_t address;
    uint32_t end_address;
    uint32_t address2;
    uint32_t end_address2;
    uint8_t synced;
} ambit3_log_header_t;

/*
 * Static functions
 */
static void init(ambit_object_t *object, uint32_t driver_param);
static void deinit(ambit_object_t *object);
static int personal_settings_get(ambit_object_t *object, ambit_personal_settings_t *settings);
static int log_read(ambit_object_t *object, ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref);
static int gps_orbit_header_read(ambit_object_t *object, uint8_t data[8]);
static int gps_orbit_write(ambit_object_t *object, uint8_t *data, size_t datalen);
static int sport_mode_write(ambit_object_t *object, ambit_sport_mode_device_settings_t *ambit_device_settings);
static int app_data_write(ambit_object_t *object, ambit_sport_mode_device_settings_t *ambit_device_settings, ambit_app_rules_t *ambit_apps);

static int parse_log_header_block(ambit_object_t *object, libambit_sbem0102_data_t *reply_data_object, ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref, uint16_t *log_entries_walked, uint16_t log_entries_total);
static size_t parse_log_entry(ambit_object_t *object, const uint8_t *log_data, ambit3_log_header_t *log_header);
static int get_memory_maps(ambit_object_t *object);
static int log_synced(ambit_object_t *object, ambit_log_entry_t *log_entry);
static int flash_read(ambit_object_t *object, uint32_t address, uint32_t length, uint8_t *buffer);
static int flash_write(ambit_object_t *object, uint32_t address, const uint8_t *data, uint32_t length, bool include_sha256_hash);
static int memory_map_get(ambit_object_t *object, ambit_memory_region_t *regions, int max_regions);

/*
 * Global variables
 */
ambit_device_driver_t ambit_device_driver_ambit3 = {
    init,
    deinit,
    libambit_device_driver_lock_log,
    libambit_device_driver_date_time_set,
    libambit_device_driver_status_get,
    personal_settings_get,
    log_read,
    gps_orbit_header_read,
    gps_orbit_write,
    NULL, // navigation_read
    NULL, // navigation_write
    sport_mode_write,
    app_data_write,
    log_synced,
    flash_read,
    flash_write,
    memory_map_get
};


/*
 * Static functions implementation
 */

/**
 * Performs a lookup of the ambit3 firmware generation for the watch product id and firmware version.
 *
 * \param device_info Device info structure of the watch.
 * \return Enumeration of the watch firmware generation.
 */
static enum ambit3_fw_gen get_ambit3_fw_gen(ambit_device_info_t *device_info)
{
    struct generation {
        uint8_t fw_version[4];
        enum ambit3_fw_gen gen;
    };

    struct generation ambit3_gen[] =  {
        {{2, 4, 1, 0}, AMBIT3_FW_GEN4},
        {{2, 2, 16, 0}, AMBIT3_FW_GEN3},
        {{2, 0, 4, 0}, AMBIT3_FW_GEN2},
        {{0, 0, 0, 0}, AMBIT3_FW_GEN1},
    };

    struct generation ambit3_vert_gen[] = {
        {{1, 1, 22, 0}, AMBIT3_VERT_FW_GEN3},
        {{1, 0, 27, 0}, AMBIT3_VERT_FW_GEN2},
        {{1, 0, 0, 0}, AMBIT3_VERT_FW_GEN1},
    };

    struct generation traverse_gen[] = {
        {{2, 0, 18, 0}, TRAVERSE_FW_GEN2},
        {{1, 0, 4, 0}, TRAVERSE_FW_GEN1},
    };

    struct generation* iter;

    switch (device_info->product_id) {
      case 0x1e:
      case 0x1c:
      case 0x1b:
        for (size_t i = 0; i < sizeof (ambit3_gen) / sizeof ((ambit3_gen)[0]) && (iter = &ambit3_gen[i]); i++) {
            if (libambit_fw_version_number(iter->fw_version) <= libambit_fw_version_number(device_info->fw_version))
                return iter->gen;
        }
        break;
      case 0x2c: // Ambit3 Vertical
        for (size_t i = 0; i < sizeof (ambit3_vert_gen) / sizeof ((ambit3_vert_gen)[0]) && (iter = &ambit3_vert_gen[i]); i++) {
            if (libambit_fw_version_number(iter->fw_version) <= libambit_fw_version_number(device_info->fw_version))
                return iter->gen;
        }
        break;
      case 0x2b: // Traverse
      case 0x2d: // Traverse Alpha
        for (size_t i = 0; i < sizeof (traverse_gen) / sizeof ((traverse_gen)[0]) && (iter = &traverse_gen[i]); i++) {
            if (libambit_fw_version_number(iter->fw_version) <= libambit_fw_version_number(device_info->fw_version))
                return iter->gen;
        }
        break;
    }

    abort();
}

/**
 * Performs a lookup of the ambit3 driver parameters for the watch firmware generation.
 *
 * \param fw_gen Firmware version of the watch.
 * \return ambit3_driver_params_t structure containing the driver parameters.
 */
static ambit3_driver_params_t get_ambit3_driver_params(enum ambit3_fw_gen fw_gen) {

    struct ambit3_driver_params_lookup {
        enum ambit3_fw_gen gen;
        ambit3_driver_params_t driver_params;
    };

    struct ambit3_driver_params_lookup dp_lookup[] =  {
            {AMBIT3_FW_GEN1,        {0x02, 0x3f, 0x81, 0x4e, 0x4f, 0x7e, 0x00, 0x00}},
            {AMBIT3_FW_GEN2,        {0x00, 0x4b, 0x8d, 0x5a, 0x5b, 0x8a, 0x1a, 0x00}},
            {AMBIT3_FW_GEN3,        {0x00, 0x4a, 0x8c, 0x59, 0x5a, 0x89, 0x1a, 0x00}},
            {AMBIT3_FW_GEN4,        {0x00, 0x4a, 0x8d, 0x59, 0x5a, 0x8a, 0x1c, 0x8b}},
            {AMBIT3_VERT_FW_GEN1,   {0x00, 0x4a, 0x8c, 0x59, 0x5a, 0x8a, 0x1c, 0x00}},
            {AMBIT3_VERT_FW_GEN2,   {0x00, 0x4a, 0x8b, 0x58, 0x59, 0x89, 0x1c, 0x00}},
            {AMBIT3_VERT_FW_GEN3,   {0x00, 0x4a, 0x8a, 0x57, 0x58, 0x88, 0x1c, 0x00}},
            {TRAVERSE_FW_GEN1,      {0x00, 0x4a, 0x8a, 0x56, 0x57, 0x87, 0x1c, 0x00}},
            {TRAVERSE_FW_GEN2,      {0x00, 0x4a, 0x88, 0x55, 0x56, 0x86, 0x1c, 0x00}},
    };

    ambit3_driver_params_t params;

    struct ambit3_driver_params_lookup* iter;

    for (size_t i = 0; i < sizeof (dp_lookup) / sizeof ((dp_lookup)[0]) && (iter = &dp_lookup[i]); i++) {
        if (iter->gen == fw_gen) {
            params = iter->driver_params;
            break;
        }
    }

    return params;
}

/**
 * Initialises the ambit3 driver.
 *
 * \param object
 * \param PMEM20 chunk size.
 */
static void init(ambit_object_t *object, uint32_t driver_param)
{
    struct ambit_device_driver_data_s *data;

    if ((data = calloc(1, sizeof(struct ambit_device_driver_data_s))) != NULL) {
        object->driver_data = data;
        libambit_pmem20_init(&object->driver_data->pmem20, object, driver_param);
        libambit_sbem0102_init(&object->driver_data->sbem0102, object, driver_param);

        // get fw generation specific parameters
        object->driver_data->fw_gen = get_ambit3_fw_gen(&object->device_info);
        object->driver_data->driver_params = get_ambit3_driver_params(object->driver_data->fw_gen);
    }
}

/**
 * De-initialises the ambit3 driver.
 *
 * \param object
 */
static void deinit(ambit_object_t *object)
{
    if (object->driver_data != NULL) {
        libambit_pmem20_deinit(&object->driver_data->pmem20);
        libambit_sbem0102_deinit(&object->driver_data->sbem0102);
    }
}

static float ieee754_to_float(uint32_t bits)
{
    int sign = bits >> 31 ? -1 : 1;
    int exp = (int)(((bits >> 23) & 0xff) - 127);
    int frac = (int)((bits & 0x7fffff) | 0x800000);

    return sign * frac * powf(2.0f, exp - 23);
}

/**
 * Gets the personal settings from the watch.
 *
 * \param object
 * \param settings Setting structure to populate.
 * \return 0 if successful.
 */
static int personal_settings_get(ambit_object_t *object, ambit_personal_settings_t *settings)
{
    uint8_t send_data[4] = { 0x00, 0x00, 0x00, 0x00 };
    libambit_sbem0102_data_t reply_data_object;
    uint32_t alarm_num;
    uint32_t decli_num;

    LOG_INFO("Reading personal settings");

    libambit_sbem0102_data_init(&reply_data_object);
    if (libambit_sbem0102_command_request_raw(&object->driver_data->sbem0102, ambit_command_ambit3_settings, send_data, sizeof(send_data), &reply_data_object) != 0) {
        LOG_WARNING("Failed to read personal settings");
        return -1;
    }

    memset(settings, 0, sizeof(ambit_personal_settings_t));

    while (libambit_sbem0102_data_next(&reply_data_object) == 0) {
        switch (libambit_sbem0102_data_id(&reply_data_object)) {
          case 0x01:
            settings->date_format = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x02:
            settings->tones_mode = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x03:
            settings->gps_position_format = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x04:
            decli_num = read32(libambit_sbem0102_data_ptr(&reply_data_object), 0);
            settings->compass_declination_f = ieee754_to_float(decli_num);
            break;
          case 0x08:
            settings->units_mode = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x12:
            settings->language = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x13: // Map orientation
            settings->navigation_style = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x14:
            settings->time_format = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x15:
            settings->sync_time_w_gps = !libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x16: // Dual time enabled
            break;
          case 0x17:
            settings->alarm_enable = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x18:
            alarm_num = read32(libambit_sbem0102_data_ptr(&reply_data_object), 0);
            settings->alarm.hour = (alarm_num / 60 / 60);
            settings->alarm.minute = (alarm_num / 60) % 60;
            break;
          case 0x19:
            settings->is_male = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x1a:
            settings->weight = read16(libambit_sbem0102_data_ptr(&reply_data_object), 0);
            break;
          case 0x1b:
            settings->max_hr = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x1f:
            if (libambit_sbem0102_data_len(&reply_data_object) == 11) {
                sscanf((const char*)libambit_sbem0102_data_ptr(&reply_data_object), "%04hu-", &settings->birthyear);
            }
            break;
          case 0x20: // Display contrast
            settings->display_brightness = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x21:
            settings->display_is_negative = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x22:
            settings->backlight_mode = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x23:
            settings->backlight_brightness = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x26:
            settings->alti_baro_mode = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x27:
            settings->fused_alti_disabled = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          case 0x28:
            settings->storm_alarm = libambit_sbem0102_data_ptr(&reply_data_object)[0];
            break;
          default:
            /*
            printf("Got id=%02x: ", libambit_sbem0102_data_id(&reply_data_object));
            switch(libambit_sbem0102_data_len(&reply_data_object)) {
              case 1:
                printf("%d", libambit_sbem0102_data_ptr(&reply_data_object)[0]);
                break;
              case 2:
                printf("%d", read16(libambit_sbem0102_data_ptr(&reply_data_object), 0));
                break;
              case 4:
                printf("%d", read32(libambit_sbem0102_data_ptr(&reply_data_object), 0));
                break;
              default:
                {
                    int q;
                for(q=0; q<libambit_sbem0102_data_len(&reply_data_object); q++)
                    printf("%02x", libambit_sbem0102_data_ptr(&reply_data_object)[q]);
                }
                break;
            }
            printf("\n");
            */
            break;
        }
    }

    return 0;
}

/**
 * Processes the log read replies for gen1 firmware. Gen1 has a log_header_data_id byte for each log header.
 *
 * \param object
 * \param reply_data_object Data received from the watch.
 * \param skip_cb Callback function when log header received.
 * \param push_cb Callback function when log received.
 * \param progress_cb Callback function when progress received.
 * \param userref
 * \return Number of entries read or -1 if error.
 */
static int process_log_read_replies_gen1(ambit_object_t *object, libambit_sbem0102_data_t *reply_data_object,
                                         ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref)
{
    ambit3_log_header_t log_header;
    ambit_log_entry_t *log_entry;

    int entries_read = 0;

    uint16_t log_entries_total = 0;
    uint16_t log_entries_walked = 0;
    uint16_t log_entries_notsynced;
    ONLYDEBUGVAR(log_entries_notsynced);

    log_header.header.activity_name = NULL;

    while (libambit_sbem0102_data_next(reply_data_object) == 0) {
        if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_entries_total_data_id) {
            log_entries_total = read16(libambit_sbem0102_data_ptr(reply_data_object), 0);
            LOG_INFO("Number of logs=%d", log_entries_total);
        }
        else if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_entries_notsynced_data_id) {
            log_entries_notsynced = read16(libambit_sbem0102_data_ptr(reply_data_object), 0);
            LOG_INFO("Number of logs marked as not synchronized=%d", log_entries_notsynced);
        }
        else if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_header_data_id) {
            const uint8_t *data = libambit_sbem0102_data_ptr(reply_data_object);

            if(parse_log_entry(object, data, &log_header) != 0) {
                LOG_INFO("Log header parsed successfully");
                if (!skip_cb || skip_cb(userref, &log_header.header) != 0) {
                    LOG_INFO("Reading data of log %d of %d", log_entries_walked + 1, log_entries_total);
                    log_entry = libambit_pmem20_log_read_entry_address(&object->driver_data->pmem20,
                                                                       log_header.address,
                                                                       log_header.end_address - log_header.address,
                                                                       0, 0,
                                                                       LIBAMBIT_PMEM20_FLAGS_NONE);
                    if (log_entry != NULL) {
                        if (push_cb != NULL) {
                            push_cb(userref, log_entry);
                        }
                        entries_read++;

                        libambit_log_entry_free(log_entry);
                    }
                }
                else {
                    LOG_INFO("Log entry already exists, skipping");
                }
            }
            else {
                LOG_INFO("Failed to parse log header");
            }
            log_entries_walked++;
            if (progress_cb != NULL && log_entries_total != 0) {
                progress_cb(userref, log_entries_total, log_entries_walked, 100*log_entries_walked/log_entries_total);
            }
        }
        else {
            LOG_INFO("Unknown data id 0x%x", libambit_sbem0102_data_id(reply_data_object));
        }
    }
    
    return entries_read;
}

/**
 * Processes the log read replies for all firmware except gen1. There is a log_header_data_id byte for a blick
 * of log headers.
 *
 * \param object
 * \param reply_data_object Data received from the watch.
 * \param skip_cb Callback function when log header received.
 * \param push_cb Callback function when log received.
 * \param progress_cb Callback function when progress received.
 * \param userref
 * \return Number of entries read or -1 if error.
 */
static int process_log_read_replies(ambit_object_t *object, libambit_sbem0102_data_t *reply_data_object,
                                         ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref)
{
    uint16_t log_entries_total = 0;
    uint16_t log_entries_notsynced;
    uint16_t log_entries_walked = 0;
    int entries_read = 0;
    ONLYDEBUGVAR(log_entries_notsynced);

    while (libambit_sbem0102_data_next(reply_data_object) == 0) {
        if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_entries_total_data_id) {
            log_entries_total = read16(libambit_sbem0102_data_ptr(reply_data_object), 0);
        }
        else if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_entries_notsynced_data_id) {
            log_entries_notsynced = read16(libambit_sbem0102_data_ptr(reply_data_object), 0);
            LOG_INFO("Number of logs marked as not synchronized=%d", log_entries_notsynced);
        }
        else if (libambit_sbem0102_data_id(reply_data_object) == object->driver_data->driver_params.log_header_data_id) {
            entries_read = parse_log_header_block(object, reply_data_object, skip_cb, push_cb, progress_cb, userref, &log_entries_walked, log_entries_total);
        }
        else {
            LOG_INFO("Unknown data id 0x%x", libambit_sbem0102_data_id(reply_data_object));
        }
    }

    return entries_read;
}

/**
 * Gets the logs from the watch.
 *
 * \param skip_cb Callback function when log header received.
 * \param push_cb Callback function when log received.
 * \param progress_cb Callback function when progress received.
 * \param userref
 * \return Number of entries read or -1 if error.
 */
static int log_read(ambit_object_t *object, ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref)
{
    int entries_read = 0;
    libambit_sbem0102_data_t send_data_object, reply_data_object;
    LOG_INFO("Reading log headers");

    libambit_sbem0102_data_init(&reply_data_object);
    libambit_sbem0102_data_init(&send_data_object);
    libambit_sbem0102_data_add(&send_data_object, object->driver_data->driver_params.log_header_request_data_id, NULL, 0);

    if (libambit_sbem0102_command_request(&object->driver_data->sbem0102, ambit_command_ambit3_log_headers, &send_data_object, &reply_data_object) != 0) {
        LOG_WARNING("Failed to read log headers");
        return -1;
    }

    if (object->driver_data->memory_maps.initialized == 0) {
        if (get_memory_maps(object) != 0) {
            return -1;
        }
    }

    // Initialize PMEM20 log before starting to read logs
    libambit_pmem20_log_init(&object->driver_data->pmem20, object->driver_data->memory_maps.exercise_log.start, object->driver_data->memory_maps.exercise_log.size);

    if (object->driver_data->fw_gen == AMBIT3_FW_GEN1) {
        entries_read = process_log_read_replies_gen1(object, &reply_data_object, skip_cb, push_cb, progress_cb, userref);
    }
    else {
        entries_read = process_log_read_replies(object, &reply_data_object, skip_cb, push_cb, progress_cb, userref);
    }

    printf("Finished reading logs... I think...\n");

    libambit_sbem0102_data_free(&send_data_object);
    printf("Finished freeing data 1\n");
    libambit_sbem0102_data_free(&reply_data_object);

    printf("Finished freeing data 2\n");

    return entries_read;
}

/**
 * Gets the gps orbit header from the watch.
 *
 * \param object
 * \param data Buffer to populate with gps orbit data.
 * \return 0 if successful.
 */
static int gps_orbit_header_read(ambit_object_t *object, uint8_t data[8])
{
    uint8_t *reply_data = NULL;
    size_t replylen = 0;
    int ret = -1;

    if (libambit_protocol_command(object, ambit_command_gps_orbit_head, NULL, 0, &reply_data, &replylen, 0) == 0 && replylen >= 9) {
        memcpy(data, &reply_data[1], 8);
        libambit_protocol_free(reply_data);

        ret = 0;
    }
    else {
        LOG_WARNING("Failed to read GPS orbit header");
    }

    return ret;
}

/**
 * Writes the gps orbit header to the watch.
 *
 * \param object
 * \param data Buffer containing gps orbit data.
 * \param datalen Size of gps orbit data buiffer.
 * \return 0 if successful.
 */
static int gps_orbit_write(ambit_object_t *object, uint8_t *data, size_t datalen)
{
    uint8_t header[8], cmpheader[8];
    int ret = -1;

    LOG_INFO("Writing GPS orbit data");

    libambit_protocol_command(object, ambit_command_write_start, NULL, 0, NULL, NULL, 0);

    if (object->driver->gps_orbit_header_read(object, header) == 0) {
        cmpheader[0] = data[7]; // Year, swap bytes
        cmpheader[1] = data[6];
        cmpheader[2] = data[8];
        cmpheader[3] = data[9];
        cmpheader[4] = data[13]; // 4 byte swap
        cmpheader[5] = data[12];
        cmpheader[6] = data[11];
        cmpheader[7] = data[10];

        // Check if new data differs 
        if (memcmp(header, cmpheader, 8) != 0) {
            ret = libambit_pmem20_gps_orbit_write(&object->driver_data->pmem20, data, datalen, true);
        }
        else {
            LOG_INFO("Current GPS orbit data is already up to date, skipping");
            ret = 0;
        }
    }

    return ret;
}

/**
 * Writes sport mode (CustomModes) configuration to an Ambit3-family device.
 *
 * Unlike the legacy Ambit/Ambit2 driver, Ambit3 does not keep the CustomModes
 * region at a fixed PMEM20 offset; the address is discovered per-device via
 * the memory-map command (get_memory_maps()) and the write is validated by
 * the firmware against a SHA256 hash of the written bytes.
 *
 * \param object
 * \param ambit_device_settings Sport mode settings to serialize and write.
 * \return 0 on success, negative on failure.
 */
/**
 * Copies a NUL-safe string of up to `max` bytes from `src` into `dst`, which must have
 * room for max+1 bytes (dst is always left NUL-terminated).
 */
static void copy_bounded_name(char *dst, const char *src, size_t max)
{
    size_t n = 0;
    while (n < max && src[n] != '\0') {
        n++;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/**
 * Maps the legacy (Ambit1/Ambit2-era) per-mode settings scalars onto the real Ambit3
 * BXml settings block. Field-by-field provenance:
 *
 * - name/activity_id/recording_interval/autolap/alti_baro_mode/auto_pause/auto_scroll(ing):
 *   direct 1:1 renames, both formats store the same value.
 * - use_hw <- hrbelt_and_pods, gps_power_mode <- gps_interval, hr_high/low <- heartrate_max/min,
 *   hr_limits_use <- use_heartrate_limits, int_timer_flags <- use_interval_timer,
 *   int_timer_count <- interval_repetitions: same concept under a renamed field, best-effort
 *   but high-confidence direct copies.
 * - interval_slots[5] (Flags/MaxLimit/MinLimit) <- backlight_mode/display_mode/quick_navigation:
 *   CONFIRMED mapping (not a guess) - this project's custom_modes_bxml.c and the sibling
 *   sommet project both independently found the watch repurposes the otherwise-unused 6th
 *   interval-timer slot to carry these three settings.
 * - interval_slots[0..4] (the actual interval-timer configuration) and auto_start: the
 *   legacy struct's interval_timer_max/min/unit fields do not have a confirmed mapping onto
 *   the new slot format's Type/MaxLimit/MinLimit/Len encoding, so they are left at zero
 *   (interval timer off) here rather than guessed. custom_mode_id is left to the caller
 *   (see sport_mode_write()): 0 when creating a brand new mode's id, untouched when updating
 *   an existing one.
 */
static void convert_legacy_settings_to_bxml(const ambit_sport_mode_settings_t *legacy, ambit_custom_mode_settings_t *bxml)
{
    memset(bxml, 0, sizeof(*bxml));
    copy_bounded_name(bxml->name, legacy->activity_name, sizeof(legacy->activity_name));
    bxml->activity_id       = legacy->activity_id;
    bxml->use_hw             = legacy->hrbelt_and_pods;
    bxml->alti_baro_mode     = legacy->alti_baro_mode;
    bxml->gps_power_mode     = legacy->gps_interval;
    bxml->recording_interval = legacy->recording_interval;
    bxml->autolap            = legacy->autolap;
    bxml->hr_high             = legacy->heartrate_max;
    bxml->hr_low              = legacy->heartrate_min;
    bxml->hr_limits_use       = legacy->use_heartrate_limits;
    bxml->auto_pause          = legacy->auto_pause;
    bxml->auto_scrolling      = legacy->auto_scroll;
    bxml->int_timer_flags     = legacy->use_interval_timer;
    bxml->int_timer_count     = legacy->interval_repetitions;

    bxml->interval_slots[5].flags     = (uint8_t)legacy->backlight_mode;
    bxml->interval_slots[5].max_limit = legacy->display_mode;
    bxml->interval_slots[5].min_limit = legacy->quick_navigation;
}

/**
 * Builds a brand-new BXml display list from the legacy per-mode display array (row1/row2/row3
 * + views), for use ONLY when a mode has no existing on-device match to preserve displays
 * from (see sport_mode_write()). EXPERIMENTAL / not hardware-confirmed: real Ambit3 firmware
 * uses several richer display templates (observed live: 0x0111/0x0122/0x0123/0x0127/0x0150,
 * fixed system screens present on every real mode) that this legacy-era struct has no concept
 * of at all, and the exact semantics of the BXml field "Type" value (as opposed to "Index")
 * are not fully pinned down even in the sibling sommet project's own reverse-engineering notes.
 * This function reuses the legacy row/view "item" values directly as the new field's Index
 * (both formats appear to use the same underlying FT_* field-catalogue numbering) and leaves
 * Type at 0, which is what a 0-shortcut slot commonly shows on real hardware, but this has NOT
 * been round-trip verified against real firmware the way the rest of custom_modes_bxml.c has.
 */
static void convert_legacy_displays_to_bxml(const ambit_sport_mode_t *legacy_mode, ambit_custom_mode_t *bxml_mode)
{
    int i;

    bxml_mode->displays_count = 0;
    for (i = 0; i < legacy_mode->displays_count && bxml_mode->displays_count < CUSTOM_MODES_BXML_MAX_DISPLAYS; i++) {
        const ambit_sport_mode_display_t *legacy_disp = &legacy_mode->display[i];
        ambit_custom_mode_display_t *bxml_disp = &bxml_mode->displays[bxml_mode->displays_count];
        int field_count = 0;

        memset(bxml_disp, 0, sizeof(*bxml_disp));
        bxml_disp->template_id = legacy_disp->type; /* both formats use the same SINGLE/DOUBLE/TRIPLE/GRAPH_DISPLAY_TYPE numbering */
        bxml_disp->type = 10; /* observed constant on every real user-editable display of this shape */

        if (field_count < CUSTOM_MODES_BXML_MAX_FIELDS) {
            bxml_disp->fields[field_count].index = legacy_disp->row1;
            field_count++;
        }
        if (legacy_disp->type != SINGLE_ROW_DISPLAY_TYPE && field_count < CUSTOM_MODES_BXML_MAX_FIELDS) {
            bxml_disp->fields[field_count].index = legacy_disp->row2;
            field_count++;
        }
        if ((legacy_disp->type == TRIPLE_ROWS_DISPLAY_TYPE || legacy_disp->type == GRAPH_DISPLAY_TYPE) && field_count < CUSTOM_MODES_BXML_MAX_FIELDS) {
            bxml_disp->fields[field_count].index = legacy_disp->row3;
            field_count++;
        }
        bxml_disp->fields_count = (uint16_t)field_count;

        bxml_mode->displays_count++;
    }
}

/**
 * Finds the index within `region->modes` whose custom_mode_id matches `legacy_id` in its
 * low 16 bits (the legacy struct only carries a 16-bit sport_mode_id). Returns -1 if none.
 */
static int find_matching_bxml_mode(const ambit_custom_modes_region_t *region, uint16_t legacy_id)
{
    int i;
    for (i = 0; i < region->modes_count; i++) {
        if ((region->modes[i].settings.custom_mode_id & 0xffff) == legacy_id) {
            return i;
        }
    }
    return -1;
}

/**
 * Writes sport mode / CustomModes data to an Ambit3-family device.
 *
 * Real Ambit3 firmware (confirmed live, 2026-08-25) uses a materially different on-flash
 * format for CustomModes than the legacy Ambit1/Ambit2 format sport_mode_serialize.c
 * implements - see custom_modes_bxml.h for the full story and provenance. This function
 * bridges the old libambit_sport_mode_write() API (still used throughout the GUI/CLI/
 * Movescount-JSON path, all built around the legacy ambit_sport_mode_device_settings_t
 * struct) onto that real format via a read-modify-write:
 *
 *  1. Read the device's current CustomModes region and decode it.
 *  2. For each mode the caller supplied: if a mode with the same id already exists on the
 *     device, update ONLY its settings scalars in place and leave its existing displays/
 *     rules/app-meta completely untouched - this preserves richer display templates already
 *     on the watch that the legacy struct has no way to represent (see
 *     convert_legacy_displays_to_bxml()'s own doc comment). If the mode is genuinely new,
 *     append it with best-effort (experimental, not hardware-verified) displays built from
 *     the caller's row/view data.
 *  3. If the caller supplied sport_mode_groups, rebuild the device's multisport slot list
 *     from them (matching REPLACE semantics already expected of this API); otherwise leave
 *     the device's existing multisport slots untouched.
 *  4. Encode and write the result back, still hash-validated like the pre-BXml write path.
 */
static int sport_mode_write(ambit_object_t *object, ambit_sport_mode_device_settings_t *ambit_device_settings)
{
    ambit_custom_modes_region_t region;
    uint8_t *live, *encoded;
    uint32_t region_size, encoded_len;
    uint32_t max_custom_mode_id;
    int mode_index_map[64]; /* ambit_device_settings->sport_modes_count is caller-controlled but realistically small */
    int i, ret;

    LOG_INFO("Writing Custom mode data (Ambit3)");

    if (object->driver_data->memory_maps.initialized == 0 && get_memory_maps(object) != 0) {
        LOG_WARNING("Failed to read memory map, cannot locate CustomModes region");
        return -1;
    }

    region_size = object->driver_data->memory_maps.sport_modes.size;
    if (region_size == 0) {
        LOG_WARNING("Device did not report a CustomModes memory region");
        return -1;
    }

    if (ambit_device_settings->sport_modes_count > 64) {
        LOG_ERROR("sport_mode_write: %u sport modes exceeds this function's internal limit of 64", ambit_device_settings->sport_modes_count);
        return -1;
    }

    live = (uint8_t*)malloc(region_size);
    if (live == NULL) {
        LOG_ERROR("Could not allocate memory for sport_mode_write");
        return -1;
    }

    if (libambit_pmem20_flash_read(&object->driver_data->pmem20, object->driver_data->memory_maps.sport_modes.start, region_size, live) != 0) {
        LOG_ERROR("sport_mode_write: failed to read current CustomModes region");
        free(live);
        return -1;
    }

    if (ambit_custom_modes_decode(live, region_size, &region) != 0) {
        LOG_ERROR("sport_mode_write: failed to decode current CustomModes region, refusing to write blind");
        free(live);
        return -1;
    }
    free(live);

    max_custom_mode_id = 0;
    for (i = 0; i < region.modes_count; i++) {
        if (region.modes[i].settings.custom_mode_id > max_custom_mode_id) {
            max_custom_mode_id = region.modes[i].settings.custom_mode_id;
        }
    }

    for (i = 0; i < (int)ambit_device_settings->sport_modes_count; i++) {
        ambit_sport_mode_t *legacy_mode = &ambit_device_settings->sport_modes[i];
        int existing_idx = find_matching_bxml_mode(&region, legacy_mode->settings.sport_mode_id);

        if (existing_idx >= 0) {
            uint32_t keep_id = region.modes[existing_idx].settings.custom_mode_id;
            LOG_INFO("sport_mode_write: updating settings for existing mode \"%s\" (id %u), preserving its displays",
                      region.modes[existing_idx].settings.name, legacy_mode->settings.sport_mode_id);
            convert_legacy_settings_to_bxml(&legacy_mode->settings, &region.modes[existing_idx].settings);
            region.modes[existing_idx].settings.custom_mode_id = keep_id; /* convert_legacy_settings_to_bxml() zeroes the struct first */
            mode_index_map[i] = existing_idx;
        } else {
            if (region.modes_count >= CUSTOM_MODES_BXML_MAX_MODES) {
                LOG_ERROR("sport_mode_write: no room for new mode \"%s\", CustomModes already has the max %d modes this driver supports",
                           legacy_mode->settings.activity_name, CUSTOM_MODES_BXML_MAX_MODES);
                return -1;
            }
            LOG_WARNING("sport_mode_write: creating NEW mode \"%s\" with experimental/unverified display data - see convert_legacy_displays_to_bxml()",
                         legacy_mode->settings.activity_name);
            int new_idx = region.modes_count;
            convert_legacy_settings_to_bxml(&legacy_mode->settings, &region.modes[new_idx].settings);
            max_custom_mode_id++;
            region.modes[new_idx].settings.custom_mode_id = max_custom_mode_id;
            convert_legacy_displays_to_bxml(legacy_mode, &region.modes[new_idx]);
            region.modes[new_idx].rules_count = 0;
            region.modes[new_idx].has_app_meta = false;
            region.modes_count++;
            mode_index_map[i] = new_idx;
        }
    }

    if (ambit_device_settings->sport_mode_groups_count > 0) {
        region.sport_modes_count = 0;
        for (i = 0; i < (int)ambit_device_settings->sport_mode_groups_count && region.sport_modes_count < CUSTOM_MODES_BXML_MAX_SPORT_MODES; i++) {
            ambit_sport_mode_group_t *group = &ambit_device_settings->sport_mode_groups[i];
            ambit_multisport_slot_t *slot = &region.sport_modes[region.sport_modes_count];
            uint32_t j;

            memset(slot, 0, sizeof(*slot));
            copy_bounded_name(slot->name, group->activity_name, sizeof(group->activity_name));
            slot->activity_id = group->activity_id;
            slot->exercises_count = 0;
            for (j = 0; j < group->sport_mode_index_count && slot->exercises_count < CUSTOM_MODES_BXML_MAX_EXERCISES; j++) {
                uint16_t legacy_pos = group->sport_mode_index[j];
                if (legacy_pos < ambit_device_settings->sport_modes_count) {
                    slot->exercises[slot->exercises_count++] = (uint16_t)mode_index_map[legacy_pos];
                } else {
                    LOG_WARNING("sport_mode_write: group \"%s\" references out-of-range mode position %u, skipping", group->activity_name, legacy_pos);
                }
            }
            region.sport_modes_count++;
        }
    }

    encoded = (uint8_t*)malloc(region_size);
    if (encoded == NULL) {
        LOG_ERROR("Could not allocate memory for sport_mode_write");
        return -1;
    }

    if (ambit_custom_modes_encode(&region, encoded, region_size, &encoded_len) != 0) {
        LOG_ERROR("sport_mode_write: encoded CustomModes region does not fit in the device's %u-byte region", region_size);
        free(encoded);
        return -1;
    }

    ret = libambit_pmem20_data_write_addr(&object->driver_data->pmem20,
                                           object->driver_data->memory_maps.sport_modes.start,
                                           encoded, encoded_len, true);

    free(encoded);

    return ret;
}

/**
 * Writes App (Suunto Apps / rules) data to an Ambit3-family device.
 *
 * See sport_mode_write() above for why this differs from the legacy driver's
 * fixed-address version. EXPERIMENTAL: the write is firmware-accepted (the
 * region hash matches after write), but whether the watch executes a freshly
 * written app has not been confirmed on real hardware.
 *
 * \param object
 * \param ambit_device_settings Sport mode settings the apps are attached to.
 * \param ambit_apps App rule binaries to serialize and write.
 * \return 0 on success, negative on failure.
 */
/**
 * Finds an existing directory entry whose binary is byte-identical to (\a binary, \a
 * binary_length). Binary content is the only stable identity apps_directory.h's real format
 * offers - see app_data_write()'s own doc comment for why.
 */
static int find_matching_apps_entry_by_binary(const ambit_apps_dir_region_t *region, const uint8_t *binary, uint32_t binary_length)
{
    int i;
    for (i = 0; i < region->entries_count; i++) {
        if (region->entries[i].binary_length == binary_length &&
            memcmp(region->entries[i].binary, binary, binary_length) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Writes App (Suunto Apps / rules) data to an Ambit3-family device, using the real on-device
 * Apps directory format (apps_directory.h) via a read-modify-write - see sport_mode_write()'s
 * own doc comment for the parallel CustomModes story; this is the same fix for the Apps
 * region, closing the gap flagged in this project's training-plans plan (Phase 1's
 * app_data_write() previously used the older, Ambit1/Ambit2-era serialize_app_data() format,
 * unverified - and now known-mismatched - against real Ambit3 firmware).
 *
 * KNOWN GAP: the legacy ambit_app_rule_t/ambit_sport_mode_device_settings_t API this function
 * still takes (unchanged, so every existing caller keeps working) has no name or activityId
 * field per app at all - only an opaque caller-assigned app_id and the raw compiled bytecode.
 * The real directory format needs both. So an app already present on the device (matched here
 * by exact binary content, the only stable identity available) keeps its real on-device name/
 * activityId untouched; a genuinely new app gets a synthesized placeholder name ("App <id>")
 * and activityId 0, which is what will actually show on the watch until a future API revision
 * threads real name/activityId through from the GUI/Movescount-JSON layer.
 */
static int app_data_write(ambit_object_t *object, ambit_sport_mode_device_settings_t *ambit_device_settings, ambit_app_rules_t *ambit_apps)
{
    ambit_apps_dir_region_t region;
    uint8_t *live, *encoded;
    uint32_t region_size, encoded_len;
    uint32_t i, j;
    int ret;

    LOG_INFO("Writing App data (Ambit3)");

    if (object->driver_data->memory_maps.initialized == 0 && get_memory_maps(object) != 0) {
        LOG_WARNING("Failed to read memory map, cannot locate Apps region");
        return -1;
    }

    region_size = object->driver_data->memory_maps.apps.size;
    if (region_size == 0) {
        LOG_WARNING("Device did not report an Apps memory region");
        return -1;
    }

    if (ambit_device_settings->app_ids_count == 0) {
        return 0;
    }

    live = (uint8_t*)malloc(region_size);
    if (live == NULL) {
        LOG_ERROR("Could not allocate memory for app_data_write");
        return -1;
    }

    if (libambit_pmem20_flash_read(&object->driver_data->pmem20, object->driver_data->memory_maps.apps.start, region_size, live) != 0) {
        LOG_ERROR("app_data_write: failed to read current Apps region");
        free(live);
        return -1;
    }

    if (ambit_apps_dir_decode(live, region_size, &region) != 0) {
        LOG_INFO("app_data_write: no valid existing Apps directory found, starting from an empty one");
        memset(&region, 0, sizeof(region));
    }
    free(live);

    for (i = 0; i < ambit_device_settings->app_ids_count; i++) {
        uint32_t app_id = ambit_device_settings->app_ids[i];
        int app_rule_index = -1;
        const uint8_t *binary;
        uint32_t binary_length;
        int existing_idx;

        for (j = 0; j < ambit_apps->app_rules_count; j++) {
            if (ambit_apps->app_rules[j].app_id == app_id) {
                app_rule_index = (int)j;
                break;
            }
        }
        if (app_rule_index < 0) {
            LOG_ERROR("app_data_write: no app rule data found for app_id %u", app_id);
            return -1;
        }

        binary = ambit_apps->app_rules[app_rule_index].app_rule_data;
        binary_length = ambit_apps->app_rules[app_rule_index].app_rule_data_length;

        existing_idx = find_matching_apps_entry_by_binary(&region, binary, binary_length);
        if (existing_idx >= 0) {
            LOG_INFO("app_data_write: app_id %u already present on device (matched by content), keeping its existing name/activityId", app_id);
            continue;
        }

        if (region.entries_count >= APPS_DIR_MAX_ENTRIES) {
            LOG_ERROR("app_data_write: no room for app_id %u, Apps directory already has the max %d entries this driver supports",
                       app_id, APPS_DIR_MAX_ENTRIES);
            return -1;
        }

        {
            ambit_apps_dir_entry_t *entry = &region.entries[region.entries_count];
            memset(entry, 0, sizeof(*entry));
            snprintf(entry->name, sizeof(entry->name), "App %u", app_id);
            entry->activity_id = 0;
            entry->binary = (uint8_t*)binary;
            entry->binary_length = binary_length;
            region.entries_count++;
        }
    }

    encoded = (uint8_t*)malloc(region_size);
    if (encoded == NULL) {
        LOG_ERROR("Could not allocate memory for app_data_write");
        return -1;
    }

    if (ambit_apps_dir_encode(&region, encoded, region_size, &encoded_len) != 0) {
        LOG_ERROR("app_data_write: encoded Apps directory does not fit in the device's %u-byte region", region_size);
        free(encoded);
        return -1;
    }

    ret = libambit_pmem20_data_write_addr(&object->driver_data->pmem20,
                                           object->driver_data->memory_maps.apps.start,
                                           encoded, encoded_len, true);

    free(encoded);

    return ret;
}

/**
 * Reads an arbitrary flash region back from an Ambit3-family device, for
 * verifying a write (or inspecting Apps/CustomModes/TrainingProgram) without
 * needing a driver-specific higher-level parser.
 *
 * \param object
 * \param address Flash start address (e.g. from get_memory_maps()).
 * \param length Number of bytes to read.
 * \param buffer Caller-allocated buffer of at least `length` bytes.
 * \return 0 on success, negative on failure.
 */
static int flash_read(ambit_object_t *object, uint32_t address, uint32_t length, uint8_t *buffer)
{
    return libambit_pmem20_flash_read(&object->driver_data->pmem20, address, length, buffer);
}

/**
 * Writes raw bytes directly to a flash address. Low-level primitive - see the
 * libambit_flash_write() doc comment in libambit.h for the caveats (caller owns region
 * addressing and content validity).
 */
static int flash_write(ambit_object_t *object, uint32_t address, const uint8_t *data, uint32_t length, bool include_sha256_hash)
{
    return libambit_pmem20_data_write_addr(&object->driver_data->pmem20, address, data, length, include_sha256_hash);
}

/**
 * Fills `regions` with the device's live memory map (name/start/size for
 * every region this driver recognizes), triggering a memory-map read first
 * if not already cached. Lets callers (diagnostics, test tools) work off the
 * addresses this specific device/firmware actually reports, instead of
 * assumed constants.
 */
static int memory_map_get(ambit_object_t *object, ambit_memory_region_t *regions, int max_regions)
{
    int count = 0;
    struct { const char *name; memory_map_entry_t *entry; } table[] = {
        { "Waypoints",       &object->driver_data->memory_maps.waypoints },
        { "Routes",          &object->driver_data->memory_maps.routes },
        { "Rules",           &object->driver_data->memory_maps.rules },
        { "GpsSGEE",         &object->driver_data->memory_maps.gps },
        { "CustomModes",     &object->driver_data->memory_maps.sport_modes },
        { "TrainingProgram", &object->driver_data->memory_maps.training_program },
        { "ExerciseLog",     &object->driver_data->memory_maps.exercise_log },
        { "EventLog",        &object->driver_data->memory_maps.event_log },
        { "BlePairingInfo",  &object->driver_data->memory_maps.ble_pairing },
        { "Apps",            &object->driver_data->memory_maps.apps },
    };
    size_t i;

    if (object->driver_data->memory_maps.initialized == 0 && get_memory_maps(object) != 0) {
        LOG_WARNING("Failed to read memory map");
        return -1;
    }

    for (i = 0; i < sizeof(table)/sizeof(table[0]) && count < max_regions; i++) {
        if (table[i].entry->size == 0) {
            continue; // Not reported by this device/firmware
        }
        strncpy(regions[count].name, table[i].name, sizeof(regions[count].name) - 1);
        regions[count].name[sizeof(regions[count].name) - 1] = '\0';
        regions[count].start = table[i].entry->start;
        regions[count].size = table[i].entry->size;
        count++;
    }

    return count;
}

/**
 * Processes a block of log headers (not gen1).
 *
 * \param object
 * \param reply_data_object Data received from the watch.
 * \param skip_cb Callback function when log header received.
 * \param push_cb Callback function when log received.
 * \param progress_cb Callback function when progress received.
 * \param userref
 * \param log_entries_walked Log entries walked.
 * \param log_entries_total Log entries total.
 * \return Number of entries read in this block or -1 if error.
 */
static int parse_log_header_block(ambit_object_t *object, libambit_sbem0102_data_t *reply_data_object, ambit_log_skip_cb skip_cb, ambit_log_push_cb push_cb, ambit_log_progress_cb progress_cb, void *userref,  uint16_t *log_entries_walked, uint16_t log_entries_total)
{
    ambit3_log_header_t log_header;
    ambit_log_entry_t *log_entry;
    const uint8_t *data;
    size_t length = 0;
    size_t offset = 0;
    size_t log_read_len = 0;
    int current_parse_num_log_read = 0;
    int skip;
    
    length = libambit_sbem0102_data_len(reply_data_object);
    data = libambit_sbem0102_data_ptr(reply_data_object);

    while(offset<length) {
        log_header.header.activity_name = NULL;
        log_read_len = parse_log_entry(object, &data[offset], &log_header);

        if(log_read_len == 0) {
            LOG_ERROR("Could not parse log header");
            return -1;
        }

        offset += log_read_len;

        LOG_INFO ("Next offset: %d of %d\n", offset, length);

        if (skip_cb && !skip_cb(userref, &log_header.header)) {
            skip = 1;
        }
        else {
            skip = 0;
        }
        
        if (skip && !log_header.synced) {
            LOG_INFO("Log not previously synchronized, force update");
            skip = 0;
        }
        
        if (!skip) {
            LOG_INFO("Reading data of log %d of %d", *log_entries_walked + 1, log_entries_total);
            log_entry = libambit_pmem20_log_read_entry_address(&object->driver_data->pmem20,
                                                               log_header.address,
                                                               log_header.end_address - log_header.address,
                                                               log_header.address2,
                                                               log_header.end_address2 - log_header.address2,
                                                               LIBAMBIT_PMEM20_FLAGS_UNKNOWN2_PADDING_48);
            LOG_INFO("Completed data of log %d of %d", *log_entries_walked + 1, log_entries_total);
            if (log_entry != NULL) {
                if (push_cb != NULL) {
                    push_cb(userref, log_entry);
                    LOG_INFO("Completed push_cb");
                }
            }
        }
        else {
            LOG_INFO("Log entry already exists, skipping");
        }

        (*log_entries_walked)++;
        current_parse_num_log_read++;

        if(*log_entries_walked > log_entries_total) {
            log_entries_total = *log_entries_walked; // Handle situations where ambit reports wrong number of total entries
        }

        if (progress_cb != NULL && log_entries_total != 0) {
            LOG_INFO("Do progress_cb");
            progress_cb(userref, log_entries_total, *log_entries_walked, 100*(*log_entries_walked)/log_entries_total);
        }
    }

    return current_parse_num_log_read;
}

/**
 * Parses the log header data for one log entry..
 *
 * \param data Raw log header data buffer.
 * \param log_header Structure to populate with header field values.
 * \return Buffer offset at the end of the log header.
 */
static size_t parse_log_entry(ambit_object_t *object, const uint8_t *data, ambit3_log_header_t *log_header)
{
    struct tm tm;
    char *ptr;
    size_t offset = 0;

    // Start with parsing the time
    if ((ptr = libambit_strptime((const char *)data, "%Y-%m-%dT%H:%M:%S", &tm)) == NULL) {
        return 0;
    }

    log_header->header.date_time.year = 1900 + tm.tm_year;
    log_header->header.date_time.month = tm.tm_mon + 1;
    log_header->header.date_time.day = tm.tm_mday;
    log_header->header.date_time.hour = tm.tm_hour;
    log_header->header.date_time.minute = tm.tm_min;
    log_header->header.date_time.msec = tm.tm_sec*1000;
    offset += (size_t)ptr - (size_t)data + 1;

    log_header->synced = read8inc(data, &offset);

    log_header->address = read32inc(data, &offset);
    log_header->end_address = read32inc(data, &offset);
    log_header->address2 = read32inc(data, &offset);
    log_header->end_address2 = read32inc(data, &offset);
    log_header->header.heartrate_min = read8inc(data, &offset);
    log_header->header.heartrate_avg = read8inc(data, &offset);
    log_header->header.heartrate_max = read8inc(data, &offset);

    if (object->driver_data->fw_gen == AMBIT3_FW_GEN1 || object->driver_data->fw_gen == AMBIT3_FW_GEN2) {
        log_header->header.heartrate_max_time = read32inc(data, &offset);
        log_header->header.heartrate_min_time = read32inc(data, &offset);
    }
    else {
        log_header->header.heartrate_min_time = read32inc(data, &offset);
        log_header->header.heartrate_max_time = read32inc(data, &offset);
    }

    // temperature format is messed up, 1 byte is missing, just skip for now
    log_header->header.temperature_min = 0;
    log_header->header.temperature_max = 0;
    offset += 2;
    log_header->header.temperature_min_time = read32inc(data, &offset);
    log_header->header.temperature_max_time = read32inc(data, &offset);
    log_header->header.altitude_min = read16inc(data, &offset);
    log_header->header.altitude_max = read16inc(data, &offset);
    log_header->header.altitude_min_time = read32inc(data, &offset);
    log_header->header.altitude_max_time = read32inc(data, &offset);
    log_header->header.cadence_avg = read8inc(data, &offset);
    log_header->header.cadence_max = read8inc(data, &offset);
    log_header->header.cadence_max_time = read32inc(data, &offset);
    log_header->header.speed_avg = read16inc(data, &offset); // 10 m/h
    log_header->header.speed_max = read16inc(data, &offset); // 10 m/h
    log_header->header.speed_max_time = read32inc(data, &offset);
    offset += 4; // Unknown bytes
    log_header->header.duration = read32inc(data, &offset)*100; // seconds 0.1
    log_header->header.ascent = read16inc(data, &offset);
    log_header->header.descent = read16inc(data, &offset);
    log_header->header.ascent_time = read32inc(data, &offset)*1000;
    log_header->header.descent_time = read32inc(data, &offset)*1000;
    log_header->header.recovery_time = read16inc(data, &offset)*60*1000;
    log_header->header.peak_training_effect = read8inc(data, &offset);

    if (log_header->header.activity_name) {
        free(log_header->header.activity_name);
    }
    log_header->header.activity_name = utf8memconv((const char*)(data + offset), 16, "UTF-8");

    offset += (strnlen((const char*)(data + offset), 20)+1);

    log_header->header.distance = read32inc(data, &offset);
    log_header->header.energy_consumption = read16inc(data, &offset);

    offset += object->driver_data->driver_params.log_header_tail_length;

    return offset;
}

/**
 * Gets the memory maps from the watch.
 *
 * \param object
 * \return 0 if successful.
 */
static int get_memory_maps(ambit_object_t *object)
{
    uint8_t legacy_format = 0;
    uint8_t *reply_data = NULL;
    size_t replylen = 0;
    uint8_t send_data[4] = { 0x00, 0x00, 0x00, 0x00 };
    libambit_sbem0102_data_t reply_data_object;
    uint8_t mm_entry_data_id = 0;
    memory_map_entry_t *mm_entry;
    const uint8_t *ptr;

    legacy_format = object->driver_data->driver_params.mm_legacy_format;

    if (libambit_protocol_command(object, ambit_command_waypoint_count, NULL, 0, &reply_data, &replylen, legacy_format) != 0 || replylen < 4) {
        libambit_protocol_free(reply_data);
        LOG_WARNING("Failed to read memory map key");
        return -1;
    }
    libambit_protocol_free(reply_data);

    libambit_sbem0102_data_init(&reply_data_object);
    if (libambit_sbem0102_command_request_raw(&object->driver_data->sbem0102, ambit_command_ambit3_memory_map, send_data, sizeof(send_data), &reply_data_object) != 0) {
        LOG_WARNING("Failed to read memory map");
        return -1;
    }

    mm_entry_data_id = object->driver_data->driver_params.mm_entry_data_id;

    while (libambit_sbem0102_data_next(&reply_data_object) == 0) {
        if (libambit_sbem0102_data_id(&reply_data_object) == mm_entry_data_id) {
            ptr = libambit_sbem0102_data_ptr(&reply_data_object);
            LOG_INFO("Memory map entry \"%s\"", ptr);
            mm_entry = NULL;
            if (strcmp((char*)ptr, "Waypoints") == 0) {
                mm_entry = &object->driver_data->memory_maps.waypoints;
            }
            else if (strcmp((char*)ptr, "Routes") == 0) {
                mm_entry = &object->driver_data->memory_maps.routes;
            }
            else if (strcmp((char*)ptr, "Rules") == 0) {
                mm_entry = &object->driver_data->memory_maps.rules;
            }
            else if (strcmp((char*)ptr, "GpsSGEE") == 0) {
                mm_entry = &object->driver_data->memory_maps.gps;
            }
            else if (strcmp((char*)ptr, "CustomModes") == 0) {
                mm_entry = &object->driver_data->memory_maps.sport_modes;
            }
            else if (strcmp((char*)ptr, "TrainingProgram") == 0) {
                mm_entry = &object->driver_data->memory_maps.training_program;
            }
            else if (strcmp((char*)ptr, "ExerciseLog") == 0) {
                mm_entry = &object->driver_data->memory_maps.exercise_log;
            }
            else if (strcmp((char*)ptr, "EventLog") == 0) {
                mm_entry = &object->driver_data->memory_maps.event_log;
            }
            else if (strcmp((char*)ptr, "BlePairingInfo") == 0) {
                mm_entry = &object->driver_data->memory_maps.ble_pairing;
            }
            else if (strcmp((char*)ptr, "Apps") == 0) {
                mm_entry = &object->driver_data->memory_maps.apps;
            }
            else {
                LOG_WARNING("Unknown memory map type \"%s\"", (char*)ptr);
            }

            if (mm_entry != NULL) {
                // We have dealed with the name, advance to hash
                ptr += strlen((char*)ptr) + 1;

                if (libambit_htob((const char*)ptr, mm_entry->hash, sizeof(mm_entry->hash)) < 0) {
                    LOG_ERROR("Failed to read memory map hash");
                }
                ptr += strlen((char*)ptr) + 1;

                mm_entry->start = read32(ptr, 0);
                ptr += 4;
                mm_entry->size = read32(ptr, 0);

                LOG_INFO("Memory map entry \"%s\": start=0x%08x size=%u", (char*)libambit_sbem0102_data_ptr(&reply_data_object), mm_entry->start, mm_entry->size);
            }
        }
    }

    object->driver_data->memory_maps.initialized = 1;
    libambit_sbem0102_data_free(&reply_data_object);

    LOG_INFO("Memory map successfully parsed");

    return 0;
}

/**
 * Set log as synchronized
 *
 * \param object
 * \param log_entry Log to set as synchronized
 * \return 0 if successful.
 */
static int log_synced(ambit_object_t *object, ambit_log_entry_t *log_entry)
{
    libambit_sbem0102_data_t send_data_object, reply_data_object;

    LOG_INFO("Sync log");

    struct {
        uint8_t timestamp[0x14];
        uint8_t synced;
    } sbem0102_synced;

    ambit_date_time_t dt;
    memcpy(&dt, &log_entry->header.date_time, sizeof(dt));
    snprintf((char*)sbem0102_synced.timestamp, sizeof(sbem0102_synced.timestamp), "%04d-%02d-%02dT%02d:%02d:%02d",
            dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.msec);

    sbem0102_synced.synced = 1;
    
    libambit_sbem0102_data_init(&reply_data_object);
    libambit_sbem0102_data_init(&send_data_object);
    libambit_sbem0102_data_add(&send_data_object, object->driver_data->driver_params.log_synced_data_id, (uint8_t*)&sbem0102_synced, sizeof(sbem0102_synced));

    if (libambit_sbem0102_command_request(&object->driver_data->sbem0102, ambit_command_ambit3_log_synced, &send_data_object, &reply_data_object) != 0) {
        LOG_WARNING("Failed to sync log");
        return -1;
    }
    
    return 0;
}
