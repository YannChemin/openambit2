#include "custom_modes_bxml.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

/* BXml tag ids - see custom_modes_bxml.h for provenance. */
#define TAG_DEVICE_CUSTOM               0x0003
#define TAG_EXERCISE_MODES              0x0100
#define TAG_EXERCISE_MODES_MODE         0x0101
#define TAG_EXERCISE_MODES_SETTING_LEN64 0x0103
#define TAG_EXERCISE_MODES_DISPLAYS     0x0105
#define TAG_EXERCISE_MODES_DISPLAY      0x0106
#define TAG_EXERCISE_MODES_DISP_SETTING 0x0107
#define TAG_EXERCISE_MODES_DISP_FIELD   0x0108
#define TAG_EXERCISE_MODES_DISP_FIELD_SETTING  0x0109
#define TAG_EXERCISE_MODES_DISP_FIELD_SHORTCUT 0x010a
#define TAG_EXERCISE_MODES_TYPE         0x010b
#define TAG_EXERCISE_MODES_RULES        0x010c
#define TAG_EXERCISE_MODES_RULE         0x010d
#define TAG_EXERCISE_MODES_APP_META     0x01ff
#define TAG_SPORT_MODES                 0x0200
#define TAG_SPORT_MODE                  0x0210
#define TAG_SPORT_MODE_SETTING_NAME     0x0212 /* legacy 24-byte name, not used on Ambit3 */
#define TAG_SPORT_MODE_ACTIVITY_ID      0x0213
#define TAG_SPORT_MODE_EXERCISE         0x0214
#define TAG_SPORT_MODE_SETTING_NAME_LEN64 0x0215
#define TAG_SPORT_MODE_ORDER            0x02fe
#define TAG_SPORT_MODE_APP_META         0x02ff

#define SETTINGS_BODY_SIZE 138 /* 64 (name) + 16*2 (fields) + 12 (interval slot 0) + 5*6 (slots 1-5) */

static uint16_t rd_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd_u32(const uint8_t *p) { return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24)); }
static void wr_u16(uint8_t *p, uint16_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; }
static void wr_u32(uint8_t *p, uint32_t v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; p[2] = (v >> 16) & 0xff; p[3] = (v >> 24) & 0xff; }

/* Bounded strlen (avoids relying on POSIX strnlen under -std=c99). */
static size_t bounded_strlen(const char *s, size_t max)
{
    size_t n = 0;
    while (n < max && s[n] != '\0') {
        n++;
    }
    return n;
}

/* Reads one [u16 tag][u16 length] header at data[offset]; returns false if it doesn't fit. */
static bool read_tag(const uint8_t *data, uint32_t data_len, uint32_t offset, uint16_t *tag_id, uint16_t *length)
{
    if (offset + 4 > data_len) {
        return false;
    }
    *tag_id = rd_u16(data + offset);
    *length = rd_u16(data + offset + 2);
    return true;
}

/* ---------------------------------------------------------------- decode ---------------------------------------------------------------- */

