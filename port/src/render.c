/* render.c — the GBA picture, one scanline at a time.
 *
 * Reads the same VRAM, palette RAM, OAM and display registers the console's
 * PPU does and produces RGBA pixels. Between lines it calls back into the
 * frame loop so the game's HBlank interrupt and HBlank DMA can change
 * registers mid-frame exactly as on hardware (wave effects, battle intros).
 *
 * The output can be wider than 240 pixels: columns left/right of the GBA
 * screen are drawn from the same backgrounds and sprite coordinates
 * (`x_origin` is where screen x = 0 lands). That is the basis of widescreen.
 *
 * Supported: modes 0-2 (text + affine BGs), modes 3-5 (bitmaps), regular and
 * affine sprites (incl. double size), OBJ window, WIN0/WIN1, alpha blending,
 * brighten/darken, semi-transparent sprites, BG and OBJ mosaic. */
#include <stdint.h>
#include <string.h>

#include "pc_hw.h"
#include "pc_port.h"

#define MAX_WIDTH 1024

#define REG16(off) (*(const uint16_t *)&gPcIo[off])
#define REG32(off) (*(const uint32_t *)&gPcIo[off])

enum { LAYER_BG0, LAYER_BG1, LAYER_BG2, LAYER_BG3, LAYER_OBJ, LAYER_BD };

/* One candidate pixel of a layer: 15-bit colour, or transparent. */
#define TRANSPARENT 0x8000

static uint16_t bg_line[4][MAX_WIDTH];
static uint16_t obj_color[MAX_WIDTH];
static uint8_t obj_prio[MAX_WIDTH];
static uint8_t obj_semi[MAX_WIDTH];
static uint8_t obj_window[MAX_WIDTH];

/* Affine reference points: latched at the start of the frame and on writes,
 * advanced by PB/PD every line. */
static int32_t affine_x[2], affine_y[2];
static uint32_t affine_reg_x[2], affine_reg_y[2];

static inline uint16_t pal_bg(int i) { return ((const uint16_t *)gPcPltt)[i] & 0x7FFF; }
static inline uint16_t pal_obj(int i) { return ((const uint16_t *)gPcPltt)[256 + i] & 0x7FFF; }

static int32_t sign28(uint32_t v) { return (int32_t)(v << 4) >> 4; }

/* ---- backgrounds ----------------------------------------------------------- */

static void draw_text_bg(int bg, int line, int x_origin, int width)
{
    uint16_t cnt = REG16(0x08 + bg * 2);
    int char_base = ((cnt >> 2) & 3) * 0x4000;
    int screen_base = ((cnt >> 8) & 0x1F) * 0x800;
    int color256 = (cnt >> 7) & 1;
    int size = cnt >> 14;
    int map_w = (size & 1) ? 512 : 256;
    int map_h = (size & 2) ? 512 : 256;
    int hofs = REG16(0x10 + bg * 4) & 0x1FF;
    int vofs = REG16(0x12 + bg * 4) & 0x1FF;
    uint16_t mosaic = REG16(0x4C);
    int mos_h = (cnt & 0x40) ? (mosaic & 0xF) + 1 : 1;
    int mos_v = (cnt & 0x40) ? ((mosaic >> 4) & 0xF) + 1 : 1;
    int y = ((line - line % mos_v) + vofs) & (map_h - 1);
    uint16_t *out = bg_line[bg];

    for (int sx = 0; sx < width; sx++) {
        int gx = sx - x_origin;
        int mx = gx - ((gx % mos_h) + mos_h) % mos_h;
        int x = (mx + hofs) & (map_w - 1);
        int block = (x >> 8) + (y >> 8) * (map_w >> 8);
        const uint16_t *map = (const uint16_t *)&gPcVram[screen_base + block * 0x800];
        uint16_t entry = map[((y & 0xFF) >> 3) * 32 + ((x & 0xFF) >> 3)];
        int tx = x & 7, ty = y & 7;
        int index;

        if (entry & 0x400) tx = 7 - tx;
        if (entry & 0x800) ty = 7 - ty;
        if (color256) {
            int addr = char_base + (entry & 0x3FF) * 64 + ty * 8 + tx;
            index = addr < 0x10000 ? gPcVram[addr] : 0;
            out[sx] = index ? pal_bg(index) : TRANSPARENT;
        } else {
            int addr = char_base + (entry & 0x3FF) * 32 + ty * 4 + (tx >> 1);
            index = addr < 0x10000 ? (gPcVram[addr] >> ((tx & 1) * 4)) & 0xF : 0;
            out[sx] = index ? pal_bg((entry >> 12) * 16 + index) : TRANSPARENT;
        }
    }
}

