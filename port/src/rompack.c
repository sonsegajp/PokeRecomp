/* rompack.c — the game's content comes from the player's own cartridge.
 *
 * A released game DLL has the ROM's content (graphics, maps, scripts, text,
 * music) taken out by tools/rompack.c, which appends a table saying where in
 * a retail ROM each removed run of bytes lives. Importing the player's ROM
 * (PokeRecomp_Extract, or on first run: --rom, a ROM next to the runner, or a
 * file dialog) checks its SHA-1 and unpacks those bytes into <game>.pkmn —
 * like Ship of Harkinian's oot.otr. Every launch then fills them back in
 * from the .pkmn before any game code runs; a checksum proves the result is
 * exactly what was taken out. A development build without the table runs
 * as is.
 *
 * A second, "foreign" layer holds the other region's player sprites, built in
 * for visitors (xgfx/): it comes from the player's copy of that game, into
 * <game>_<region>.pkmn (made when that ROM is imported). Without it the game
 * runs the same; a visitor just isn't drawn with their own sprites.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <commdlg.h>

#include "pc_rompack.h"

#if defined(EMERALD)
#define GAME_NAME L"Pokémon Emerald"
#define PACK_NAME L"emerald.pkmn"
#elif defined(LEAFGREEN)
#define GAME_NAME L"Pokémon LeafGreen"
#define PACK_NAME L"leafgreen.pkmn"
#else
#define GAME_NAME L"Pokémon FireRed"
#define PACK_NAME L"firered.pkmn"
#endif
#define PACK_MAGIC "PKMN\0\0\0\1"
#define XPACK_MAGIC "PKMX\0\0\0\1"

struct Range { uint32_t rva, len; };
struct Piece { uint32_t rva, len, rom; };
struct Table { uint8_t sha1[20]; uint32_t romsize, count; const struct Piece *pieces; };

static uint8_t *overlay;
static uint32_t nranges, ntables;
static uint64_t total, check;
static const struct Range *ranges;
static struct Table tables[4];
static int headless;

/* the foreign layer (PRPX), if the build has one */
static uint8_t *xoverlay;
static char xid[16];
static uint32_t xnranges, xntables;
static uint64_t xtotal, xcheck;
static const struct Range *xranges;
static struct Table xtables[4];
static int foreign_ready = 1;   /* no foreign layer: the sprites are inside */

/* ---- helpers ---------------------------------------------------------------------------- */

static uint32_t rol(uint32_t v, int n) { return v << n | v >> (32 - n); }

static void sha1(const uint8_t *data, size_t len, uint8_t out[20])
{
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    size_t blocks = (len + 8) / 64 + 1;
    for (size_t b = 0; b < blocks; b++) {
        uint8_t block[64];
        uint32_t w[80], a, bb, c, d, e;
        size_t off = b * 64;
        if (off + 64 <= len) memcpy(block, data + off, 64);
        else {
            memset(block, 0, 64);
            if (off < len) memcpy(block, data + off, len - off);
            if (off <= len) block[len - off] = 0x80;
            if (b == blocks - 1)
                for (int i = 0; i < 8; i++) block[56 + i] = (uint8_t)((uint64_t)len * 8 >> (56 - 8 * i));
        }
        for (int i = 0; i < 16; i++)
            w[i] = (uint32_t)block[4 * i] << 24 | block[4 * i + 1] << 16 | block[4 * i + 2] << 8 | block[4 * i + 3];
        for (int i = 16; i < 80; i++) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        a = h[0]; bb = h[1]; c = h[2]; d = h[3]; e = h[4];
        for (int i = 0; i < 80; i++) {
            uint32_t f, k, t;
            if (i < 20) f = (bb & c) | (~bb & d), k = 0x5A827999;
            else if (i < 40) f = bb ^ c ^ d, k = 0x6ED9EBA1;
            else if (i < 60) f = (bb & c) | (bb & d) | (c & d), k = 0x8F1BBCDC;
            else f = bb ^ c ^ d, k = 0xCA62C1D6;
            t = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(bb, 30); bb = a; a = t;
        }
        h[0] += a; h[1] += bb; h[2] += c; h[3] += d; h[4] += e;
    }
    for (int i = 0; i < 20; i++) out[i] = (uint8_t)(h[i / 4] >> (24 - 8 * (i % 4)));
}

