/* pc_settings.h — PC-only settings, stored in settings.ini next to the game.
 *
 * Shared by the platform layer and the in-game PC SETTINGS screen, so plain
 * C with no SDL types. */
#ifndef PC_SETTINGS_H
#define PC_SETTINGS_H

enum PcFilter {
    PC_FILTER_NEAREST,   /* raw pixels */
    PC_FILTER_SHARP,     /* crisp pixels at any scale (smoothed only at pixel edges) */
    PC_FILTER_SMOOTH,    /* bilinear */
    PC_FILTER_LCD,       /* GBA LCD: pixel grid + subpixel stripes */
    PC_FILTER_CRT,       /* scanlines + shadow mask */
    PC_FILTER_COUNT
};

enum PcScaling {
    PC_SCALE_FIT,        /* largest aspect-correct size */
    PC_SCALE_INTEGER,    /* largest whole-number multiple */
    PC_SCALE_STRETCH,    /* fill the window */
    PC_SCALE_COUNT
};

enum PcWidescreen {
    PC_WIDE_OFF,         /* 3:2, the GBA screen */
    PC_WIDE_16_10,
    PC_WIDE_16_9,
    PC_WIDE_COUNT
};

struct PcSettings {
    int window_scale;    /* 2..8: window size as a multiple of the GBA screen */
    int fullscreen;      /* 0/1 */
    int filter;          /* enum PcFilter */
    int scaling;         /* enum PcScaling */
    int color_correct;   /* 0/1: emulate the GBA LCD's colours */
    int widescreen;      /* enum PcWidescreen */
    int vsync;           /* 0/1 */
    int volume;          /* 0..10 */
    int hq_audio;        /* 1 = 48 kHz interpolated mixer, 0 = GBA-accurate 13 kHz */
    int fast_forward;    /* speed multiplier while the turbo key is held: 2..8 */
};

extern struct PcSettings gPcSettings;

void PcSettingsLoad(void);
void PcSettingsSave(void);
/* Re-apply display settings (window size, fullscreen, vsync) after a change. */
void PcSettingsApplyVideo(void);

#endif