static int decode_settings(const uint8_t *data, uint32_t content, uint32_t length, ambit_custom_mode_settings_t *out)
{
    uint32_t cursor;
    int i;

    if (length != SETTINGS_BODY_SIZE) {
        LOG_ERROR("CustomModes settings block: expected %d bytes, got %u", SETTINGS_BODY_SIZE, length);
        return -1;
    }

    memset(out, 0, sizeof(*out));
    memcpy(out->name, data + content, CUSTOM_MODES_BXML_NAME_SIZE);
    out->name[CUSTOM_MODES_BXML_NAME_SIZE] = '\0';

    cursor = content + CUSTOM_MODES_BXML_NAME_SIZE;
    out->activity_id        = rd_u16(data + cursor); cursor += 2;
    {
        uint16_t id_low  = rd_u16(data + cursor); cursor += 2;
        uint16_t id_high = rd_u16(data + cursor); cursor += 2;
        out->custom_mode_id = (uint32_t)id_low | ((uint32_t)id_high << 16);
    }
    out->use_hw              = rd_u16(data + cursor); cursor += 2;
    out->alti_baro_mode      = rd_u16(data + cursor); cursor += 2;
    out->gps_power_mode      = rd_u16(data + cursor); cursor += 2;
    out->recording_interval  = rd_u16(data + cursor); cursor += 2;
    out->autolap             = rd_u16(data + cursor); cursor += 2;
    out->hr_high              = rd_u16(data + cursor); cursor += 2;
    out->hr_low               = rd_u16(data + cursor); cursor += 2;
    out->hr_limits_use        = rd_u16(data + cursor); cursor += 2;
    out->auto_start           = rd_u16(data + cursor); cursor += 2;
    out->auto_pause           = rd_u16(data + cursor); cursor += 2;
    out->auto_scrolling       = rd_u16(data + cursor); cursor += 2;
    out->int_timer_flags      = rd_u16(data + cursor); cursor += 2;
    out->int_timer_count      = rd_u16(data + cursor); cursor += 2;

    for (i = 0; i < CUSTOM_MODES_BXML_INTERVAL_SLOTS; i++) {
        ambit_custom_mode_interval_slot_t *slot = &out->interval_slots[i];
        slot->flags     = data[cursor]; cursor += 1;
        slot->type      = data[cursor]; cursor += 1;
        slot->max_limit = rd_u16(data + cursor); cursor += 2;
        slot->min_limit = rd_u16(data + cursor); cursor += 2;
        if (i == 0) {
            slot->padding = rd_u16(data + cursor); cursor += 2;
            slot->len     = rd_u32(data + cursor); cursor += 4;
        }
    }

    return 0;
}

static int decode_disp_field(const uint8_t *data, uint32_t content, uint32_t length, ambit_custom_mode_disp_field_t *out)
{
    uint32_t cursor = content, end = content + length;

    memset(out, 0, sizeof(*out));

    while (cursor < end) {
        uint16_t tag_id, tag_len;
        uint32_t sub_content;

        if (!read_tag(data, end, cursor, &tag_id, &tag_len)) {
            break;
        }
        sub_content = cursor + 4;

        if (tag_id == TAG_EXERCISE_MODES_DISP_FIELD_SETTING && tag_len == 4) {
            out->index = rd_u16(data + sub_content);
            out->type  = rd_u16(data + sub_content + 2);
        } else if (tag_id == TAG_EXERCISE_MODES_DISP_FIELD_SHORTCUT && tag_len == 2) {
            if (out->shortcuts_count >= CUSTOM_MODES_BXML_MAX_SHORTCUTS) {
                LOG_WARNING("CustomModes: too many shortcuts on a display field, truncating");
            } else {
                out->shortcuts[out->shortcuts_count++] = rd_u16(data + sub_content);
            }
        }
        cursor = sub_content + tag_len;
    }

    return 0;
}

static int decode_display(const uint8_t *data, uint32_t content, uint32_t length, ambit_custom_mode_display_t *out)
{
    uint32_t cursor = content, end = content + length;

    memset(out, 0, sizeof(*out));

    while (cursor < end) {
        uint16_t tag_id, tag_len;
        uint32_t sub_content;

        if (!read_tag(data, end, cursor, &tag_id, &tag_len)) {
            break;
        }
        sub_content = cursor + 4;

        if (tag_id == TAG_EXERCISE_MODES_DISP_SETTING && tag_len == 4) {
            out->template_id = rd_u16(data + sub_content);
            out->type        = rd_u16(data + sub_content + 2);
        } else if (tag_id == TAG_EXERCISE_MODES_DISP_FIELD) {
            if (out->fields_count >= CUSTOM_MODES_BXML_MAX_FIELDS) {
                LOG_WARNING("CustomModes: too many fields on a display, truncating");
            } else if (decode_disp_field(data, sub_content, tag_len, &out->fields[out->fields_count]) == 0) {
                out->fields_count++;
            }
        }
        cursor = sub_content + tag_len;
    }

    return 0;
}

