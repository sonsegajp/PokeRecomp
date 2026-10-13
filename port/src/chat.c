/* chat.c — online text chat overlay.
 *
 * Lines are drawn with the game's own dialogue font (latin_normal glyphs from
 * text.c), one glyph pixel per overlay pixel on an overlay twice the game's
 * resolution — so it reads as the in-game font at half size. Recent messages
 * show for a while over the game; while typing, the whole log stays up. */
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "pc_chat.h"
#include "pc_controls.h"
#include "pc_net.h"
#include "pc_settings.h"

/* text.c (PC): glyph data for the game's normal latin font. */
const uint16_t *PcFontNormalGlyph(uint16_t glyphId, uint8_t *width);

#define EOS 0xFF
#define MAX_INPUT 60
#define LOG_LINES 8
#define SHOW_LINES 6
#define SHOW_MS 10000
#define FADE_MS 1000
#define LINE_H 16
#define MARGIN 6

#define COL_TEXT   0xFFFFFFFFu
#define COL_SHADOW 0xFF404040u
#define COL_NAME   0xFFF8C878u   /* 0xAABBGGRR: sky blue */
#define COL_SYSTEM 0xFF60D8F8u   /* gold: the server's announcements */
#define COL_GUILD  0xFF78F088u   /* green: guild chat */
#define COL_BOX    0x99000000u
#define COL_SELF   0xFF78E8F8u   /* yellow: your own name tag */
#define TAG_MS     100           /* tags not refreshed for this long are gone */
#define COL_INPUT  0xCC181818u

struct Line {
    uint8_t system;          /* from the server, not a player */
    uint8_t guild;           /* guild chat */
    uint8_t name[8];
    uint16_t tag;
    uint8_t text[MAX_INPUT + 1];
    ULONGLONG when;
};

static struct Line log_lines[LOG_LINES];
static int log_count;
static int input_open;
static uint8_t input[MAX_INPUT + 1];
static int input_len;
static int swallow_t;   /* the T that opened the line also arrives as text */
static uint32_t overlay[960 * 320];

static struct {
    int count;
    int16_t x[PC_CHAT_MAX_TAGS], y[PC_CHAT_MAX_TAGS];
    uint8_t name[PC_CHAT_MAX_TAGS][8];
    uint8_t self[PC_CHAT_MAX_TAGS];
    ULONGLONG when;
} tags;

void PcChatSetNameTags(int count, const int16_t *x, const int16_t *y,
                       const uint8_t (*names)[8], const uint8_t *self)
{
    if (count > PC_CHAT_MAX_TAGS) count = PC_CHAT_MAX_TAGS;
    tags.count = count;
    for (int i = 0; i < count; i++) {
        tags.x[i] = x[i];
        tags.y[i] = y[i];
        memcpy(tags.name[i], names[i], 8);
        tags.name[i][7] = EOS;
        tags.self[i] = self[i];
    }
    tags.when = GetTickCount64();
}

/* ---- charset ------------------------------------------------------------------ */

static uint8_t ascii_to_game(unsigned c)
{
    if (c >= 'A' && c <= 'Z') return 0xBB + (c - 'A');
    if (c >= 'a' && c <= 'z') return 0xD5 + (c - 'a');
    if (c >= '0' && c <= '9') return 0xA1 + (c - '0');
    switch (c) {
    case ' ': return 0x00;
    case '!': return 0xAB;
    case '?': return 0xAC;
    case '.': return 0xAD;
    case '-': return 0xAE;
    case ',': return 0xB8;
    case '/': return 0xBA;
    case '\'': return 0xB4;
    case '(': return 0x5C;
    case ')': return 0x5D;
    case '&': return 0x2D;
    case '+': return 0x2E;
    case '=': return 0x35;
    case ';': return 0x36;
    case '%': return 0x5B;
    case ':': return 0xF0;
    case '<': return 0x85;
    case '>': return 0x86;
    }
    return EOS;  /* not in the font: dropped */
}

/* ---- input ---------------------------------------------------------------------------- */

int PcChatActive(void) { return input_open; }

void PcChatOpen(void)
{
    if (!PcNetConnected()) return;
    input_open = 1;
    swallow_t = 1;
    input_len = 0;
    input[0] = EOS;
}

void PcChatTypeUtf8(const char *text)
{
    if (swallow_t && (text[0] == 't' || text[0] == 'T') && !text[1]) {
        swallow_t = 0;
        return;
    }
    swallow_t = 0;
    for (; *text && input_len < MAX_INPUT; text++) {
        uint8_t g = ascii_to_game((unsigned char)*text);
        if (g != EOS) input[input_len++] = g;
    }
    input[input_len] = EOS;
}

void PcChatBackspace(void)
{
    if (input_len > 0) input[--input_len] = EOS;
}

/* "/g text" (or "/G") goes to your guild, on every server */
void PcChatSubmit(void)
{
    if (input_len > 3 && input[0] == 0xBA && (input[1] == 0xDB || input[1] == 0xC1) && input[2] == 0x00)
        PcNetSendGuildChat(input + 3, input_len - 2);
    else if (input_len > 0)
        PcNetSendChat(input, input_len + 1);
    input_open = 0;
}