static void draw_affine_bg(int bg, int x_origin, int width)
{
    int a = bg - 2;
    uint16_t cnt = REG16(0x08 + bg * 2);
    int char_base = ((cnt >> 2) & 3) * 0x4000;
    int screen_base = ((cnt >> 8) & 0x1F) * 0x800;
    int size = 128 << (cnt >> 14);
    int wrap = (cnt >> 13) & 1;
    int16_t pa = (int16_t)REG16(0x20 + a * 0x10);
    int16_t pc = (int16_t)REG16(0x24 + a * 0x10);
    int32_t x = affine_x[a] - pa * x_origin, y = affine_y[a] - pc * x_origin;
    uint16_t *out = bg_line[bg];

    for (int sx = 0; sx < width; sx++, x += pa, y += pc) {
        int px = x >> 8, py = y >> 8;
        if (wrap) {
            px &= size - 1;
            py &= size - 1;
        } else if (px < 0 || py < 0 || px >= size || py >= size) {
            out[sx] = TRANSPARENT;
            continue;
        }
        int tile = gPcVram[screen_base + (py >> 3) * (size >> 3) + (px >> 3)];
        int addr = char_base + tile * 64 + (py & 7) * 8 + (px & 7);
        int index = addr < 0x10000 ? gPcVram[addr] : 0;
        out[sx] = index ? pal_bg(index) : TRANSPARENT;
    }
}

static void draw_bitmap_bg(int mode, int line, int x_origin, int width)
{
    int frame = (REG16(0x00) >> 4) & 1;
    uint16_t *out = bg_line[2];

    for (int sx = 0; sx < width; sx++) {
        int x = sx - x_origin;
        out[sx] = TRANSPARENT;
        if (mode == 3 && x >= 0 && x < 240) {
            out[sx] = ((const uint16_t *)gPcVram)[line * 240 + x] & 0x7FFF;
        } else if (mode == 4 && x >= 0 && x < 240) {
            int index = gPcVram[frame * 0xA000 + line * 240 + x];
            out[sx] = index ? pal_bg(index) : TRANSPARENT;
        } else if (mode == 5 && x >= 0 && x < 160 && line < 128) {
            out[sx] = ((const uint16_t *)&gPcVram[frame * 0xA000])[line * 160 + x] & 0x7FFF;
        }
    }
}

/* ---- sprites ------------------------------------------------------------------ */

static const uint8_t obj_dims[3][4][2] = {
    {{8, 8}, {16, 16}, {32, 32}, {64, 64}},   /* square */
    {{16, 8}, {32, 8}, {32, 16}, {64, 32}},   /* horizontal */
    {{8, 16}, {8, 32}, {16, 32}, {32, 64}},   /* vertical */
};

