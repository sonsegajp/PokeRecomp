/* dma.c — the four GBA DMA channels.
 *
 * Immediate transfers happen inside PcDmaSet. Channels started on HBlank or
 * VBlank are remembered and replayed by the frame loop at those moments
 * (HBlank DMA is how the game's scanline effects reach the display
 * registers). Sound-FIFO transfers (channels 1/2, "special" timing) are not
 * needed: our mixer reads the sound engine directly. */
#include <stdint.h>
#include <string.h>

#include "pc_hw.h"
#include "pc_port.h"

#define DMA_DEST_MASK   0x0060
#define DMA_DEST_INC    0x0000
#define DMA_DEST_DEC    0x0020
#define DMA_DEST_FIXED  0x0040
#define DMA_DEST_RELOAD 0x0060
#define DMA_SRC_MASK    0x0180
#define DMA_SRC_DEC     0x0080
#define DMA_SRC_FIXED   0x0100
#define DMA_REPEAT      0x0200
#define DMA_32BIT       0x0400
#define DMA_START_MASK  0x3000
#define DMA_START_NOW     0x0000
#define DMA_START_VBLANK  0x1000
#define DMA_START_HBLANK  0x2000
#define DMA_START_SPECIAL 0x3000
#define DMA_ENABLE      0x8000

struct Channel {
    uintptr_t src, dest, dest_reload;
    uint32_t count;     /* units per transfer */
    uint16_t control;
    int armed;          /* waiting for its HBlank/VBlank trigger */
};

static struct Channel channels[4];

static void transfer(struct Channel *ch)
{
    int wide = (ch->control & DMA_32BIT) != 0;
    int unit = wide ? 4 : 2;
    intptr_t src_step = unit, dest_step = unit;

    switch (ch->control & DMA_SRC_MASK) {
    case DMA_SRC_DEC:   src_step = -unit; break;
    case DMA_SRC_FIXED: src_step = 0; break;
    }
    switch (ch->control & DMA_DEST_MASK) {
    case DMA_DEST_DEC:   dest_step = -unit; break;
    case DMA_DEST_FIXED: dest_step = 0; break;
    }

    for (uint32_t i = 0; i < ch->count; i++) {
        if (wide)
            *(uint32_t *)ch->dest = *(const uint32_t *)ch->src;
        else
            *(uint16_t *)ch->dest = *(const uint16_t *)ch->src;
        ch->src += src_step;
        ch->dest += dest_step;
    }

    if ((ch->control & DMA_DEST_MASK) == DMA_DEST_RELOAD)
        ch->dest = ch->dest_reload;
}

static void finish(int n)
{
    struct Channel *ch = &channels[n];

    if ((ch->control & DMA_START_MASK) != DMA_START_NOW && (ch->control & DMA_REPEAT))
        return; /* stays armed for the next trigger */
    ch->armed = 0;
    ch->control &= ~DMA_ENABLE;
    *(uint16_t *)&gPcIo[0xBA + n * 12] = ch->control;
}

void PcDmaSet(int n, const void *src, void *dest, unsigned int control)
{
    struct Channel *ch = &channels[n & 3];
    uint32_t count = control & 0xFFFF;

    n &= 3;
    *(uint32_t *)&gPcIo[0xB0 + n * 12] = (uint32_t)(uintptr_t)src;
    *(uint32_t *)&gPcIo[0xB4 + n * 12] = (uint32_t)(uintptr_t)dest;
    *(uint32_t *)&gPcIo[0xB8 + n * 12] = control;

    ch->src = (uintptr_t)src;
    ch->dest = ch->dest_reload = (uintptr_t)dest;
    ch->control = (uint16_t)(control >> 16);
    ch->count = count ? count : (n == 3 ? 0x10000 : 0x4000);
    ch->armed = 0;

    if (!(ch->control & DMA_ENABLE))
        return;
    switch (ch->control & DMA_START_MASK) {
    case DMA_START_NOW:
        transfer(ch);
        finish(n);
        break;
    case DMA_START_VBLANK:
    case DMA_START_HBLANK:
        ch->armed = 1;
        break;
    case DMA_START_SPECIAL:
        break; /* sound FIFO / video capture: handled elsewhere or unused */
    }
}

void PcDmaStop(int n)
{
    n &= 3;
    channels[n].armed = 0;
    channels[n].control &= ~(DMA_ENABLE | DMA_REPEAT | DMA_START_MASK);
    *(uint16_t *)&gPcIo[0xBA + n * 12] = channels[n].control;
}

static void run_timed(uint16_t timing)
{
    for (int n = 0; n < 4; n++) {
        struct Channel *ch = &channels[n];
        if (ch->armed && (ch->control & DMA_START_MASK) == timing) {
            transfer(ch);
            finish(n);
        }
    }
}

void PcDmaOnHBlank(void) { run_timed(DMA_START_HBLANK); }
void PcDmaOnVBlank(void) { run_timed(DMA_START_VBLANK); }
