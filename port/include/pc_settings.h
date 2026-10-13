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
    PC_WIDE_AUTO,        /* match the window / monitor shape: no side bars */
    PC_WIDE_COUNT
};

/* Controls: what each GBA button (and fast-forward) is on the keyboard and on
 * a controller. Keyboard: SDL scancodes (USB HID usage ids). Controller: SDL
 * game controller buttons, or the triggers (PC_PAD_LT / PC_PAD_RT). Remapped
 * in the launcher (Options > Controls) or in game (F1, or hold Back + Start). */
enum PcBind {
    PC_BIND_A, PC_BIND_B, PC_BIND_SELECT, PC_BIND_START,
    PC_BIND_UP, PC_BIND_DOWN, PC_BIND_LEFT, PC_BIND_RIGHT,
    PC_BIND_L, PC_BIND_R, PC_BIND_TURBO,
    PC_BIND_COUNT
};
#define PC_PAD_LT 100
#define PC_PAD_RT 101
#define PC_KEY_MAX 511
#define PC_PAD_MAX 101

/* Widest field the camera keeps drawn (field_camera.c WIDE_COLS): ~2.7:1.
 * Wider windows get thin side bars. */
#define PC_WIDE_MAX_WIDTH 432

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
    int key_bind[PC_BIND_COUNT];   /* enum PcBind -> keyboard scancode */
    int pad_bind[PC_BIND_COUNT];   /* enum PcBind -> controller button / PC_PAD_LT / PC_PAD_RT */
};

extern const int gPcDefaultKeyBind[PC_BIND_COUNT], gPcDefaultPadBind[PC_BIND_COUNT];

extern struct PcSettings gPcSettings;

void PcSettingsLoad(void);
void PcSettingsSave(void);
/* Re-apply display settings (window size, fullscreen, vsync) after a change. */
void PcSettingsApplyVideo(void);

#endif