void PcChatCancel(void) { input_open = 0; }

void PcChatReceive(const uint8_t *name, uint16_t tag, const uint8_t *text)
{
    struct Line *l;
    int i;

    if (log_count == LOG_LINES) {
        memmove(log_lines, log_lines + 1, sizeof log_lines[0] * (LOG_LINES - 1));
        log_count--;
    }
    l = &log_lines[log_count++];
    l->system = name == NULL;
    l->guild = 0;
    if (name) memcpy(l->name, name, 8);
    l->tag = tag;
    for (i = 0; i < MAX_INPUT && text[i] != EOS; i++) l->text[i] = text[i];
    l->text[i] = EOS;
    l->when = GetTickCount64();
}

void PcChatReceiveGuild(const uint8_t *name, uint16_t tag, const uint8_t *text)
{
    PcChatReceive(name, tag, text);
    log_lines[log_count - 1].guild = 1;
}

/* ---- drawing -------------------------------------------------------------------------- */

static int ov_w, ov_h;

static void put(int x, int y, uint32_t c)
{
    if (x >= 0 && y >= 0 && x < ov_w && y < ov_h) overlay[y * ov_w + x] = c;
}

static void fill(int x, int y, int w, int h, uint32_t c)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            put(x + i, y + j, c);
}

static uint32_t fade(uint32_t c, unsigned alpha)  /* alpha 0..255 */
{
    return (c & 0x00FFFFFFu) | (((c >> 24) * alpha / 255) << 24);
}

/* Draw one glyph; returns its advance. */
static int glyph(int x, int y, uint8_t ch, uint32_t color, unsigned alpha)
{
    uint8_t width;
    const uint16_t *g = PcFontNormalGlyph(ch, &width);
    for (int t = 0; t < 4; t++) {
        const uint16_t *tile = g + t * 8;
        int tx = x + (t & 1) * 8, ty = y + (t >> 1) * 8;
        for (int r = 0; r < 8; r++) {
            uint16_t row = tile[r];
            for (int half = 0; half < 2; half++) {
                uint8_t b = half == 0 ? (uint8_t)(row >> 8) : (uint8_t)row;
                for (int n = 0; n < 4; n++) {
                    int v = (b >> ((3 - n) * 2)) & 3;   /* low bits = rightmost pixel (text_printer.c LUT) */
                    if (v == 1) put(tx + half * 4 + n, ty + r, fade(color, alpha));
                    else if (v == 2) put(tx + half * 4 + n, ty + r, fade(COL_SHADOW, alpha));
                }
            }
        }
    }
    return width;
}

static int text_width(const uint8_t *s)
{
    int w = 0;
    uint8_t width;
    for (; *s != EOS; s++) {
        PcFontNormalGlyph(*s, &width);
        w += width;
    }
    return w;
}

static int text(int x, int y, const uint8_t *s, uint32_t color, unsigned alpha)
{
    for (; *s != EOS; s++)
        x += glyph(x, y, *s, color, alpha);
    return x;
}

static const uint8_t sep[] = {0xF0, 0x00, EOS};      /* ": " */
static const uint8_t prompt[] = {0x86, 0x00, EOS};   /* "> " */
static const uint8_t caret[] = {0xAE, EOS};          /* "-" */

/* ASCII text in the game's font (characters it doesn't have are dropped) */
static int ascii_text(int x, int y, const char *s, uint32_t color)
{
    uint8_t buf[64];
    int n = 0;
    for (; *s && n < 63; s++) {
        uint8_t g = ascii_to_game((unsigned char)*s);
        if (g != EOS) buf[n++] = g;
    }
    buf[n] = EOS;
    return text(x, y, buf, color, 255);
}

#define COL_PANEL 0xE0181818u
#define COL_PICK  0xFF805820u   /* the selected cell */
#define COL_DIM   0xFFB0B0B0u

/* the controls panel: actions down the side, keyboard and controller columns */
static void draw_controls(int w, int h)
{
    int column, waiting, row = PcControlsCursor(&column, &waiting);
    int pw = 420, ph = (PC_BIND_COUNT + 4) * LINE_H + 12;
    int x0 = (w - pw) / 2, y0 = (h - ph) / 2, y;
    int cx[2] = {x0 + 150, x0 + 290};

    fill(x0, y0, pw, ph, COL_PANEL);
    y = y0 + 4;
    ascii_text(x0 + 10, y, "CONTROLS", COL_SYSTEM);
    ascii_text(cx[0], y, "Keyboard", COL_NAME);
    ascii_text(cx[1], y, "Controller", COL_NAME);
    y += LINE_H + 4;
    for (int i = 0; i < PC_BIND_COUNT; i++, y += LINE_H) {
        ascii_text(x0 + 10, y, PcControlsActionName(i), COL_TEXT);
        for (int c = 0; c < 2; c++) {
            const char *name = c ? PcControlsPadName(i) : PcControlsKeyName(i);
            if (i == row && c == column) {
                fill(cx[c] - 4, y, 132, LINE_H, COL_PICK);
                if (waiting) name = c ? "Press a button" : "Press a key";
            }
            ascii_text(cx[c], y, name, c == column && i == row ? COL_TEXT : COL_DIM);
        }
    }
    y += 6;
    ascii_text(x0 + 10, y, waiting ? (column ? "Press the new button. B: cancel" : "Press the new key. Esc: cancel")
                                   : "Enter / A: change    Del / Y: default", COL_DIM);
    ascii_text(x0 + 10, y + LINE_H, "Esc / F1 / B: close", COL_DIM);
}

