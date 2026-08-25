/*
 * (C) Copyright 2026 Yann Chemin
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
 * Encoder/decoder for the Ambit3-family CustomModes flash region's "BXml" tag-tree
 * format, distinct from and newer than the fixed-offset format sport_mode_serialize.c
 * implements (which matches Ambit1/Ambit2-era firmware only). Confirmed live against a
 * real Ambit3 Sport (firmware 2.4.17): its CustomModes region uses tag 0x0103
 * (EXERCISE_MODES_SETTING_NAME_LEN64, 138-byte settings block, 64-char name) rather than
 * sport_mode_serialize.c's tag 0x0102 (90-byte settings block, 16-char name).
 *
 * The tag layout and field semantics here are ported from the sibling project sommet's
 * independent reverse-engineering (https://github.com/skinnie/sommet, tools/custom_modes.py
 * and tools/custom_modes_write.py), credited per this project's usual attribution practice.
 * This is a from-scratch C reimplementation of the documented wire format, not a copy of
 * their Python source or of any decompiled/copyrighted third-party code.
 */
#ifndef __CUSTOM_MODES_BXML_H__
#define __CUSTOM_MODES_BXML_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define CUSTOM_MODES_BXML_NAME_SIZE       64
#define CUSTOM_MODES_BXML_MAX_MODES       16
#define CUSTOM_MODES_BXML_MAX_SPORT_MODES 16
#define CUSTOM_MODES_BXML_MAX_DISPLAYS    16
#define CUSTOM_MODES_BXML_MAX_FIELDS      8
#define CUSTOM_MODES_BXML_MAX_SHORTCUTS   8
#define CUSTOM_MODES_BXML_MAX_RULES       4
#define CUSTOM_MODES_BXML_MAX_EXERCISES   8
#define CUSTOM_MODES_BXML_INTERVAL_SLOTS  6   /* 1 full slot + 5 short slots */

typedef struct ambit_custom_mode_interval_slot_s {
    uint8_t  flags;
    uint8_t  type;
    uint16_t max_limit;
    uint16_t min_limit;
    uint16_t padding;   /* slot 0 (full) only, ignored for slots 1-5 */
    uint32_t len;       /* slot 0 (full) only, ignored for slots 1-5 */
} ambit_custom_mode_interval_slot_t;

typedef struct ambit_custom_mode_settings_s {
    char     name[CUSTOM_MODES_BXML_NAME_SIZE + 1]; /* +1 for NUL, wire field is not NUL-terminated */
    uint16_t activity_id;
    uint32_t custom_mode_id;
    uint16_t use_hw;
    uint16_t alti_baro_mode;
    uint16_t gps_power_mode;
    uint16_t recording_interval;
    uint16_t autolap;
    uint16_t hr_high;
    uint16_t hr_low;
    uint16_t hr_limits_use;
    uint16_t auto_start;
    uint16_t auto_pause;
    uint16_t auto_scrolling;
    uint16_t int_timer_flags;
    uint16_t int_timer_count;
    ambit_custom_mode_interval_slot_t interval_slots[CUSTOM_MODES_BXML_INTERVAL_SLOTS];
} ambit_custom_mode_settings_t;

typedef struct ambit_custom_mode_disp_field_s {
    uint16_t index;
    uint16_t type;
    uint16_t shortcuts[CUSTOM_MODES_BXML_MAX_SHORTCUTS];
    uint16_t shortcuts_count;
} ambit_custom_mode_disp_field_t;

typedef struct ambit_custom_mode_display_s {
    uint16_t template_id;
    uint16_t type;
    ambit_custom_mode_disp_field_t fields[CUSTOM_MODES_BXML_MAX_FIELDS];
    uint16_t fields_count;
} ambit_custom_mode_display_t;

typedef struct ambit_custom_mode_rule_s {
    uint16_t rule_idx;
    bool     use_rule;
    bool     log_rule;
} ambit_custom_mode_rule_t;

typedef struct ambit_custom_mode_s {
    ambit_custom_mode_settings_t settings;

    ambit_custom_mode_display_t displays[CUSTOM_MODES_BXML_MAX_DISPLAYS];
    uint16_t displays_count;

    ambit_custom_mode_rule_t rules[CUSTOM_MODES_BXML_MAX_RULES];
    uint16_t rules_count;

    bool     has_app_meta;
    uint32_t app_meta_timestamp1;
    uint32_t app_meta_timestamp2;
} ambit_custom_mode_t;

typedef struct ambit_multisport_slot_s {
    char     name[CUSTOM_MODES_BXML_NAME_SIZE + 1];
    uint16_t activity_id;

    uint16_t exercises[CUSTOM_MODES_BXML_MAX_EXERCISES];
    uint16_t exercises_count;

    bool     has_order;
    uint32_t order;

    bool     has_app_meta;
    uint32_t app_meta;
} ambit_multisport_slot_t;

typedef struct ambit_custom_modes_region_s {
    uint16_t format_type;

    ambit_custom_mode_t modes[CUSTOM_MODES_BXML_MAX_MODES];
    uint16_t modes_count;

    ambit_multisport_slot_t sport_modes[CUSTOM_MODES_BXML_MAX_SPORT_MODES];
    uint16_t sport_modes_count;
} ambit_custom_modes_region_t;

/**
 * Decodes a raw CustomModes flash region dump into \a out.
 * \param data raw region bytes (as read via libambit_flash_read())
 * \param data_len number of bytes in \a data (the full region size is fine; trailing
 *        0xff/erased-flash padding after the BXml tree is ignored)
 * \param out destination struct, need not be pre-initialized
 * \return 0 on success, -1 on a malformed/unrecognized region or if a fixed-size
 *         array above would overflow (logged via LOG_ERROR/LOG_WARNING)
 */
int ambit_custom_modes_decode(const uint8_t *data, uint32_t data_len, ambit_custom_modes_region_t *out);

/**
 * Encodes \a in back into the BXml tag-tree body, padded with 0xff (erased-flash
 * convention, matching what real Ambit3 firmware writes for the unused tail of the
 * region) up to \a out_capacity bytes.
 * \param in source struct
 * \param out destination buffer, at least \a out_capacity bytes
 * \param out_capacity total region size (e.g. the CustomModes region's reported size)
 * \param out_len set to \a out_capacity on success (the whole padded region is always written)
 * \return 0 on success, -1 if the encoded BXml body would not fit in \a out_capacity
 */
int ambit_custom_modes_encode(const ambit_custom_modes_region_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* __CUSTOM_MODES_BXML_H__ */