static int decode_rule(const uint8_t *data, uint32_t content, uint32_t length, ambit_custom_mode_rule_t *out)
{
    if (length != 6) {
        LOG_WARNING("CustomModes rule: expected 6 bytes, got %u", length);
        return -1;
    }
    out->rule_idx = rd_u16(data + content);
    out->use_rule = rd_u16(data + content + 2) != 0;
    out->log_rule = rd_u16(data + content + 4) != 0;
    return 0;
}

static int decode_exercise_mode(const uint8_t *data, uint32_t content, uint32_t length, ambit_custom_mode_t *out)
{
    uint32_t cursor = content, end = content + length;

    memset(out, 0, sizeof(*out));

    while (cursor < end) {
        uint16_t tag_id, tag_len;
        uint32_t sub_content;

        if (!read_tag(data, end, cursor, &tag_id, &tag_len)) {
            break;
        }
        sub_content = cursor + 4;

        if (tag_id == TAG_EXERCISE_MODES_SETTING_LEN64) {
            if (decode_settings(data, sub_content, tag_len, &out->settings) != 0) {
                return -1;
            }
        } else if (tag_id == TAG_EXERCISE_MODES_APP_META && tag_len == 8) {
            out->has_app_meta       = true;
            out->app_meta_timestamp1 = rd_u32(data + sub_content);
            out->app_meta_timestamp2 = rd_u32(data + sub_content + 4);
        } else if (tag_id == TAG_EXERCISE_MODES_DISPLAYS) {
            uint32_t sc = sub_content, se = sub_content + tag_len;
            while (sc < se) {
                uint16_t d_tag, d_len;
                uint32_t d_content;
                if (!read_tag(data, se, sc, &d_tag, &d_len)) {
                    break;
                }
                d_content = sc + 4;
                if (d_tag == TAG_EXERCISE_MODES_DISPLAY) {
                    if (out->displays_count >= CUSTOM_MODES_BXML_MAX_DISPLAYS) {
                        LOG_WARNING("CustomModes: too many displays on a mode, truncating");
                    } else if (decode_display(data, d_content, d_len, &out->displays[out->displays_count]) == 0) {
                        out->displays_count++;
                    }
                }
                sc = d_content + d_len;
            }
        } else if (tag_id == TAG_EXERCISE_MODES_RULES) {
            uint32_t sc = sub_content, se = sub_content + tag_len;
            while (sc < se) {
                uint16_t r_tag, r_len;
                uint32_t r_content;
                if (!read_tag(data, se, sc, &r_tag, &r_len)) {
                    break;
                }
                r_content = sc + 4;
                if (r_tag == TAG_EXERCISE_MODES_RULE) {
                    if (out->rules_count >= CUSTOM_MODES_BXML_MAX_RULES) {
                        LOG_WARNING("CustomModes: too many rules on a mode, truncating");
                    } else if (decode_rule(data, r_content, r_len, &out->rules[out->rules_count]) == 0) {
                        out->rules_count++;
                    }
                }
                sc = r_content + r_len;
            }
        }
        cursor = sub_content + tag_len;
    }

    return 0;
}

