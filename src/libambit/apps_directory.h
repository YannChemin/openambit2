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
 * Encoder/decoder for the Ambit3-family "Apps" flash region's real on-device format: a
 * self-describing directory (num_entries, a per-entry offset table, total_length) followed
 * by fixed 32-byte [header+name] blocks + an 8-byte "IAMRULE" magic + compiled bytecode per
 * entry. This is a different, later-discovered format than the simpler length-prefixed one
 * sport_mode_serialize.c's serialize_app_data()/calculate_app_rule_checksum() implement
 * (which openambit2's Phase 1 app_data_write() currently uses, unverified on real hardware -
 * see this project's training-plans plan for the full history).
 *
 * Ported from the sibling sommet project's independent reverse-engineering
 * (https://github.com/skinnie/sommet, tools/apps.py), which verified this format byte-exact
 * against real SuuntoLink install captures and a live device read. Credited per this
 * project's usual attribution practice. From-scratch C reimplementation of the documented
 * wire format, not a copy of their Python source or of any decompiled/copyrighted code.
 *
 * One field remains genuinely unknown even in sommet's own research: the directory header's
 * second u16 varies across real samples in a way consistent with (but not proven to be)
 * num_entries ^ 0x02 - this implementation writes that value on encode (matching every
 * sample checked) and does not use it for anything on decode.
 */
#ifndef APPS_DIRECTORY_H
#define APPS_DIRECTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APPS_DIR_NAME_LEN     29  /* null-padded, or truncated if the real name doesn't fit */
#define APPS_DIR_MAGIC        "IAMRULE"  /* 7 bytes + the NUL below = the real 8-byte magic */
#define APPS_DIR_MAGIC_LEN    8
#define APPS_DIR_MAX_ENTRIES  64

typedef struct ambit_apps_dir_entry_s {
    uint8_t  activity_id;
    char     name[APPS_DIR_NAME_LEN + 1]; /* +1 for NUL; wire field is not NUL-terminated if it fills all 29 bytes */
    uint8_t  *binary;         /* compiled bytecode, WITHOUT the IAMRULE magic */
    uint32_t binary_length;
} ambit_apps_dir_entry_t;

typedef struct ambit_apps_dir_region_s {
    ambit_apps_dir_entry_t entries[APPS_DIR_MAX_ENTRIES];
    uint16_t entries_count;
} ambit_apps_dir_region_t;

/**
 * Decodes a raw Apps flash region dump into \a out. \a out's binary pointers point into
 * \a data (no allocation/copy) - they are only valid as long as \a data stays alive.
 * \return 0 on success, -1 if \a data doesn't look like a real directory (e.g. an empty/
 *         all-0xff region, or a malformed/unrecognized header) - logged via LOG_ERROR.
 */
int ambit_apps_dir_decode(const uint8_t *data, uint32_t data_len, ambit_apps_dir_region_t *out);

/**
 * Encodes \a in into the real on-device directory format. Unlike CustomModes' region (which
 * is always written and read as one fixed-size block), a real Apps-region write only ever
 * covers the directory's own total_length - there is no padding to the full declared region
 * size (200000 bytes) here, matching real captured installs and avoiding a needlessly slow
 * ~200KB write/read for what's usually a few KB of real content.
 * \param out destination buffer, at least \a out_capacity bytes
 * \param out_capacity size of \a out (a sanity bound, e.g. the region's reported size)
 * \param out_len set to the real encoded length (<= out_capacity) on success
 * \return 0 on success, -1 if the encoded directory would not fit in \a out_capacity
 */
int ambit_apps_dir_encode(const ambit_apps_dir_region_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len);

/**
 * The per-entry 'marker' byte: XOR checksum of (IAMRULE magic + binary), XORed with the low
 * byte of that payload's length. Identical formula to sport_mode_serialize.c's
 * calculate_app_rule_checksum() applied to magic+binary instead of plain binary+length -
 * cross-validated independently by both this project and sommet's own research.
 */
uint8_t ambit_apps_dir_entry_marker(const uint8_t *binary, uint32_t binary_length);

#ifdef __cplusplus
}
#endif

#endif /* APPS_DIRECTORY_H */