static uint64_t fnv64(uint64_t h, const uint8_t *p, size_t n)
{
    while (n--) h = (h ^ *p++) * 0x100000001B3ull;
    return h;
}

static uint8_t *read_whole(const wchar_t *path, size_t *size)
{
    FILE *f = _wfopen(path, L"rb");
    uint8_t *buf;
    long n;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0 || !(buf = malloc((size_t)n))) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *size = (size_t)n;
    return buf;
}

static void complain(const wchar_t *text)
{
    if (headless) fwprintf(stderr, L"%ls\n", text);
    else MessageBoxW(NULL, text, L"PokeRecomp", MB_OK | MB_ICONERROR);
}

/* ---- the table appended to the exe ---------------------------------------------------------- */

static const uint8_t *parse_tables(const uint8_t *p, uint32_t n, struct Table *t)
{
    for (uint32_t i = 0; i < n && i < 4; i++) {
        memcpy(t[i].sha1, p, 20); p += 20;
        memcpy(&t[i].romsize, p, 4); p += 4;
        memcpy(&t[i].count, p, 4); p += 4;
        t[i].pieces = (const struct Piece *)p;
        p += (size_t)t[i].count * sizeof(struct Piece);
    }
    return p;
}

/* The overlay ending at `end` in the file, if it ends with `tail_magic`. */
static uint8_t *read_tail(FILE *f, long end, const char *tail_magic, const char *magic, uint32_t *size)
{
    uint8_t tail[12], *buf;
    if (end < 12 || fseek(f, end - 12, SEEK_SET) || fread(tail, 1, 12, f) != 12 || memcmp(tail + 4, tail_magic, 8))
        return NULL;
    memcpy(size, tail, 4);
    if (*size > (uint32_t)end || !(buf = malloc(*size))) return NULL;
    if (fseek(f, end - (long)*size, SEEK_SET) || fread(buf, 1, *size, f) != *size || memcmp(buf, magic, 4)) {
        free(buf);
        return NULL;
    }
    return buf;
}

static int read_overlay(const wchar_t *exe)
{
    FILE *f = _wfopen(exe, L"rb");
    long end;
    uint32_t size, xsize;
    const uint8_t *p;

    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    end = ftell(f);
    xoverlay = read_tail(f, end, "PRPXEND!", "PRPX", &xsize);
    if (xoverlay) {
        end -= (long)xsize;
        p = xoverlay + 4 + 4;                  /* magic, version */
        memcpy(xid, p, 16); p += 16;
        xid[15] = 0;
        memcpy(&xntables, p, 4); p += 4;
        memcpy(&xnranges, p, 4); p += 4;
        memcpy(&xtotal, p, 8); p += 8;
        memcpy(&xcheck, p, 8); p += 8;
        xranges = (const struct Range *)p;
        parse_tables(p + (size_t)xnranges * sizeof(struct Range), xntables, xtables);
    }
    overlay = read_tail(f, end, "PRPKEND!", "PRPK", &size);
    fclose(f);
    if (!overlay) return 0;
    p = overlay + 4 + 4;                       /* magic, version */
    memcpy(&ntables, p, 4); p += 4;
    memcpy(&nranges, p, 4); p += 4;
    memcpy(&total, p, 8); p += 8;
    memcpy(&check, p, 8); p += 8;
    ranges = (const struct Range *)p;
    parse_tables(p + (size_t)nranges * sizeof(struct Range), ntables, tables);
    return 1;
}

/* ---- the pack ------------------------------------------------------------------------------ */

static uint8_t *load_layer_pack(const wchar_t *path, const char *magic, uint64_t ltotal, uint64_t lcheck)
{
    size_t size;
    uint8_t *buf = read_whole(path, &size);
    uint64_t t, c;

    if (!buf) return NULL;
    if (size != 24 + ltotal || memcmp(buf, magic, 8)) { free(buf); return NULL; }
    memcpy(&t, buf + 8, 8);
    memcpy(&c, buf + 16, 8);
    if (t != ltotal || c != lcheck || fnv64(0xCBF29CE484222325ull, buf + 24, ltotal) != lcheck) { free(buf); return NULL; }
    return buf;
}