static int decode_sport_mode_slot(const uint8_t *data, uint32_t content, uint32_t length, ambit_multisport_slot_t *out)
{
    uint32_t cursor = content, end = content + length;

    memset(out, 0, sizeof(*out));

    while (cursor < end) {
        uint16_t tag_id, tag_len;
        uint32_t sub_content;

        if (!read_tag(data, end, cursor, &tag_id, &tag_len)) {
            break;
        }
        sub_content = cursor + 4;

        if (tag_id == TAG_SPORT_MODE_SETTING_NAME_LEN64) {
            uint32_t n = tag_len < CUSTOM_MODES_BXML_NAME_SIZE ? tag_len : CUSTOM_MODES_BXML_NAME_SIZE;
            memcpy(out->name, data + sub_content, n);
            out->name[CUSTOM_MODES_BXML_NAME_SIZE] = '\0';
        } else if (tag_id == TAG_SPORT_MODE_ACTIVITY_ID && tag_len == 2) {
            out->activity_id = rd_u16(data + sub_content);
        } else if (tag_id == TAG_SPORT_MODE_EXERCISE && tag_len == 2) {
            if (out->exercises_count >= CUSTOM_MODES_BXML_MAX_EXERCISES) {
                LOG_WARNING("CustomModes: too many exercises on a multisport slot, truncating");
            } else {
                out->exercises[out->exercises_count++] = rd_u16(data + sub_content);
            }
        } else if (tag_id == TAG_SPORT_MODE_ORDER && tag_len == 4) {
            out->has_order = true;
            out->order     = rd_u32(data + sub_content);
        } else if (tag_id == TAG_SPORT_MODE_APP_META && tag_len == 4) {
            out->has_app_meta = true;
            out->app_meta     = rd_u32(data + sub_content);
        }
        cursor = sub_content + tag_len;
    }

    return 0;
}

int ambit_custom_modes_decode(const uint8_t *data, uint32_t data_len, ambit_custom_modes_region_t *out)
{
    uint16_t root_tag, root_len;
    uint32_t cursor, end;

    memset(out, 0, sizeof(*out));

    if (!read_tag(data, data_len, 0, &root_tag, &root_len) || root_tag != TAG_DEVICE_CUSTOM) {
        LOG_ERROR("CustomModes region: expected DEVICE_CUSTOM tag at offset 0");
        return -1;
    }

    cursor = 4;
    end = 4 + root_len;
    if (end > data_len) {
        LOG_ERROR("CustomModes region: DEVICE_CUSTOM length %u exceeds buffer (%u bytes)", root_len, data_len);
        return -1;
    }

    while (cursor < end) {
        uint16_t tag_id, tag_len;
        uint32_t content;

        if (!read_tag(data, end, cursor, &tag_id, &tag_len)) {
            break;
        }
        content = cursor + 4;

        if (tag_id == TAG_EXERCISE_MODES) {
            uint32_t sc = content, se = content + tag_len;
            while (sc < se) {
                uint16_t m_tag, m_len;
                uint32_t m_content;
                if (!read_tag(data, se, sc, &m_tag, &m_len)) {
                    break;
                }
                m_content = sc + 4;
                if (m_tag == TAG_EXERCISE_MODES_TYPE && m_len == 2) {
                    out->format_type = rd_u16(data + m_content);
                } else if (m_tag == TAG_EXERCISE_MODES_MODE) {
                    if (out->modes_count >= CUSTOM_MODES_BXML_MAX_MODES) {
                        LOG_WARNING("CustomModes: too many exercise modes, truncating");
                    } else if (decode_exercise_mode(data, m_content, m_len, &out->modes[out->modes_count]) == 0) {
                        out->modes_count++;
                    } else {
                        return -1;
                    }
                }
                sc = m_content + m_len;
            }
        } else if (tag_id == TAG_SPORT_MODES) {
            uint32_t sc = content, se = content + tag_len;
            while (sc < se) {
                uint16_t s_tag, s_len;
                uint32_t s_content;
                if (!read_tag(data, se, sc, &s_tag, &s_len)) {
                    break;
                }
                s_content = sc + 4;
                if (s_tag == TAG_SPORT_MODE) {
                    if (out->sport_modes_count >= CUSTOM_MODES_BXML_MAX_SPORT_MODES) {
                        LOG_WARNING("CustomModes: too many multisport slots, truncating");
                    } else if (decode_sport_mode_slot(data, s_content, s_len, &out->sport_modes[out->sport_modes_count]) == 0) {
                        out->sport_modes_count++;
                    }
                }
                sc = s_content + s_len;
            }
        }
        cursor = content + tag_len;
    }

    return 0;
}

/* ---------------------------------------------------------------- encode ---------------------------------------------------------------- */

/* All build_* helpers append to *pos and advance it; caller guarantees enough room
 * (ambit_custom_modes_encode bounds-checks the total against out_capacity up front by
 * encoding into a generously-sized scratch buffer first). */

