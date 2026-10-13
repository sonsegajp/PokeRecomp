/* rompack — take the cartridge's content out of a built PokeRecomp exe.
 *
 *   rompack IN.exe OUT.exe --sha1 HEX [--sha1 HEX ...] --dir DIR [--dir DIR ...] [--allow RANGES]
 *
 * Every run of bytes in the exe's data sections (.data, .rdata) that also
 * exists in a retail ROM is game content: graphics, maps, scripts, text,
 * music. Those runs are zeroed in OUT.exe, and a table appended to the exe
 * (an "overlay" the Windows loader ignores) says where in the ROM each one
 * comes from. On first launch the game reads the player's ROM, rebuilds the
 * bytes into <game>.pak, and from then on fills them in from the pack
 * (port/src/rompack.c). Bytes are only ever copied where they were proven
 * identical, so the rebuilt exe is exact by construction (and checksummed).
 *
 * --allow limits it to the game's own data sections (tools/game_ranges.py),
 * so nothing the C runtime needs before main() is ever taken out.
 *
 * ROMs are found by SHA-1 in the given directories. With several known ROMs
 * (e.g. revision 1.0 and 1.1) only bytes every one of them can supply are
 * taken out, and each gets its own table. With none found, OUT is IN.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define WINDOW 12           /* bytes looked up at once */
#define MIN_RUN 12          /* shortest run worth taking out */
#define MAX_ROMS 4
#define HASH_BITS 25

typedef struct { uint32_t rva, len, rom; } Piece;

/* ---- SHA-1 (as in the game's loader) -------------------------------------------------- */

static uint32_t rol(uint32_t v, int n) { return v << n | v >> (32 - n); }

static void sha1(const uint8_t *data, size_t len, uint8_t out[20])
{
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    size_t total = ((len + 8) / 64 + 1) * 64;
    for (size_t off = 0; off < total; off += 64) {
        uint8_t block[64];
        uint32_t w[80];
        for (int i = 0; i < 64; i++) {
            size_t p = off + i;
            block[i] = p < len ? data[p] : p == len ? 0x80 : 0;
        }
        if (off + 64 == total)
            for (int i = 0; i < 8; i++) block[56 + i] = (uint8_t)((uint64_t)len * 8 >> (56 - 8 * i));
        for (int i = 0; i < 16; i++)
            w[i] = (uint32_t)block[4 * i] << 24 | block[4 * i + 1] << 16 | block[4 * i + 2] << 8 | block[4 * i + 3];
        for (int i = 16; i < 80; i++) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) f = (b & c) | (~b & d), k = 0x5A827999;
            else if (i < 40) f = b ^ c ^ d, k = 0x6ED9EBA1;
            else if (i < 60) f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDC;
            else f = b ^ c ^ d, k = 0xCA62C1D6;
            uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = t;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }
    for (int i = 0; i < 20; i++) out[i] = (uint8_t)(h[i / 4] >> (24 - 8 * (i % 4)));
}

static uint64_t fnv64(uint64_t h, const uint8_t *p, size_t n)
{
    while (n--) h = (h ^ *p++) * 0x100000001B3ull;
    return h;
}

/* ---- files ---------------------------------------------------------------------------- */

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    *size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(*size ? *size : 1);
    if (fread(buf, 1, *size, f) != *size) { free(buf); buf = NULL; }
    fclose(f);
    return buf;
}

static int hex_to_sha1(const char *hex, uint8_t out[20])
{
    for (int i = 0; i < 20; i++) {
        unsigned v;
        if (sscanf(hex + 2 * i, "%2x", &v) != 1) return 0;
        out[i] = (uint8_t)v;
    }
    return 1;
}

/* ---- PE sections ---------------------------------------------------------------------- */

typedef struct { char name[9]; uint32_t rva, vsize, raw, rawsize; } Section;

static int pe_sections(const uint8_t *exe, size_t size, Section *out, int max)
{
    uint32_t pe = *(const uint32_t *)(exe + 0x3C);
    if (pe + 24 > size || memcmp(exe + pe, "PE\0\0", 4)) return -1;
    uint16_t count = *(const uint16_t *)(exe + pe + 6);
    uint16_t optsize = *(const uint16_t *)(exe + pe + 20);
    const uint8_t *sec = exe + pe + 24 + optsize;
    int n = 0;
    for (int i = 0; i < count && n < max; i++, sec += 40) {
        memcpy(out[n].name, sec, 8);
        out[n].name[8] = 0;
        out[n].vsize = *(const uint32_t *)(sec + 8);
        out[n].rva = *(const uint32_t *)(sec + 12);
        out[n].rawsize = *(const uint32_t *)(sec + 16);
        out[n].raw = *(const uint32_t *)(sec + 20);
        n++;
    }
    return n;
}