static uint8_t *load_pack(const wchar_t *path)
{
    return load_layer_pack(path, PACK_MAGIC, total, check);
}

static void (*sProgress)(int percent, void *ctx);
static void *sProgressCtx;

static void progress(int percent)
{
    if (sProgress) sProgress(percent, sProgressCtx);
}

/* A ROM -> a layer's pack bytes (NULL if it isn't one the layer knows). */
static uint8_t *unpack_layer(const uint8_t *rom, size_t size, const char *magic, const struct Table *ltables,
                             uint32_t lntables, uint64_t total, uint64_t check)
{
    uint8_t h[20], *pack, *out;
    const struct Table *tab = NULL;

    progress(45);
    sha1(rom, size, h);
    progress(60);
    for (uint32_t t = 0; t < lntables && t < 4; t++)
        if (!memcmp(h, ltables[t].sha1, 20) && size == ltables[t].romsize)
            tab = &ltables[t];
    if (!tab || !(pack = malloc(24 + total)))
        return NULL;
    memcpy(pack, magic, 8);
    memcpy(pack + 8, &total, 8);
    memcpy(pack + 16, &check, 8);
    out = pack + 24;
    for (uint32_t i = 0; i < tab->count; i++) {        /* pieces come in range order */
        const struct Piece *pc = &tab->pieces[i];
        if ((uint64_t)pc->rom + pc->len > size || out + pc->len > pack + 24 + total) { free(pack); return NULL; }
        memcpy(out, rom + pc->rom, pc->len);
        out += pc->len;
        if ((i & 2047) == 0) progress(60 + (int)(30ull * i / tab->count));
    }
    if (out != pack + 24 + total || fnv64(0xCBF29CE484222325ull, pack + 24, total) != check) { free(pack); return NULL; }
    return pack;
}

static uint8_t *unpack_rom(const uint8_t *rom, size_t size)
{
    return unpack_layer(rom, size, PACK_MAGIC, tables, ntables, total, check);
}

static uint8_t *try_rom(const wchar_t *path)
{
    size_t size;
    uint8_t *rom = read_whole(path, &size), *pack;
    if (!rom) return NULL;
    pack = size <= 32u << 20 ? unpack_rom(rom, size) : NULL;
    free(rom);
    return pack;
}

/* Any .gba in `dir` that is the right ROM. */
static uint8_t *find_rom(const wchar_t *dir)
{
    wchar_t pattern[MAX_PATH], path[MAX_PATH];
    WIN32_FIND_DATAW fd;
    HANDLE h;
    uint8_t *pack = NULL;

    _snwprintf(pattern, MAX_PATH, L"%ls\\*.gba", dir);
    h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    do {
        _snwprintf(path, MAX_PATH, L"%ls\\%ls", dir, fd.cFileName);
        pack = try_rom(path);
    } while (!pack && FindNextFileW(h, &fd));
    FindClose(h);
    return pack;
}

static uint8_t *ask_for_rom(void)
{
    wchar_t path[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {0};
    uint8_t *pack;

    MessageBoxW(NULL,
                L"PokeRecomp runs on your own " GAME_NAME L" (USA) cartridge.\n\n"
                L"Choose your ROM file. It is read once, to unpack the game's graphics, "
                L"maps, music and text into " PACK_NAME L"; after that the ROM isn't needed.",
                L"PokeRecomp", MB_OK | MB_ICONINFORMATION);
    for (;;) {
        ofn.lStructSize = sizeof ofn;
        ofn.lpstrFilter = L"Game Boy Advance ROM (*.gba)\0*.gba\0All files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = L"Choose your " GAME_NAME L" ROM";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (!GetOpenFileNameW(&ofn)) return NULL;
        if ((pack = try_rom(path)) != NULL) return pack;
        if (MessageBoxW(NULL,
                        L"That isn't a " GAME_NAME L" (USA) ROM this version of PokeRecomp knows "
                        L"(it checks the file's SHA-1). Choose another?",
                        L"PokeRecomp", MB_RETRYCANCEL | MB_ICONWARNING) != IDRETRY)
            return NULL;
    }
}

/* The game is a DLL (games/<id>.dll) run by PokeRecompGame.exe; the table and
 * the data it fills in belong to the DLL, not to the process. */
static HMODULE game_module(void)
{
    HMODULE m = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)(void *)game_module, &m);
    return m;
}