static void draw_objs(int line, int x_origin, int width)
{
    uint16_t dispcnt = REG16(0x00);
    int one_d = (dispcnt >> 6) & 1;
    int bitmap_mode = (dispcnt & 7) >= 3;
    uint16_t mosaic = REG16(0x4C);
    int mos_h = ((mosaic >> 8) & 0xF) + 1, mos_v = ((mosaic >> 12) & 0xF) + 1;

    for (int i = 0; i < width; i++) {
        obj_color[i] = TRANSPARENT;
        obj_prio[i] = 4;
        obj_semi[i] = 0;
        obj_window[i] = 0;
    }
    if (!(dispcnt & 0x1000))
        return;

    /* Lower OAM index wins, so draw from the back. */
    for (int n = 127; n >= 0; n--) {
        const uint16_t *o = (const uint16_t *)&gPcOam[n * 8];
        uint16_t a0 = o[0], a1 = o[1], a2 = o[2];
        int affine = (a0 >> 8) & 1;
        int shape = a0 >> 14, size = a1 >> 14;

        if (!affine && (a0 & 0x200)) continue; /* disabled */
        if (shape == 3) continue;
        int w = obj_dims[shape][size][0], h = obj_dims[shape][size][1];
        int bw = w, bh = h;
        if (affine && (a0 & 0x200)) { bw *= 2; bh *= 2; } /* double size */

        int y = a0 & 0xFF;
        if (y >= 160) y -= 256;
        int row = line - y;
        if (row < 0 || row >= bh) continue;
        int x = a1 & 0x1FF;
        if (x >= 256) x -= 512;

        int mode = (a0 >> 10) & 3;
        int color256 = (a0 >> 13) & 1;
        int mos = (a0 >> 12) & 1;
        int tile = a2 & 0x3FF, prio = (a2 >> 10) & 3, pal = a2 >> 12;
        if (bitmap_mode && tile < 512) continue;
        int16_t pa = 0x100, pb = 0, pc = 0, pd = 0x100;
        if (affine) {
            const int16_t *p = (const int16_t *)gPcOam;
            int g = (a1 >> 9) & 0x1F;
            pa = p[g * 16 + 3]; pb = p[g * 16 + 7]; pc = p[g * 16 + 11]; pd = p[g * 16 + 15];
        }
        if (mos) row -= row % mos_v;
        int row_stride = one_d ? (w / 8) * (color256 ? 2 : 1) : 32;

        for (int c = 0; c < bw; c++) {
            int sx = x + c + x_origin;
            if (sx < 0 || sx >= width) continue;
            int tx, ty;
            int cc = mos ? c - (c % mos_h) : c;
            if (affine) {
                int dx = cc - bw / 2, dy = row - bh / 2;
                tx = ((pa * dx + pb * dy) >> 8) + w / 2;
                ty = ((pc * dx + pd * dy) >> 8) + h / 2;
                if (tx < 0 || ty < 0 || tx >= w || ty >= h) continue;
            } else {
                tx = (a1 & 0x1000) ? w - 1 - cc : cc;
                ty = (a1 & 0x2000) ? h - 1 - row : row;
            }
            int t = tile + (ty / 8) * row_stride + (tx / 8) * (color256 ? 2 : 1);
            int index;
            if (color256) {
                int addr = 0x10000 + (t & 0x3FF) * 32 + (ty & 7) * 8 + (tx & 7);
                index = gPcVram[addr & 0x17FFF];
            } else {
                int addr = 0x10000 + (t & 0x3FF) * 32 + (ty & 7) * 4 + ((tx & 7) >> 1);
                index = (gPcVram[addr & 0x17FFF] >> ((tx & 1) * 4)) & 0xF;
            }
            if (!index) continue;
            if (mode == 2) {
                obj_window[sx] = 1;
                continue;
            }
            if (prio <= obj_prio[sx] || obj_color[sx] == TRANSPARENT) {
                obj_color[sx] = color256 ? pal_obj(index) : pal_obj(pal * 16 + index);
                obj_prio[sx] = prio;
                obj_semi[sx] = mode == 1;
            }
        }
    }
}

/* ---- composition --------------------------------------------------------------- */

