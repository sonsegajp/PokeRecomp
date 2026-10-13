/* pc_hw.h — the GBA hardware the game addresses directly, as host memory.
 *
 * Included by pret's gba/defines.h and gba/io_reg.h when PLATFORM_PC is
 * defined. The game keeps writing "registers" and "VRAM" exactly as on the
 * console; the platform layer (renderer, DMA unit, mixer) reads them back.
 * Plain C only: this is seen by every game translation unit. */
#ifndef PC_HW_H
#define PC_HW_H

#define PC_VRAM_SIZE 0x18000
#define PC_PLTT_SIZE 0x400
#define PC_OAM_SIZE  0x400
#define PC_IO_SIZE   0x400
#define PC_FLASH_SIZE 0x20000

extern unsigned char gPcIo[PC_IO_SIZE];      /* 0x04000000 I/O registers */
extern unsigned char gPcPltt[PC_PLTT_SIZE];  /* 0x05000000 palette RAM */
extern unsigned char gPcVram[PC_VRAM_SIZE];  /* 0x06000000 VRAM */
/* PC extension: text BGs with their bit set in gPcBgWide are 512 pixels wide;
 * their right-hand screenblock lives in gPcVramWide[bg] (the GBA's 64 KB of BG
 * VRAM has no room for it). Used by the widescreen field. */
extern unsigned char gPcVramWide[4][0x800];
extern unsigned char gPcBgWide;
extern unsigned char gPcOam[PC_OAM_SIZE];    /* 0x07000000 OAM */
extern unsigned char gPcFlash[PC_FLASH_SIZE];/* 0x0E000000 flash save chip */

/* Words the BIOS keeps at the top of IWRAM. */
extern void *gPcSoundInfoPtr;      /* 0x03007FF0 */
extern unsigned short gPcIntrCheck;/* 0x03007FF8 */
extern void *gPcIntrVector;        /* 0x03007FFC */

/* DMA register writes go through here: immediate channels run at once,
 * HBlank/VBlank-timed channels are remembered for the renderer. */
void PcDmaSet(int channel, const void *src, void *dest, unsigned int control);
void PcDmaStop(int channel);

/* Give the host a chance to run (draw, deliver interrupts) until any of the
 * interrupt flags in `mask` has been raised by a handler. */
void PcWaitForInterrupt(unsigned short mask);

/* Call from a "still busy?" poll. On the GBA, code that spins on such a poll
 * is released by an interrupt (VBlank drains the DMA queue, etc.); here the
 * game only yields at VBlank waits. A poll that keeps failing many times in
 * one frame is a spin, so this lets the frame end. One poll per frame (the
 * normal "check again next frame" pattern) never yields. */
void PcSpinWait(void);

/* Test hook (--warp GROUP NUM X Y): the overworld warps the player there as
 * soon as they can move. */
struct PcDebugWarp { int pending, group, num, x, y; };
extern struct PcDebugWarp gPcDebugWarp;
/* Test hook (--give-mon SPECIES LEVEL, repeatable up to 6): added to the
 * party the same way. */
struct PcDebugGiveMon { int species, level; };
extern struct PcDebugGiveMon gPcDebugGiveMon[6];
/* Test hook (--give-item ITEM COUNT): added to the bag the same way. */
struct PcDebugGiveItem { int item, count; };
extern struct PcDebugGiveItem gPcDebugGiveItem;
/* Online test hooks: --net-autojoin (register/login, join server 0),
 * --net-request KIND (ask the first trainer on the map), --net-accept. */
struct PcDebugNet { int autojoin, request_kind, accept, travel; char chat[64]; };  /* --net-chat TEXT */
extern struct PcDebugNet gPcDebugNet;

#endif
