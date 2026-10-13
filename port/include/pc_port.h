/* pc_port.h — internal interfaces of the platform layer (not seen by game code). */
#ifndef PC_PORT_H
#define PC_PORT_H

#include <stdint.h>

#define GBA_WIDTH  240
#define GBA_HEIGHT 160

/* main.c */
void PcRestartGame(void);
void PcFatal(const char *fmt, ...);
uint16_t PcIoRead16(uint32_t offset);
void PcIoWrite16(uint32_t offset, uint16_t value);

/* Raise an interrupt: sets IF and, if IME and IE allow it, runs the game's
 * handler now (on the host fiber). `flag` is the INTR_FLAG_* bit. */
void PcRaiseInterrupt(uint16_t flag);

/* dma.c */
void PcDmaOnHBlank(void);   /* run HBlank-timed channels (after each visible line) */
void PcDmaOnVBlank(void);   /* run VBlank-timed channels */

/* render.c — draw one frame from VRAM/OAM/palette/registers into `out`
 * (RGBA8888, `width` x 160). Calls `hblank` after each visible line. */
/* Draw a frame `width` pixels wide (the GBA screen centered). Only the BGs in
 * `wide_bgs` (bit per BG) extend past the GBA screen; the rest end at its edges. */
void PcRenderFrame(uint32_t *out, int width, int wide_bgs, void (*hblank)(int line));
/* Provided by the game (overworld.c): is the field (map) being shown? */
int PcFieldIsOnScreen(void);

/* video.c */
int PcVideoInit(const char *title);
void PcVideoPresent(const uint32_t *frame, int width, int height);
void PcVideoShutdown(void);
int PcVideoDisplayWidth(void);   /* logical width for the widescreen setting */
void PcVideoDumpScreen(const char *path);
int PcVideoVsyncPaces(void);

/* input.c */
/* Returns KEYINPUT (active-low). Also handles host hotkeys:
 * F11 fullscreen, F2 next filter, Tab (hold) fast-forward. */
uint16_t PcInputPoll(int *quit, int *fast_forward);

/* audio.c */
void PcAudioInit(int muted);
void PcAudioFrame(void);           /* after each VBlank */

/* nullguard.c */
void PcNullGuardInstall(void);

/* testing.c */
int PcTestLoadInput(const char *path);
uint16_t PcTestInput(int frame);
int PcTestOpenWav(const char *path);
void PcTestCaptureAudio(const float *stereo, int frames);
void PcTestCloseWav(void);

/* save.c */
void PcSaveLoad(void);
void PcSaveFlush(void);

#endif