const uint32_t *PcChatOverlay(int w, int h)
{
    ULONGLONG now = GetTickCount64();
    int shown = 0, y, i;

    if (w * h > (int)(sizeof overlay / sizeof overlay[0])) return NULL;
    ov_w = w;
    ov_h = h;
    memset(overlay, 0, (size_t)w * h * 4);

    if (PcControlsActive()) {
        draw_controls(w, h);
        return overlay;
    }

    /* name tags: the overlay is 2x the game, centered on the 240-wide screen */
    if (tags.count && now - tags.when < TAG_MS) {
        int x_origin = (w / 2 - 240) / 2;
        for (i = 0; i < tags.count; i++) {
            int tw = text_width(tags.name[i]);
            int tx = 2 * (tags.x[i] + x_origin) - tw / 2, ty = 2 * tags.y[i] - LINE_H;
            if (tx + tw < 0 || tx >= w || ty + LINE_H < 0 || ty >= h) continue;
            fill(tx - 2, ty + 1, tw + 4, LINE_H - 2, COL_BOX);
            text(tx, ty, tags.name[i], tags.self[i] ? COL_SELF : COL_NAME, 255);
            shown = 1;
        }
    }

    y = h - MARGIN - LINE_H;
    if (input_open) {
        int x;
        fill(MARGIN - 2, y - 1, w - 2 * (MARGIN - 2), LINE_H, COL_INPUT);
        x = text(MARGIN, y, prompt, COL_NAME, 255);
        x = text(x, y, input, COL_TEXT, 255);
        if ((now / 400) & 1) text(x, y, caret, COL_TEXT, 255);
        y -= LINE_H + 2;
        shown = 1;
    }
    for (i = log_count - 1; i >= 0 && log_count - i <= SHOW_LINES; i--) {
        struct Line *l = &log_lines[i];
        ULONGLONG age = now - l->when;
        unsigned alpha = 255;
        uint8_t name[20];
        int n, x, width;

        if (!input_open) {
            if (age >= SHOW_MS) break;
            if (age > SHOW_MS - FADE_MS) alpha = (unsigned)((SHOW_MS - age) * 255 / FADE_MS);
        }
        if (l->system) {
            fill(MARGIN - 2, y - 1, text_width(l->text) + 4, LINE_H, fade(COL_BOX, alpha));
            text(MARGIN, y, l->text, COL_SYSTEM, alpha);
            y -= LINE_H;
            shown = 1;
            continue;
        }
        for (n = 0; n < 7 && l->name[n] != EOS; n++) name[n] = l->name[n];
        name[n] = EOS;
        width = text_width(name) + text_width(sep) + text_width(l->text);
        fill(MARGIN - 2, y - 1, width + 4, LINE_H, fade(COL_BOX, alpha));
        x = text(MARGIN, y, name, l->guild ? COL_GUILD : COL_NAME, alpha);
        x = text(x, y, sep, COL_TEXT, alpha);
        text(x, y, l->text, l->guild ? COL_GUILD : COL_TEXT, alpha);
        y -= LINE_H;
        shown = 1;
    }
    return shown ? overlay : NULL;
}

void PcChatDebugShot(const uint32_t *frame, int w, int h, const char *path)
{
    const uint32_t *chat = PcChatOverlay(w * 2, h * 2);
    FILE *f;
    int W = w * 2, H = h * 2;

    if (!chat || !(f = fopen(path, "wb"))) return;
    {
        uint32_t size = 54 + (uint32_t)W * H * 4;
        uint8_t hdr[54] = {'B', 'M'};
        hdr[2] = size; hdr[3] = size >> 8; hdr[4] = size >> 16; hdr[5] = size >> 24;
        hdr[10] = 54; hdr[14] = 40;
        hdr[18] = W; hdr[19] = W >> 8;
        hdr[22] = H; hdr[23] = H >> 8;
        hdr[26] = 1; hdr[28] = 32;
        fwrite(hdr, 1, 54, f);
    }
    for (int y = H - 1; y >= 0; y--) {
        for (int x = 0; x < W; x++) {
            uint32_t g = frame[(y / 2) * w + x / 2], o = chat[y * W + x];
            unsigned a = o >> 24, out[3];
            for (int c = 0; c < 3; c++) {
                unsigned gc = (g >> (8 * c)) & 0xFF, oc = (o >> (8 * c)) & 0xFF;
                out[c] = (oc * a + gc * (255 - a)) / 255;
            }
            uint8_t px[4] = {(uint8_t)out[2], (uint8_t)out[1], (uint8_t)out[0], 0xFF};  /* BMP is BGRA */
            fwrite(px, 1, 4, f);
        }
    }
    fclose(f);
}