/* Base relocations: words the Windows loader patches when it moves the exe
 * (ASLR). Those must never be taken out — the loader would patch the zeros
 * and the pack would then overwrite the patch. Returns a list of RVAs. */
static uint32_t *pe_relocs(const uint8_t *exe, size_t size, const Section *secs, int nsec, int *count)
{
    uint32_t pe = *(const uint32_t *)(exe + 0x3C);
    const uint8_t *opt = exe + pe + 24;
    uint32_t rva = *(const uint32_t *)(opt + 96 + 5 * 8), len = *(const uint32_t *)(opt + 96 + 5 * 8 + 4);
    uint32_t *out = NULL, raw = 0;
    int n = 0, cap = 0;

    *count = 0;
    for (int s = 0; s < nsec; s++)
        if (rva >= secs[s].rva && rva < secs[s].rva + secs[s].vsize) raw = secs[s].raw + (rva - secs[s].rva);
    if (!rva || !raw || raw + len > size) return NULL;
    for (uint32_t off = 0; off + 8 <= len;) {
        uint32_t page = *(const uint32_t *)(exe + raw + off), bsize = *(const uint32_t *)(exe + raw + off + 4);
        if (bsize < 8) break;
        for (uint32_t e = 8; e + 2 <= bsize; e += 2) {
            uint16_t ent = *(const uint16_t *)(exe + raw + off + e);
            if ((ent >> 12) == 0) continue;           /* padding */
            if (n == cap) out = realloc(out, (cap = cap ? cap * 2 : 4096) * sizeof *out);
            out[n++] = page + (ent & 0xFFF);
        }
        off += bsize;
    }
    *count = n;
    return out;
}

/* ---- ROM index: WINDOW-byte windows -> first offset --------------------------------------- */

typedef struct { const uint8_t *rom; size_t size; uint32_t *slot; } Index;

static uint64_t key(const uint8_t *p)
{
    uint64_t a, b = 0;
    memcpy(&a, p, 8);
    memcpy(&b, p + 8, WINDOW - 8);
    return (a * 0x9E3779B97F4A7C15ull) ^ (b * 0xC2B2AE3D27D4EB4Full) ^ (a >> 29);
}

static int uniform(const uint8_t *p, int n)
{
    for (int i = 1; i < n; i++) if (p[i] != p[0]) return 0;
    return 1;
}

static void index_build(Index *ix, const uint8_t *rom, size_t size)
{
    size_t slots = (size_t)1 << HASH_BITS;
    ix->rom = rom;
    ix->size = size;
    ix->slot = malloc(slots * sizeof(uint32_t));
    memset(ix->slot, 0xFF, slots * sizeof(uint32_t));
    for (size_t off = 0; off + WINDOW <= size; off++) {
        if (uniform(rom + off, WINDOW)) continue;
        size_t s = key(rom + off) >> (64 - HASH_BITS);
        while (ix->slot[s] != 0xFFFFFFFF) {
            if (!memcmp(rom + ix->slot[s], rom + off, WINDOW)) goto next;   /* keep the first copy */
            s = (s + 1) & (slots - 1);
        }
        ix->slot[s] = (uint32_t)off;
    next:;
    }
}

static int64_t index_find(const Index *ix, const uint8_t *p)
{
    size_t slots = (size_t)1 << HASH_BITS, s = key(p) >> (64 - HASH_BITS);
    while (ix->slot[s] != 0xFFFFFFFF) {
        if (!memcmp(ix->rom + ix->slot[s], p, WINDOW)) return ix->slot[s];
        s = (s + 1) & (slots - 1);
    }
    return -1;
}

typedef struct { Piece *v; int n, cap; } Runs;

static void runs_add(Runs *r, Piece p)
{
    if (r->n == r->cap) r->v = realloc(r->v, (r->cap = r->cap ? r->cap * 2 : 4096) * sizeof(Piece));
    r->v[r->n++] = p;
}

/* Greedy matching of one section's bytes against one ROM: marks covered
 * bytes and records each run with where in the ROM it was found. */