static int win_contains(int win, int x, int line)
{
    uint16_t h = REG16(0x40 + win * 2), v = REG16(0x44 + win * 2);
    int x1 = h >> 8, x2 = h & 0xFF, y1 = v >> 8, y2 = v & 0xFF;
    int inx, iny;

    if (x2 > 240 || x1 > x2) x2 = 240;
    inx = x1 <= x2 ? (x >= x1 && x < x2) : (x >= x1 || x < x2);
    if (y2 > 160 || y1 > y2) iny = (y1 <= y2) ? (line >= y1 && line < 160) : (line >= y1 || line < y2);
    else iny = line >= y1 && line < y2;
    return inx && iny;
}

static inline uint32_t to_rgba(uint16_t c)
{
    uint32_t r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return 0xFF000000u | (b << 16) | (g << 8) | r;
}

static inline uint16_t blend(uint16_t a, uint16_t b, int eva, int evb)
{
    int r = ((a & 0x1F) * eva + (b & 0x1F) * evb) >> 4;
    int g = (((a >> 5) & 0x1F) * eva + ((b >> 5) & 0x1F) * evb) >> 4;
    int bl = (((a >> 10) & 0x1F) * eva + ((b >> 10) & 0x1F) * evb) >> 4;
    if (r > 31) r = 31;
    if (g > 31) g = 31;
    if (bl > 31) bl = 31;
    return (uint16_t)(r | (g << 5) | (bl << 10));
}

static inline uint16_t brighten(uint16_t c, int evy)
{
    int r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    r += ((31 - r) * evy) >> 4;
    g += ((31 - g) * evy) >> 4;
    b += ((31 - b) * evy) >> 4;
    return (uint16_t)(r | (g << 5) | (b << 10));
}

static inline uint16_t darken(uint16_t c, int evy)
{
    int r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    r -= (r * evy) >> 4;
    g -= (g * evy) >> 4;
    b -= (b * evy) >> 4;
    return (uint16_t)(r | (g << 5) | (b << 10));
}

static void compose_line(uint32_t *out, int line, int x_origin, int width, int bg_enabled)
{
    uint16_t dispcnt = REG16(0x00);
    uint16_t bldcnt = REG16(0x50), bldalpha = REG16(0x52);
    int eva = bldalpha & 0x1F, evb = (bldalpha >> 8) & 0x1F, evy = REG16(0x54) & 0x1F;
    int windows = (dispcnt >> 13) & 7;
    uint16_t winin = REG16(0x48), winout = REG16(0x4A);
    uint16_t backdrop = pal_bg(0);
    int bg_prio[4];

    if (eva > 16) eva = 16;
    if (evb > 16) evb = 16;
    if (evy > 16) evy = 16;
    for (int bg = 0; bg < 4; bg++)
        bg_prio[bg] = REG16(0x08 + bg * 2) & 3;

    for (int sx = 0; sx < width; sx++) {
        int x = sx - x_origin;
        int mask = 0x3F; /* layers BG0-3, OBJ, effects */

        if (windows) {
            /* Columns outside the GBA screen use the "outside" setting. */
            int gx = x < 0 ? 0 : x > 239 ? 239 : x;
            if ((windows & 1) && win_contains(0, gx, line)) mask = winin & 0x3F;
            else if ((windows & 2) && win_contains(1, gx, line)) mask = (winin >> 8) & 0x3F;
            else if ((windows & 4) && obj_window[sx]) mask = (winout >> 8) & 0x3F;
            else mask = winout & 0x3F;
        }

        /* Find the top two visible layers. */
        uint16_t top = backdrop, below = backdrop;
        int top_layer = LAYER_BD, below_layer = LAYER_BD, found = 0;
        for (int prio = 0; prio < 4 && found < 2; prio++) {
            if ((mask & 0x10) && obj_color[sx] != TRANSPARENT && obj_prio[sx] == prio) {
                if (found == 0) { top = obj_color[sx]; top_layer = LAYER_OBJ; }
                else { below = obj_color[sx]; below_layer = LAYER_OBJ; }
                found++;
            }
            for (int bg = 0; bg < 4 && found < 2; bg++) {
                if (!(bg_enabled & (1 << bg)) || !(mask & (1 << bg)) || bg_prio[bg] != prio)
                    continue;
                if (bg_line[bg][sx] == TRANSPARENT) continue;
                if (found == 0) { top = bg_line[bg][sx]; top_layer = bg; }
                else { below = bg_line[bg][sx]; below_layer = bg; }
                found++;
            }
        }

        uint16_t color = top;
        int effect = (bldcnt >> 6) & 3;
        int top_is_target1 = (bldcnt >> top_layer) & 1;
        int below_is_target2 = (bldcnt >> (8 + below_layer)) & 1;

        if (top_layer == LAYER_OBJ && obj_semi[sx] && below_is_target2) {
            color = blend(top, below, eva, evb); /* semi-transparent OBJ: always alpha */
        } else if (mask & 0x20) {
            if (effect == 1 && top_is_target1 && below_is_target2)
                color = blend(top, below, eva, evb);
            else if (effect == 2 && top_is_target1)
                color = brighten(top, evy);
            else if (effect == 3 && top_is_target1)
                color = darken(top, evy);
        }
        out[sx] = to_rgba(color);
    }
}