static void wr_tag_header(uint8_t **pos, uint16_t tag_id, uint16_t length)
{
    wr_u16(*pos, tag_id); wr_u16(*pos + 2, length);
    *pos += 4;
}

static void build_settings(uint8_t **pos, const ambit_custom_mode_settings_t *s)
{
    uint8_t *body_start;
    int i;

    wr_tag_header(pos, TAG_EXERCISE_MODES_SETTING_LEN64, SETTINGS_BODY_SIZE);
    body_start = *pos;

    memset(*pos, 0, CUSTOM_MODES_BXML_NAME_SIZE);
    memcpy(*pos, s->name, bounded_strlen(s->name, CUSTOM_MODES_BXML_NAME_SIZE));
    *pos += CUSTOM_MODES_BXML_NAME_SIZE;

    wr_u16(*pos, s->activity_id); *pos += 2;
    wr_u16(*pos, (uint16_t)(s->custom_mode_id & 0xffff)); *pos += 2;
    wr_u16(*pos, (uint16_t)((s->custom_mode_id >> 16) & 0xffff)); *pos += 2;
    wr_u16(*pos, s->use_hw); *pos += 2;
    wr_u16(*pos, s->alti_baro_mode); *pos += 2;
    wr_u16(*pos, s->gps_power_mode); *pos += 2;
    wr_u16(*pos, s->recording_interval); *pos += 2;
    wr_u16(*pos, s->autolap); *pos += 2;
    wr_u16(*pos, s->hr_high); *pos += 2;
    wr_u16(*pos, s->hr_low); *pos += 2;
    wr_u16(*pos, s->hr_limits_use); *pos += 2;
    wr_u16(*pos, s->auto_start); *pos += 2;
    wr_u16(*pos, s->auto_pause); *pos += 2;
    wr_u16(*pos, s->auto_scrolling); *pos += 2;
    wr_u16(*pos, s->int_timer_flags); *pos += 2;
    wr_u16(*pos, s->int_timer_count); *pos += 2;

    for (i = 0; i < CUSTOM_MODES_BXML_INTERVAL_SLOTS; i++) {
        const ambit_custom_mode_interval_slot_t *slot = &s->interval_slots[i];
        **pos = slot->flags; *pos += 1;
        **pos = slot->type; *pos += 1;
        wr_u16(*pos, slot->max_limit); *pos += 2;
        wr_u16(*pos, slot->min_limit); *pos += 2;
        if (i == 0) {
            wr_u16(*pos, slot->padding); *pos += 2;
            wr_u32(*pos, slot->len); *pos += 4;
        }
    }

    (void)body_start; /* kept for readability/debugging; SETTINGS_BODY_SIZE is authoritative */
}

static void build_disp_field(uint8_t **pos, const ambit_custom_mode_disp_field_t *f)
{
    uint16_t body_len = 8 + f->shortcuts_count * 6; /* DISP_FIELD_SETTING tag (4+4) + one DISP_FIELD_SHORTCUT tag (4+2) each */
    int i;

    wr_tag_header(pos, TAG_EXERCISE_MODES_DISP_FIELD, body_len);
    wr_tag_header(pos, TAG_EXERCISE_MODES_DISP_FIELD_SETTING, 4);
    wr_u16(*pos, f->index); *pos += 2;
    wr_u16(*pos, f->type); *pos += 2;
    for (i = 0; i < f->shortcuts_count; i++) {
        wr_tag_header(pos, TAG_EXERCISE_MODES_DISP_FIELD_SHORTCUT, 2);
        wr_u16(*pos, f->shortcuts[i]); *pos += 2;
    }
}

static uint16_t disp_field_size(const ambit_custom_mode_disp_field_t *f)
{
    return 4 + (uint16_t)(8 + f->shortcuts_count * 6);
}