static int open_table(void)
{
    wchar_t path[MAX_PATH];
    if (overlay) return 1;
    return GetModuleFileNameW(game_module(), path, MAX_PATH) && read_overlay(path);
}

static int write_layer_pack(const wchar_t *path, const uint8_t *pack, uint64_t total)
{
    wchar_t tmp[MAX_PATH];
    FILE *f;
    int ok;

    _snwprintf(tmp, MAX_PATH, L"%ls.tmp", path);
    if ((f = _wfopen(tmp, L"wb")) == NULL) return 0;
    ok = fwrite(pack, 1, 24 + total, f) == 24 + total;
    ok &= fclose(f) == 0;
    if (ok && MoveFileExW(tmp, path, MOVEFILE_REPLACE_EXISTING)) return 1;
    DeleteFileW(tmp);
    return 0;
}

static int write_pack(const wchar_t *path, const uint8_t *pack)
{
    return write_layer_pack(path, pack, total);
}

static void apply_layer(const uint8_t *bytes, const struct Range *ranges, uint32_t nranges)
{
    uint8_t *base = (uint8_t *)game_module();

    for (uint32_t i = 0; i < nranges; i++) {
        DWORD old;
        uint8_t *dst = base + ranges[i].rva;
        VirtualProtect(dst, ranges[i].len, PAGE_READWRITE, &old);
        memcpy(dst, bytes, ranges[i].len);
        VirtualProtect(dst, ranges[i].len, old, &old);
        bytes += ranges[i].len;
    }
}

static void apply(const uint8_t *bytes)
{
    apply_layer(bytes, ranges, nranges);
}

const wchar_t *PcRomPackName(void) { return PACK_NAME; }

int PcRomPackForeignReady(void) { return foreign_ready; }

/* <game>_<region>.pkmn next to the runner: "firered_hoenn.pkmn" */
static void foreign_pack_path(wchar_t *out)
{
    wchar_t dir[MAX_PATH], base[64], *slash, *dot;
    GetModuleFileNameW(NULL, dir, MAX_PATH);
    if ((slash = wcsrchr(dir, L'\\')) != NULL) *slash = 0;
    wcsncpy(base, PACK_NAME, 63);
    base[63] = 0;
    if ((dot = wcsrchr(base, L'.')) != NULL) *dot = 0;
    _snwprintf(out, MAX_PATH, L"%ls\\%ls_%hs.pkmn", dir, base, xid);
}

/* The foreign layer from the other game's ROM. PC_ROMPACK_WRONG_ROM if it
 * isn't one (the runner offers every imported ROM to every game). */
int PcRomPackExtractForeign(const wchar_t *rom, const wchar_t *out)
{
    wchar_t path[MAX_PATH];
    size_t size;
    uint8_t *data, *pack;

    if (!open_table()) return PC_ROMPACK_NO_TABLE;
    if (!xoverlay) return PC_ROMPACK_NO_TABLE;
    if (!(data = read_whole(rom, &size))) return PC_ROMPACK_CANT_READ;
    pack = size <= 32u << 20 ? unpack_layer(data, size, XPACK_MAGIC, xtables, xntables, xtotal, xcheck) : NULL;
    free(data);
    if (!pack) return PC_ROMPACK_WRONG_ROM;
    if (!out) {
        foreign_pack_path(path);
        out = path;
    }
    if (!write_layer_pack(out, pack, xtotal)) { free(pack); return PC_ROMPACK_CANT_WRITE; }
    free(pack);
    return PC_ROMPACK_OK;
}

/* The foreign layer at boot: from its pack, else from that game's ROM if
 * one is next to the runner. */