void PcRenderFrame(uint32_t *out, int width, void (*hblank)(int line))
{
    int x_origin = (width - GBA_WIDTH) / 2;

    if (width > MAX_WIDTH) width = MAX_WIDTH;
    for (int a = 0; a < 2; a++) {
        affine_reg_x[a] = REG32(0x28 + a * 0x10);
        affine_reg_y[a] = REG32(0x2C + a * 0x10);
        affine_x[a] = sign28(affine_reg_x[a]);
        affine_y[a] = sign28(affine_reg_y[a]);
    }

    for (int line = 0; line < GBA_HEIGHT; line++) {
        uint16_t dispcnt = REG16(0x00);
        int mode = dispcnt & 7;
        uint32_t *row = out + line * width;

        *(uint16_t *)&gPcIo[0x06] = (uint16_t)line; /* VCOUNT */

        /* A write to the reference point re-latches it. */
        for (int a = 0; a < 2; a++) {
            if (REG32(0x28 + a * 0x10) != affine_reg_x[a]) {
                affine_reg_x[a] = REG32(0x28 + a * 0x10);
                affine_x[a] = sign28(affine_reg_x[a]);
            }
            if (REG32(0x2C + a * 0x10) != affine_reg_y[a]) {
                affine_reg_y[a] = REG32(0x2C + a * 0x10);
                affine_y[a] = sign28(affine_reg_y[a]);
            }
        }

        if (dispcnt & 0x80) { /* forced blank: white */
            for (int i = 0; i < width; i++) row[i] = 0xFFFFFFFFu;
        } else {
            int enabled = (dispcnt >> 8) & 0xF;
            switch (mode) {
            case 0: enabled &= 0xF; break;
            case 1: enabled &= 0x7; break;
            case 2: enabled &= 0xC; break;
            default: enabled &= 0x4; break;
            }
            for (int bg = 0; bg < 4; bg++) {
                if (!(enabled & (1 << bg))) continue;
                if (mode >= 3)
                    draw_bitmap_bg(mode, line, x_origin, width);
                else if (mode == 0 || bg < 2)
                    draw_text_bg(bg, line, x_origin, width); /* mode 0: all text; mode 1: BG0/1 */
                else
                    draw_affine_bg(bg, x_origin, width);    /* mode 1: BG2; mode 2: BG2/3 */
            }
            draw_objs(line, x_origin, width);
            compose_line(row, line, x_origin, width, enabled);
        }

        for (int a = 0; a < 2; a++) {
            affine_x[a] += (int16_t)REG16(0x22 + a * 0x10);
            affine_y[a] += (int16_t)REG16(0x26 + a * 0x10);
        }
        if (hblank)
            hblank(line);
    }
}
