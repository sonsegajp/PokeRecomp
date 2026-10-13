/* bios.c — the GBA BIOS services FireRed calls, in plain C.
 *
 * Behaviour follows the documented BIOS (copy/fill units, decompressor
 * formats, the affine maths with its 14-bit sine table and the ArcTan
 * polynomial), so results match the console bit for bit where the game can
 * observe them. */
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "pc_hw.h"
#include "pc_port.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t s16;
typedef int32_t s32;

#define CPUSET_FILL  (1u << 24)
#define CPUSET_32BIT (1u << 26)
#define CPUSET_COUNT 0x1FFFFFu

void CpuSet(const void *src, void *dest, u32 control)
{
    u32 count = control & CPUSET_COUNT;

    if (control & CPUSET_32BIT) {
        const u32 *s = (const u32 *)((uintptr_t)src & ~3u);
        u32 *d = (u32 *)((uintptr_t)dest & ~3u);
        if (control & CPUSET_FILL) {
            u32 v = *s;
            while (count--) *d++ = v;
        } else {
            while (count--) *d++ = *s++;
        }
    } else {
        const u16 *s = (const u16 *)((uintptr_t)src & ~1u);
        u16 *d = (u16 *)((uintptr_t)dest & ~1u);
        if (control & CPUSET_FILL) {
            u16 v = *s;
            while (count--) *d++ = v;
        } else {
            while (count--) *d++ = *s++;
        }
    }
}

/* 32-bit units in blocks of 8 words. */
void CpuFastSet(const void *src, void *dest, u32 control)
{
    u32 count = ((control & CPUSET_COUNT) + 7) & ~7u;
    const u32 *s = (const u32 *)((uintptr_t)src & ~3u);
    u32 *d = (u32 *)((uintptr_t)dest & ~3u);

    if (control & CPUSET_FILL) {
        u32 v = *s;
        while (count--) *d++ = v;
    } else {
        while (count--) *d++ = *s++;
    }
}

/* LZ77 (type 0x10): flag byte, then 8 blocks, each a literal byte or a
 * (length, distance) back-reference. The VRAM variant only differs in how
 * the console writes (16-bit units); the output is the same. */
static void lz77_decompress(const u8 *src, u8 *dest)
{
    u32 size = src[1] | (src[2] << 8) | (src[3] << 16);
    u8 *end = dest + size;

    src += 4;
    while (dest < end) {
        u8 flags = *src++;
        for (int i = 0; i < 8 && dest < end; i++, flags <<= 1) {
            if (flags & 0x80) {
                u32 len = (src[0] >> 4) + 3;
                u32 disp = (((src[0] & 0xF) << 8) | src[1]) + 1;
                src += 2;
                while (len-- && dest < end) {
                    *dest = *(dest - disp);
                    dest++;
                }
            } else {
                *dest++ = *src++;
            }
        }
    }
}

void LZ77UnCompWram(const void *src, void *dest) { lz77_decompress(src, dest); }
void LZ77UnCompVram(const void *src, void *dest) { lz77_decompress(src, dest); }

/* Run-length (type 0x30): flag bit 7 = run of (n & 0x7F) + 3 copies of one
 * byte, otherwise (n & 0x7F) + 1 literal bytes. */
static void rl_decompress(const u8 *src, u8 *dest)
{
    u32 size = src[1] | (src[2] << 8) | (src[3] << 16);
    u8 *end = dest + size;

    src += 4;
    while (dest < end) {
        u8 flag = *src++;
        if (flag & 0x80) {
            u32 len = (flag & 0x7F) + 3;
            u8 v = *src++;
            while (len-- && dest < end) *dest++ = v;
        } else {
            u32 len = (flag & 0x7F) + 1;
            while (len-- && dest < end) *dest++ = *src++;
        }
    }
}

void RLUnCompWram(const void *src, void *dest) { rl_decompress(src, dest); }
void RLUnCompVram(const void *src, void *dest) { rl_decompress(src, dest); }

/* ---- arithmetic ---------------------------------------------------------- */

s32 Div(s32 num, s32 denom)
{
    return denom ? num / denom : 0;
}

