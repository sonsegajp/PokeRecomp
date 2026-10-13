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
    /* X Z Backspace Enter, arrows, A S, Tab */
    .key_bind = {27, 29, 42, 40, 82, 81, 80, 79, 4, 22, 43},
    /* A B Back Start, d-pad, LB RB, RT */
    .pad_bind = {0, 1, 4, 6, 11, 12, 13, 14, 9, 10, PC_PAD_RT},
};

const int gPcDefaultKeyBind[PC_BIND_COUNT] = {27, 29, 42, 40, 82, 81, 80, 79, 4, 22, 43};
const int gPcDefaultPadBind[PC_BIND_COUNT] = {0, 1, 4, 6, 11, 12, 13, 14, 9, 10, PC_PAD_RT};

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
#define BIND(name, i) {"key_" name, &gPcSettings.key_bind[i], 0, PC_KEY_MAX}, \
                      {"pad_" name, &gPcSettings.pad_bind[i], 0, PC_PAD_MAX}
    BIND("a", PC_BIND_A), BIND("b", PC_BIND_B), BIND("select", PC_BIND_SELECT), BIND("start", PC_BIND_START),
    BIND("up", PC_BIND_UP), BIND("down", PC_BIND_DOWN), BIND("left", PC_BIND_LEFT), BIND("right", PC_BIND_RIGHT),
    BIND("l", PC_BIND_L), BIND("r", PC_BIND_R), BIND("turbo", PC_BIND_TURBO),
#undef BIND
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
    fprintf(f, "# PokeRecomp settings (also in the launcher's Options, and in game: OPTIONS > PC SETTINGS, F1 for controls)\n");
    for (size_t i = 0; i < sizeof fields / sizeof fields[0]; i++)
        fprintf(f, "%s = %d\n", fields[i].key, *fields[i].value);
    fclose(f);
}
