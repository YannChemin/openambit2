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
 * Encoder/decoder for the Ambit3-family "TrainingProgram" flash region: the native
 * Movescount-era "Today 1/2" planned-move card (TIME display mode -> [Next]). Unlike the
 * CustomModes/Apps work elsewhere in this project, this needs no App Zone bytecode and no
 * external compiler at all - it is a plain, fixed-layout list of planned-move records the
 * watch's own firmware reads directly.
 *
 * HARDWARE-CONFIRMED, not experimental: ported from the sibling sommet project
 * (https://github.com/skinnie/sommet, tools/training_program.py) after their 2026-09-03
 * confirmation on a real Ambit3 Sport (fw 2.4.17) - a planned move dated today displayed on
 * the watch's TIME-mode "Today" screen. The format itself (12-byte header + up-to-60 fixed
 * 40-byte items) was decompiled from Suunto's own desktop packer
 * (TrainingProgramAreaConverter::createBinary); the one field that defeated every earlier
 * hardware test on both projects - header bytes 4..7 - was resolved by decompiling the
 * watch's own MSP430X firmware: they are the athlete's four HR-zone boundary percentages
 * (Moderate/Hard/VeryHard/Maximal), not a signature, and the firmware rejects the whole
 * header (silently showing nothing) unless they are strictly increasing with the last <101.
 * From-scratch C reimplementation of the documented wire format, not a copy of their Python
 * source or of any decompiled/copyrighted code.
 *
 * KNOWN LIMITATION vs. this project's App-Zone "Training Plan" work (trainingplan.h):
 * a TrainingProgram item is a single flat record - name/duration/intensity/distance/
 * activityId - with no multi-step interval structure, no live target-range alerting, and no
 * countdown display of its own. It is a simpler, more limited, but hardware-proven
 * complement to (not a full replacement for) the App-Zone workout/training-plan path, which
 * remains the only way to schedule a real multi-phase interval workout. Use this for "remind
 * me I have a run planned today"; use the App-Zone path for the run's actual interval structure.
 */
#ifndef TRAINING_PROGRAM_H
#define TRAINING_PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define TRAINING_PROGRAM_NAME_LEN   23   /* UTF-8, null-padded/truncated */
#define TRAINING_PROGRAM_ITEM_SIZE  40
#define TRAINING_PROGRAM_HEADER_SIZE 12
#define TRAINING_PROGRAM_MAX_ITEMS  60   /* firmware limit, per sommet's hardware testing */

/* Suunto's own default HR-zone boundaries (Moderate/Hard/VeryHard/Maximal, % of max HR).
 * Also the firmware's own baked fallback, so it always validates and produces sane targets -
 * safe to use when the caller has no real zones to hand. */
extern const uint8_t TRAINING_PROGRAM_DEFAULT_HR_ZONES[4];

typedef struct ambit_training_program_item_s {
    uint8_t  day_offset;      /* days after the program's base date; 0 = the base date itself */
    bool     completed;
    uint16_t activity_id;
    uint32_t move_id;         /* 0 unless this planned move points at an installed guided workout (see training_program.h's own header comment) */
    uint32_t distance;        /* metres, 0 if not planned by distance */
    uint16_t duration_minutes;
    uint8_t  intensity;       /* 1-5 */
    char     name[TRAINING_PROGRAM_NAME_LEN + 1]; /* +1 for NUL */
} ambit_training_program_item_t;

typedef struct ambit_training_program_s {
    /* calendar date of the EARLIEST item; every item's real date is this date + its own day_offset days */
    uint16_t base_year;
    uint8_t  base_month;
    uint8_t  base_day;

    uint8_t  hr_zones[4]; /* Moderate/Hard/VeryHard/Maximal %, must be strictly increasing
                            * and the last must be < 101 - see this file's own header comment */

    ambit_training_program_item_t items[TRAINING_PROGRAM_MAX_ITEMS];
    uint16_t items_count;
} ambit_training_program_t;

/**
 * Encodes \a in into the TrainingProgram region's real on-device format.
 * \param out destination buffer, at least \a out_capacity bytes
 * \param out_capacity size of \a out (a sanity bound, e.g. the region's reported size)
 * \param out_len set to the real encoded length (header + items, no padding) on success
 * \return 0 on success, -1 if \a in->hr_zones are not strictly increasing (last < 101), if
 *         \a in->items_count exceeds TRAINING_PROGRAM_MAX_ITEMS, or if the encoded size would
 *         not fit in \a out_capacity - logged via LOG_ERROR
 */
int ambit_training_program_encode(const ambit_training_program_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len);

/**
 * Decodes a raw TrainingProgram region dump into \a out.
 * \return 0 on success, -1 if \a data_len is too short for the declared item count, or the
 *         item count exceeds TRAINING_PROGRAM_MAX_ITEMS - logged via LOG_ERROR. An all-0xFF
 *         (erased/empty) region decodes successfully with items_count == 0 and header fields
 *         set to their raw (0xFFFF-ish) values - the caller should check items_count, not
 *         treat a successful decode alone as "a program exists".
 */
int ambit_training_program_decode(const uint8_t *data, uint32_t data_len, ambit_training_program_t *out);

#ifdef __cplusplus
}
#endif

#endif /* TRAINING_PROGRAM_H */