u16 Sqrt(u32 num)
{
    u32 r = 0, bit = 1u << 30;

    while (bit > num) bit >>= 2;
    while (bit) {
        if (num >= r + bit) {
            num -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (u16)r;
}

/* The BIOS arctangent: a fixed-point polynomial on |i| <= 1 (Q14 input). */
static s32 bios_arctan(s32 i)
{
    s32 a = -((i * i) >> 14);
    s32 b = ((0xA9 * a) >> 14) + 0x390;
    b = ((b * a) >> 14) + 0x91C;
    b = ((b * a) >> 14) + 0xFB6;
    b = ((b * a) >> 14) + 0x16AA;
    b = ((b * a) >> 14) + 0x2081;
    b = ((b * a) >> 14) + 0x3651;
    b = ((b * a) >> 14) + 0xA2F9;
    return (i * b) >> 16;
}

u16 ArcTan2(s16 x, s16 y)
{
    s32 r;

    if (y == 0)
        return x >= 0 ? 0 : 0x8000;
    if (x == 0)
        return y >= 0 ? 0x4000 : 0xC000;
    if (y >= 0) {
        if (x >= 0)
            r = x >= y ? bios_arctan((y << 14) / x) : 0x4000 - bios_arctan((x << 14) / y);
        else
            r = -x >= y ? 0x8000 + bios_arctan((y << 14) / x) : 0x4000 - bios_arctan((x << 14) / y);
    } else {
        if (x <= 0)
            r = -x > -y ? 0x8000 + bios_arctan((y << 14) / x) : 0xC000 - bios_arctan((x << 14) / y);
        else
            r = x >= -y ? 0x10000 + bios_arctan((y << 14) / x) : 0xC000 - bios_arctan((x << 14) / y);
    }
    return (u16)r;
}

/* ---- affine parameters ----------------------------------------------------
 * The BIOS looks angles up in a 256-entry sine table scaled to 0x4000. */

static s16 sine_table[256];

static void init_sine_table(void)
{
    static int ready;
    if (ready) return;
    for (int i = 0; i < 256; i++)
        sine_table[i] = (s16)lround(sin(i * 3.14159265358979323846 / 128.0) * 16384.0);
    ready = 1;
}

static s32 bios_sin(u16 angle) { return sine_table[angle >> 8]; }
static s32 bios_cos(u16 angle) { return sine_table[((angle >> 8) + 64) & 0xFF]; }

struct BiosBgAffineSrc {
    s32 tex_x, tex_y;   /* rotation centre in the BG, 19.8 */
    s16 scr_x, scr_y;   /* where it lands on screen */
    s16 sx, sy;         /* 8.8 scale */
    u16 alpha;          /* angle, 0-0xFFFF */
};

struct BiosBgAffineDst {
    s16 pa, pb, pc, pd;
    s32 dx, dy;
};

void BgAffineSet(const void *src_, void *dest_, s32 count)
{
    const struct BiosBgAffineSrc *src = src_;
    struct BiosBgAffineDst *dest = dest_;

    init_sine_table();
    for (; count > 0; count--, src++, dest++) {
        s32 s = bios_sin(src->alpha), c = bios_cos(src->alpha);
        s16 pa = (s16)((src->sx * c) >> 14);
        s16 pb = (s16)(-(src->sx * s) >> 14);
        s16 pc = (s16)((src->sy * s) >> 14);
        s16 pd = (s16)((src->sy * c) >> 14);
        dest->pa = pa;
        dest->pb = pb;
        dest->pc = pc;
        dest->pd = pd;
        dest->dx = src->tex_x - (pa * src->scr_x + pb * src->scr_y);
        dest->dy = src->tex_y - (pc * src->scr_x + pd * src->scr_y);
    }
}

struct BiosObjAffineSrc {
    s16 sx, sy;
    u16 alpha;
    u16 pad;
};

/* Writes pa, pb, pc, pd `offset` bytes apart (2 = packed, 8 = into OAM). */
void ObjAffineSet(const void *src_, void *dest, s32 count, s32 offset)
{
    const struct BiosObjAffineSrc *src = src_;
    u8 *d = dest;

    init_sine_table();
    for (; count > 0; count--, src++) {
        s32 s = bios_sin(src->alpha), c = bios_cos(src->alpha);
        *(s16 *)(d + 0 * offset) = (s16)((src->sx * c) >> 14);
        *(s16 *)(d + 1 * offset) = (s16)(-(src->sx * s) >> 14);
        *(s16 *)(d + 2 * offset) = (s16)((src->sy * s) >> 14);
        *(s16 *)(d + 3 * offset) = (s16)((src->sy * c) >> 14);
        d += 4 * offset;
    }
}

/* ---- system ------------------------------------------------------------------ */

#define RESET_EWRAM      0x01
#define RESET_IWRAM      0x02
#define RESET_PALETTE    0x04
#define RESET_VRAM       0x08
#define RESET_OAM        0x10
#define RESET_SIO_REGS   0x20
#define RESET_SOUND_REGS 0x40
#define RESET_REGS       0x80

/* Work RAM is ordinary program memory here, already zeroed at start-up, so
 * only the hardware areas are cleared. */
void RegisterRamReset(u32 flags)
{
    if (flags & RESET_PALETTE) memset(gPcPltt, 0, sizeof gPcPltt);
    if (flags & RESET_VRAM) memset(gPcVram, 0, sizeof gPcVram);
    if (flags & RESET_OAM) memset(gPcOam, 0, sizeof gPcOam);
    if (flags & RESET_REGS) {
        memset(gPcIo, 0, 0x60);         /* display */
        memset(gPcIo + 0xB0, 0, 0x60);  /* DMA, timers */
        memset(gPcIo + 0x200, 0, 0x10); /* IE, IF, WAITCNT, IME */
        gPcIo[0] = 0x80;                /* DISPCNT forced blank */
        /* The BIOS leaves the affine BGs at identity scale (PA = PD = 1.0);
         * screens like Oak's intro rely on it and never set them. */
        gPcIo[0x21] = gPcIo[0x27] = 0x01; /* BG2PA, BG2PD = 0x0100 */
        gPcIo[0x31] = gPcIo[0x37] = 0x01; /* BG3PA, BG3PD = 0x0100 */
    }
    if (flags & RESET_SOUND_REGS) memset(gPcIo + 0x60, 0, 0x50);
    if (flags & RESET_SIO_REGS) {
        /* serial registers, but not the keypad (0x130-0x133), which is input */
        memset(gPcIo + 0x120, 0, 0x10);
        memset(gPcIo + 0x134, 0, 0x2C);
    }
}

void SoftReset(u32 flags)
{
    (void)flags;
    PcRestartGame();
}

void VBlankIntrWait(void)
{
    gPcIntrCheck &= ~1;
    while (!(gPcIntrCheck & 1))
        PcWaitForInterrupt(1);
}
