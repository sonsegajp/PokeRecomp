/* settings.c — load/save settings.ini (simple "key = value" lines). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pc_settings.h"

#define SETTINGS_PATH "settings.ini"

struct PcSettings gPcSettings = {
    .window_scale = 4,
    .fullscreen = 0,
    .filter = PC_FILTER_SHARP,
    .scaling = PC_SCALE_FIT,
    .color_correct = 0,
    .widescreen = PC_WIDE_OFF,
    .vsync = 1,
    .volume = 8,
    .hq_audio = 1,
    .fast_forward = 4,
};

static const struct { const char *key; int *value; int lo, hi; } fields[] = {
    {"window_scale", &gPcSettings.window_scale, 1, 8},
    {"fullscreen", &gPcSettings.fullscreen, 0, 1},
    {"filter", &gPcSettings.filter, 0, PC_FILTER_COUNT - 1},
    {"scaling", &gPcSettings.scaling, 0, PC_SCALE_COUNT - 1},
    {"color_correct", &gPcSettings.color_correct, 0, 1},
    {"widescreen", &gPcSettings.widescreen, 0, PC_WIDE_COUNT - 1},
    {"vsync", &gPcSettings.vsync, 0, 1},
    {"volume", &gPcSettings.volume, 0, 10},
    {"hq_audio", &gPcSettings.hq_audio, 0, 1},
    {"fast_forward", &gPcSettings.fast_forward, 2, 8},
};

void PcSettingsLoad(void)
{
    FILE *f = fopen(SETTINGS_PATH, "r");
    char line[256];

    if (!f) return;
    while (fgets(line, sizeof line, f)) {
        char key[64];
        int value;
        if (sscanf(line, " %63[a-z_] = %d", key, &value) != 2) continue;
        for (size_t i = 0; i < sizeof fields / sizeof fields[0]; i++) {
            if (strcmp(key, fields[i].key)) continue;
            if (value < fields[i].lo) value = fields[i].lo;
            if (value > fields[i].hi) value = fields[i].hi;
            *fields[i].value = value;
        }
    }
    fclose(f);
}

void PcSettingsSave(void)
{
    FILE *f = fopen(SETTINGS_PATH, "w");

    if (!f) return;
    fprintf(f, "# FireRed PC settings (also editable in-game: OPTIONS > PC SETTINGS)\n");
    for (size_t i = 0; i < sizeof fields / sizeof fields[0]; i++)
        fprintf(f, "%s = %d\n", fields[i].key, *fields[i].value);
    fclose(f);
}