static void build_display(uint8_t **pos, const ambit_custom_mode_display_t *d)
{
    uint16_t body_len = 8; /* DISP_SETTING tag */
    int i;

    for (i = 0; i < d->fields_count; i++) {
        body_len += disp_field_size(&d->fields[i]);
    }

    wr_tag_header(pos, TAG_EXERCISE_MODES_DISPLAY, body_len);
    wr_tag_header(pos, TAG_EXERCISE_MODES_DISP_SETTING, 4);
    wr_u16(*pos, d->template_id); *pos += 2;
    wr_u16(*pos, d->type); *pos += 2;
    for (i = 0; i < d->fields_count; i++) {
        build_disp_field(pos, &d->fields[i]);
    }
}

static uint16_t display_size(const ambit_custom_mode_display_t *d)
{
    uint16_t body_len = 8;
    int i;
    for (i = 0; i < d->fields_count; i++) {
        body_len += disp_field_size(&d->fields[i]);
    }
    return 4 + body_len;
}

static void build_mode(uint8_t **pos, const ambit_custom_mode_t *m)
{
    uint16_t body_len = 4 + SETTINGS_BODY_SIZE; /* SETTING_NAME_LEN64 tag */
    int i;

    if (m->has_app_meta) {
        body_len += 4 + 8;
    }
    if (m->displays_count > 0) {
        uint16_t displays_body = 0;
        for (i = 0; i < m->displays_count; i++) {
            displays_body += display_size(&m->displays[i]);
        }
        body_len += 4 + displays_body;
    }
    if (m->rules_count > 0) {
        body_len += 4 + m->rules_count * (4 + 6);
    }

    wr_tag_header(pos, TAG_EXERCISE_MODES_MODE, body_len);

    build_settings(pos, &m->settings);

    if (m->has_app_meta) {
        wr_tag_header(pos, TAG_EXERCISE_MODES_APP_META, 8);
        wr_u32(*pos, m->app_meta_timestamp1); *pos += 4;
        wr_u32(*pos, m->app_meta_timestamp2); *pos += 4;
    }

    if (m->displays_count > 0) {
        uint16_t displays_body = 0;
        for (i = 0; i < m->displays_count; i++) {
            displays_body += display_size(&m->displays[i]);
        }
        wr_tag_header(pos, TAG_EXERCISE_MODES_DISPLAYS, displays_body);
        for (i = 0; i < m->displays_count; i++) {
            build_display(pos, &m->displays[i]);
        }
    }

    if (m->rules_count > 0) {
        wr_tag_header(pos, TAG_EXERCISE_MODES_RULES, m->rules_count * (4 + 6));
        for (i = 0; i < m->rules_count; i++) {
            const ambit_custom_mode_rule_t *r = &m->rules[i];
            wr_tag_header(pos, TAG_EXERCISE_MODES_RULE, 6);
            wr_u16(*pos, r->rule_idx); *pos += 2;
            wr_u16(*pos, r->use_rule ? 1 : 0); *pos += 2;
            wr_u16(*pos, r->log_rule ? 1 : 0); *pos += 2;
        }
    }
}

static uint16_t mode_size(const ambit_custom_mode_t *m)
{
    uint16_t body_len = 4 + SETTINGS_BODY_SIZE;
    int i;

    if (m->has_app_meta) {
        body_len += 4 + 8;
    }
    if (m->displays_count > 0) {
        uint16_t displays_body = 0;
        for (i = 0; i < m->displays_count; i++) {
            displays_body += display_size(&m->displays[i]);
        }
        body_len += 4 + displays_body;
    }
    if (m->rules_count > 0) {
        body_len += 4 + m->rules_count * (4 + 6);
    }
    return 4 + body_len;
}