static void match_section(const Index *ix, const uint8_t *data, uint32_t rva0, uint32_t len, uint8_t *covered, Runs *runs)
{
    uint32_t i = 0;
    while (i + WINDOW <= len) {
        int64_t r;
        if (uniform(data + i, WINDOW) || (r = index_find(ix, data + i)) < 0) { i++; continue; }
        uint32_t n = WINDOW;
        while (i + n < len && (size_t)r + n < ix->size && data[i + n] == ix->rom[r + n]) n++;
        if (n >= MIN_RUN) {
            memset(covered + i, 1, n);
            runs_add(runs, (Piece){rva0 + i, n, (uint32_t)r});
        }
        i += n;
    }
}

/* ---- main ----------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    const char *in = NULL, *outp = NULL, *dirs[8], *allow_path = NULL;
    uint8_t want[8][20];
    int ndirs = 0, nwant = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--sha1") && i + 1 < argc && nwant < 8) { if (!hex_to_sha1(argv[++i], want[nwant])) return 2; nwant++; }
        else if (!strcmp(argv[i], "--dir") && i + 1 < argc && ndirs < 8) dirs[ndirs++] = argv[++i];
        else if (!strcmp(argv[i], "--allow") && i + 1 < argc) allow_path = argv[++i];
        else if (!in) in = argv[i];
        else if (!outp) outp = argv[i];
    }
    if (!in || !outp) { fprintf(stderr, "usage: rompack IN.exe OUT.exe --sha1 HEX... --dir DIR...\n"); return 2; }

    size_t exesize;
    uint8_t *exe = read_file(in, &exesize);
    if (!exe) { fprintf(stderr, "rompack: can't read %s\n", in); return 1; }

    /* the ROMs we know, by hash */
    uint8_t *roms[MAX_ROMS];
    size_t romsize[MAX_ROMS];
    uint8_t romsha[MAX_ROMS][20];
    int nroms = 0;
    for (int d = 0; d < ndirs; d++) {
        DIR *dir = opendir(dirs[d]);
        struct dirent *e;
        if (!dir) continue;
        while ((e = readdir(dir)) && nroms < MAX_ROMS) {
            size_t l = strlen(e->d_name), size;
            char path[1024];
            uint8_t h[20], *data;
            if (l < 4 || strcmp(e->d_name + l - 4, ".gba")) continue;
            snprintf(path, sizeof path, "%s/%s", dirs[d], e->d_name);
            if (!(data = read_file(path, &size))) continue;
            sha1(data, size, h);
            int known = 0, dup = 0;
            for (int w = 0; w < nwant; w++) known |= !memcmp(h, want[w], 20);
            for (int r = 0; r < nroms; r++) dup |= !memcmp(h, romsha[r], 20);
            if (known && !dup) {
                roms[nroms] = data;
                romsize[nroms] = size;
                memcpy(romsha[nroms], h, 20);
                printf("rompack: ROM %s\n", path);
                nroms++;
            } else free(data);
        }
        closedir(dir);
    }
    FILE *f = fopen(outp, "wb");
    if (!f) { fprintf(stderr, "rompack: can't write %s\n", outp); return 1; }
    if (nroms == 0) {
        printf("rompack: no ROM found - %s keeps its game data\n", outp);
        fwrite(exe, 1, exesize, f);
        fclose(f);
        return 0;
    }

    Section secs[32];
    int nsec = pe_sections(exe, exesize, secs, 32);
    if (nsec < 0) { fprintf(stderr, "rompack: not a PE file\n"); return 1; }

    int nrel;
    uint32_t *rel = pe_relocs(exe, exesize, secs, nsec, &nrel);
    printf("rompack: %d relocated words kept in\n", nrel);

    Index ix[MAX_ROMS];
    Runs runs[MAX_ROMS] = {{0}};
    for (int r = 0; r < nroms; r++) index_build(&ix[r], roms[r], romsize[r]);

    /* canonical ranges: bytes every ROM can supply */
    Piece *ranges = malloc(sizeof(Piece) * 400000);
    int nranges = 0;
    uint64_t taken = 0, scanned = 0;
    for (int s = 0; s < nsec; s++) {
        if (strcmp(secs[s].name, ".data") && strcmp(secs[s].name, ".rdata")) continue;
        uint32_t len = secs[s].vsize < secs[s].rawsize ? secs[s].vsize : secs[s].rawsize;
        const uint8_t *data = exe + secs[s].raw;
        uint8_t *all = malloc(len), *one = malloc(len);
        memset(all, allow_path ? 0 : 1, len);
        if (allow_path) {       /* only the game's own sections (tools/game_ranges.py) */
            FILE *af = fopen(allow_path, "r");
            unsigned rva, sz;
            while (af && fscanf(af, "%x %x", &rva, &sz) == 2)
                for (uint32_t b = rva; b < rva + sz; b++)
                    if (b >= secs[s].rva && b < secs[s].rva + len) all[b - secs[s].rva] = 1;
            if (af) fclose(af);
        }
        for (int r = 0; r < nroms; r++) {
            memset(one, 0, len);
            match_section(&ix[r], data, secs[s].rva, len, one, &runs[r]);
            for (uint32_t i = 0; i < len; i++) all[i] &= one[i];
        }
        for (int k = 0; k < nrel; k++)          /* the loader patches these: keep them */
            for (uint32_t b = rel[k]; b < rel[k] + 4; b++)
                if (b >= secs[s].rva && b < secs[s].rva + len) all[b - secs[s].rva] = 0;
        for (uint32_t i = 0; i < len;) {
            if (!all[i]) { i++; continue; }
            uint32_t j = i;
            while (j < len && all[j]) j++;
            if (j - i >= MIN_RUN) {
                ranges[nranges++] = (Piece){secs[s].rva + i, j - i, secs[s].raw + i};   /* .rom = file offset here */
                taken += j - i;
            }
            i = j;
        }
        scanned += len;
        printf("rompack: %-8s %8u bytes\n", secs[s].name, len);
        free(all);
        free(one);
    }

    /* per-ROM tables: each canonical range as pieces of that ROM */
    Piece *tables[MAX_ROMS];
    int ntable[MAX_ROMS];
    for (int r = 0; r < nroms; r++) {          /* clip that ROM's runs to the ranges */
        Runs out = {0};
        int j = 0;
        for (int k = 0; k < nranges; k++) {
            uint32_t a = ranges[k].rva, b = a + ranges[k].len, got = 0;
            while (j < runs[r].n && runs[r].v[j].rva + runs[r].v[j].len <= a) j++;
            for (int q = j; q < runs[r].n && runs[r].v[q].rva < b; q++) {
                const Piece *run = &runs[r].v[q];
                uint32_t lo = run->rva > a ? run->rva : a, hi = run->rva + run->len < b ? run->rva + run->len : b;
                if (hi > lo) {
                    runs_add(&out, (Piece){lo, hi - lo, run->rom + (lo - run->rva)});
                    got += hi - lo;
                }
            }
            if (got != ranges[k].len) { fprintf(stderr, "rompack: ROM %d can't supply rva %08x\n", r, a); return 1; }
        }
        tables[r] = out.v;
        ntable[r] = out.n;
    }

    /* checksum of what comes out, then take it out */
    uint64_t check = 0xCBF29CE484222325ull;
    for (int k = 0; k < nranges; k++) {
        check = fnv64(check, exe + ranges[k].rom, ranges[k].len);
        memset(exe + ranges[k].rom, 0, ranges[k].len);
    }

    /* overlay: "PRPK" u32 version u32 ntables u32 nranges u64 total u64 check,
     *          nranges x (u32 rva, u32 len),
     *          per table: sha1[20] u32 romsize u32 npieces, npieces x (u32 rva, u32 len, u32 rom)
     *          then u32 overlay size, "PRPKEND!" */
    long start = (long)exesize;
    fwrite(exe, 1, exesize, f);
    uint32_t hdr[3] = {1, (uint32_t)nroms, (uint32_t)nranges};
    fwrite("PRPK", 1, 4, f);
    fwrite(hdr, 4, 3, f);
    fwrite(&taken, 8, 1, f);
    fwrite(&check, 8, 1, f);
    for (int k = 0; k < nranges; k++) { fwrite(&ranges[k].rva, 4, 1, f); fwrite(&ranges[k].len, 4, 1, f); }
    for (int r = 0; r < nroms; r++) {
        uint32_t rs = (uint32_t)romsize[r], np = (uint32_t)ntable[r];
        fwrite(romsha[r], 1, 20, f);
        fwrite(&rs, 4, 1, f);
        fwrite(&np, 4, 1, f);
        fwrite(tables[r], sizeof(Piece), ntable[r], f);
    }
    uint32_t overlay = (uint32_t)(ftell(f) - start) + 12;
    fwrite(&overlay, 4, 1, f);
    fwrite("PRPKEND!", 1, 8, f);
    fclose(f);
    printf("rompack: took out %llu of %llu data bytes (%.1f%%) in %d runs; %d ROM table(s)\n",
           (unsigned long long)taken, (unsigned long long)scanned, 100.0 * taken / scanned, nranges, nroms);
    return 0;
}
