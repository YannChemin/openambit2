#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libambit.h>

static int log_skip_cb(void *ambit_object, ambit_log_header_t *log_header);
static void log_data_cb(void *object, ambit_log_entry_t *log_entry);
static void dump_memory_map_and_regions(ambit_object_t *ambit_object);
static void hex_dump(const uint8_t *data, uint32_t len, uint32_t max_bytes);

int main(int argc, char *argv[])
{
    ambit_device_info_t *info = libambit_enumerate();
    ambit_object_t *ambit_object;
    ambit_device_status_t status;
    ambit_personal_settings_t settings;
    memset(&settings, 0, sizeof(ambit_personal_settings_t));

    if (info) {
        printf("Device: %s, serial: %s\n", info->name, info->serial);
        if (0 == info->access_status) {
          printf("F/W version: %d.%d.%d\n", info->fw_version[0], info->fw_version[1], (info->fw_version[2] << 0) | (info->fw_version[3] << 8));
            if (!info->is_supported) {
                printf("Device is not supported yet!\n");
            }
        }
        else {
            printf("%s: %s\n", info->path, strerror(info->access_status));
        }

        ambit_object = libambit_new(info);
        if (ambit_object) {

            if (libambit_device_status_get(ambit_object, &status) == 0) {
                printf("Current charge: %d%%\n", status.charge);
            }
            else {
                printf("Failed to read status\n");
            }

            if (libambit_personal_settings_get(ambit_object, &settings) == 0) {
                printf("Personal settings: \n");
                printf("sportmode_button_lock: %d\n", settings.sportmode_button_lock);
                printf("weight: %d\n", settings.weight);
                printf("birthyear: %d\n", settings.birthyear);
            }
            else {
                printf("Failed to read personal settings\n");
            }

            dump_memory_map_and_regions(ambit_object);

            if (argc < 2 || strcmp(argv[1], "--no-logs") != 0) {
                libambit_log_read(ambit_object, log_skip_cb, log_data_cb, NULL, ambit_object);
            }

            libambit_close(ambit_object);
        }
    }
    else {
        printf("No clock found, exiting\n");
    }
    libambit_free_enumeration(info);

    return 0;
}

/* Ambit3-family only (libambit_memory_map_get() returns -1 on legacy
 * Ambit/Ambit2, which has no discovery command). Read-only: dumps the
 * device's own reported region addresses/sizes, then the first bytes of
 * each region of interest for this project's training-plan work
 * (CustomModes, Apps, TrainingProgram), so their real on-device layout can
 * be confirmed without needing to look at the watch's screen. */
static void dump_memory_map_and_regions(ambit_object_t *ambit_object)
{
    ambit_memory_region_t regions[16];
    int count;
    int i;
    uint8_t *buf;
    uint32_t dump_len;

    count = libambit_memory_map_get(ambit_object, regions, 16);
    if (count < 0) {
        printf("Memory map: not supported by this device's driver (legacy Ambit/Ambit2)\n");
        return;
    }

    printf("\nMemory map (%d regions):\n", count);
    for (i = 0; i < count; i++) {
        printf("  %-16s start=0x%08x size=%u\n", regions[i].name, regions[i].start, regions[i].size);
    }

    printf("\nRegion dumps (first bytes, read-only):\n");
    for (i = 0; i < count; i++) {
        if (strcmp(regions[i].name, "CustomModes") != 0 &&
            strcmp(regions[i].name, "Apps") != 0 &&
            strcmp(regions[i].name, "TrainingProgram") != 0) {
            continue;
        }

        dump_len = regions[i].size < 256 ? regions[i].size : 256;
        buf = (uint8_t*)malloc(dump_len);
        if (buf == NULL) {
            continue;
        }

        printf("\n%s (0x%08x, %u of %u bytes):\n", regions[i].name, regions[i].start, dump_len, regions[i].size);
        if (libambit_flash_read(ambit_object, regions[i].start, dump_len, buf) == 0) {
            hex_dump(buf, dump_len, dump_len);
        }
        else {
            printf("  (read failed)\n");
        }
        free(buf);
    }
}

static void hex_dump(const uint8_t *data, uint32_t len, uint32_t max_bytes)
{
    uint32_t i, j;
    uint32_t n = len < max_bytes ? len : max_bytes;

    for (i = 0; i < n; i += 16) {
        printf("  %04x: ", i);
        for (j = i; j < i + 16; j++) {
            if (j < n) {
                printf("%02x ", data[j]);
            }
            else {
                printf("   ");
            }
        }
        printf(" ");
        for (j = i; j < i + 16 && j < n; j++) {
            printf("%c", (data[j] >= 32 && data[j] < 127) ? data[j] : '.');
        }
        printf("\n");
    }
}

static int log_skip_cb(void *ambit_object, ambit_log_header_t *log_header)
{
    static int log_count = 0;

    printf("Got log header \"%s\" %d-%02d-%02d %02d:%02d:%02d\n", log_header->activity_name, log_header->date_time.year, log_header->date_time.month, log_header->date_time.day, log_header->date_time.hour, log_header->date_time.minute, log_header->date_time.msec/1000);

    if (log_count++ > 1) {
        return 0;
    }

    return 1;
}

static void log_data_cb(void *object, ambit_log_entry_t *log_entry)
{
    printf("Got log entry \"%s\" %d-%02d-%02d %02d:%02d:%02d\n", log_entry->header.activity_name, log_entry->header.date_time.year, log_entry->header.date_time.month, log_entry->header.date_time.day, log_entry->header.date_time.hour, log_entry->header.date_time.minute, log_entry->header.date_time.msec/1000);

    int i;
    for (i=0; i<log_entry->header.samples_count; i++) {
        printf("Sample #%d, type: %d, time: %04u-%02u-%02u %02u:%02u:%2.3f\n", i, log_entry->samples[i].type, log_entry->samples[i].utc_time.year, log_entry->samples[i].utc_time.month, log_entry->samples[i].utc_time.day, log_entry->samples[i].utc_time.hour, log_entry->samples[i].utc_time.minute, (1.0*log_entry->samples[i].utc_time.msec)/1000);
    }
}