static void build_sport_mode_slot(uint8_t **pos, const ambit_multisport_slot_t *sl)
{
    uint16_t body_len = 4 + CUSTOM_MODES_BXML_NAME_SIZE; /* NAME */
    int i;

    body_len += 4 + 2; /* ACTIVITY_ID */
    body_len += sl->exercises_count * (4 + 2);
    if (sl->has_order) {
        body_len += 4 + 4;
    }
    if (sl->has_app_meta) {
        body_len += 4 + 4;
    }

    wr_tag_header(pos, TAG_SPORT_MODE, body_len);

    wr_tag_header(pos, TAG_SPORT_MODE_SETTING_NAME_LEN64, CUSTOM_MODES_BXML_NAME_SIZE);
    memset(*pos, 0, CUSTOM_MODES_BXML_NAME_SIZE);
    memcpy(*pos, sl->name, bounded_strlen(sl->name, CUSTOM_MODES_BXML_NAME_SIZE));
    *pos += CUSTOM_MODES_BXML_NAME_SIZE;

    wr_tag_header(pos, TAG_SPORT_MODE_ACTIVITY_ID, 2);
    wr_u16(*pos, sl->activity_id); *pos += 2;

    for (i = 0; i < sl->exercises_count; i++) {
        wr_tag_header(pos, TAG_SPORT_MODE_EXERCISE, 2);
        wr_u16(*pos, sl->exercises[i]); *pos += 2;
    }

    if (sl->has_order) {
        wr_tag_header(pos, TAG_SPORT_MODE_ORDER, 4);
        wr_u32(*pos, sl->order); *pos += 4;
    }
    if (sl->has_app_meta) {
        wr_tag_header(pos, TAG_SPORT_MODE_APP_META, 4);
        wr_u32(*pos, sl->app_meta); *pos += 4;
    }
}

static uint16_t sport_mode_slot_size(const ambit_multisport_slot_t *sl)
{
    uint16_t body_len = 4 + CUSTOM_MODES_BXML_NAME_SIZE;
    body_len += 4 + 2;
    body_len += sl->exercises_count * (4 + 2);
    if (sl->has_order) {
        body_len += 4 + 4;
    }
    if (sl->has_app_meta) {
        body_len += 4 + 4;
    }
    return 4 + body_len;
}

int ambit_custom_modes_encode(const ambit_custom_modes_region_t *in, uint8_t *out, uint32_t out_capacity, uint32_t *out_len)
{
    uint32_t exercise_modes_body_len, sport_modes_body_len, device_custom_body_len, total_len;
    uint8_t *pos;
    int i;

    exercise_modes_body_len = 4 + 2; /* EXERCISE_MODES_TYPE tag */
    for (i = 0; i < in->modes_count; i++) {
        exercise_modes_body_len += mode_size(&in->modes[i]);
    }

    sport_modes_body_len = 0;
    for (i = 0; i < in->sport_modes_count; i++) {
        sport_modes_body_len += sport_mode_slot_size(&in->sport_modes[i]);
    }

    device_custom_body_len = (4 + exercise_modes_body_len) + (4 + sport_modes_body_len);
    total_len = 4 + device_custom_body_len;

    if (total_len > out_capacity) {
        LOG_ERROR("CustomModes region: encoded size %u exceeds region capacity %u", total_len, out_capacity);
        return -1;
    }

    pos = out;
    wr_tag_header(&pos, TAG_DEVICE_CUSTOM, (uint16_t)device_custom_body_len);

    wr_tag_header(&pos, TAG_EXERCISE_MODES, (uint16_t)exercise_modes_body_len);
    wr_tag_header(&pos, TAG_EXERCISE_MODES_TYPE, 2);
    wr_u16(pos, in->format_type); pos += 2;
    for (i = 0; i < in->modes_count; i++) {
        build_mode(&pos, &in->modes[i]);
    }

    wr_tag_header(&pos, TAG_SPORT_MODES, (uint16_t)sport_modes_body_len);
    for (i = 0; i < in->sport_modes_count; i++) {
        build_sport_mode_slot(&pos, &in->sport_modes[i]);
    }

    if ((uint32_t)(pos - out) != total_len) {
        LOG_ERROR("CustomModes region: internal size mismatch (wrote %ld, expected %u)", (long)(pos - out), total_len);
        return -1;
    }

    memset(out + total_len, 0xff, out_capacity - total_len);
    *out_len = out_capacity;

    return 0;
}