static void load_foreign(const wchar_t *dir)
{
    wchar_t path[MAX_PATH], pattern[MAX_PATH];
    WIN32_FIND_DATAW fd;
    HANDLE h;
    uint8_t *pack;

    if (!xoverlay) return;
    foreign_ready = 0;
    foreign_pack_path(path);
    pack = load_layer_pack(path, XPACK_MAGIC, xtotal, xcheck);
    if (!pack) {
        _snwprintf(pattern, MAX_PATH, L"%ls\\*.gba", dir);
        if ((h = FindFirstFileW(pattern, &fd)) != INVALID_HANDLE_VALUE) {
            do {
                wchar_t rom[MAX_PATH];
                _snwprintf(rom, MAX_PATH, L"%ls\\%ls", dir, fd.cFileName);
                if (PcRomPackExtractForeign(rom, NULL) == PC_ROMPACK_OK)
                    pack = load_layer_pack(path, XPACK_MAGIC, xtotal, xcheck);
            } while (!pack && FindNextFileW(h, &fd));
            FindClose(h);
        }
    }
    if (!pack) return;
    apply_layer(pack + 24, xranges, xnranges);
    free(pack);
    foreign_ready = 1;
}

int PcRomPackExtractProgress(const wchar_t *rom, const wchar_t *out, void (*cb)(int percent, void *ctx), void *ctx)
{
    uint8_t *data, *pack;
    long size, done = 0;
    FILE *f;

    sProgress = cb;
    sProgressCtx = ctx;
    progress(0);
    if (!open_table()) return PC_ROMPACK_NO_TABLE;
    if ((f = _wfopen(rom, L"rb")) == NULL) return PC_ROMPACK_CANT_READ;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > 32l << 20 || (data = malloc((size_t)size)) == NULL) { fclose(f); return PC_ROMPACK_WRONG_ROM; }
    while (done < size) {                       /* reading the cartridge: 0-40% */
        size_t n = fread(data + done, 1, (size_t)(size - done < (1 << 20) ? size - done : 1 << 20), f);
        if (n == 0) { fclose(f); free(data); return PC_ROMPACK_CANT_READ; }
        done += (long)n;
        progress((int)(40ll * done / size));
    }
    fclose(f);
    pack = unpack_rom(data, (size_t)size);      /* checking it, unpacking: 45-90% */
    free(data);
    if (pack == NULL) { sProgress = NULL; return PC_ROMPACK_WRONG_ROM; }
    progress(92);
    if (!write_pack(out, pack)) { free(pack); sProgress = NULL; return PC_ROMPACK_CANT_WRITE; }
    free(pack);
    progress(100);
    sProgress = NULL;
    return PC_ROMPACK_OK;
}

int PcRomPackExtract(const wchar_t *rom, const wchar_t *out)
{
    return PcRomPackExtractProgress(rom, out, NULL, NULL);
}

int PcRomPackLoad(int argc, char **argv)
{
    wchar_t dir[MAX_PATH], pack_path[MAX_PATH], *slash;
    const char *rom_arg = NULL, *pack_arg = NULL;
    uint8_t *pack;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--headless")) headless = 1;
        else if (!strcmp(argv[i], "--rom") && i + 1 < argc) rom_arg = argv[++i];
        else if (!strcmp(argv[i], "--pkmn") && i + 1 < argc) pack_arg = argv[++i];
    }
    if (!open_table())
        return 1;                           /* a build with its data still inside */

    /* the pack: --pkmn, else next to the program that runs the game */
    GetModuleFileNameW(NULL, dir, MAX_PATH);
    if ((slash = wcsrchr(dir, L'\\')) != NULL) *slash = 0;
    if (pack_arg) MultiByteToWideChar(CP_UTF8, 0, pack_arg, -1, pack_path, MAX_PATH);
    else _snwprintf(pack_path, MAX_PATH, L"%ls\\%ls", dir, PACK_NAME);

    pack = load_pack(pack_path);
    if (!pack) {
        if (rom_arg) {
            wchar_t wrom[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, rom_arg, -1, wrom, MAX_PATH);
            pack = try_rom(wrom);
        }
        if (!pack) pack = find_rom(dir);
        if (!pack && !headless) pack = ask_for_rom();
        if (!pack) {
            complain(L"PokeRecomp needs " PACK_NAME L", made from your own " GAME_NAME
                     L" (USA) ROM (import it in the launcher, or pass --rom).");
            return 0;
        }
        write_pack(pack_path, pack);
    }
    apply(pack + 24);
    free(pack);
    load_foreign(dir);
    free(overlay);
    overlay = NULL;
    free(xoverlay);
    xoverlay = NULL;
    return 1;
}
