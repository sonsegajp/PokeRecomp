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
void PcRenderFrame(uint32_t *out, int width, void (*hblank)(int line));

/* video.c */
int PcVideoInit(const char *title);
void PcVideoPresent(const uint32_t *frame, int width, int height);
void PcVideoShutdown(void);

/* input.c */
uint16_t PcInputPoll(int *quit);   /* returns KEYINPUT (active-low) */

/* audio.c */
void PcAudioInit(void);
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
