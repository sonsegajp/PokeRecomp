#ifdef PORTABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "platform/pkmn_archive.h"

bool8 PkmnArchive_Open(struct PkmnArchive *archive, const char *path)
{
    memset(archive, 0, sizeof(*archive));

    archive->file = fopen(path, "rb");
    if (!archive->file)
        return FALSE;

    if (fread(&archive->header, sizeof(archive->header), 1, archive->file) != 1)
        goto fail;

    if (archive->header.magic != PKMN_MAGIC)
    {
        printf("pkmn_archive: invalid magic %08X\n", archive->header.magic);
        goto fail;
    }

    if (archive->header.version != PKMN_VERSION && archive->header.version != PKMN_VERSION_V1)
    {
        printf("pkmn_archive: unsupported version %u\n", archive->header.version);
        goto fail;
    }

    if (archive->header.flags & PKMN_FLAG_EXTRACTED)
    {
        // v2 manifest: no ROM blob, assets are in extracted_assets/ folder
        printf("pkmn_archive: manifest mode — loading from extracted_assets/\n");
        archive->romData = NULL;
    }
    else if (archive->header.flags & PKMN_FLAG_HAS_ROM)
    {
        archive->romData = malloc(archive->header.romSize);
        if (!archive->romData)
            goto fail;

        fseek(archive->file, sizeof(struct PkmnHeader), SEEK_SET);
        if (fread(archive->romData, 1, archive->header.romSize, archive->file) != archive->header.romSize)
            goto fail;
    }

    if (archive->header.flags & PKMN_FLAG_HAS_ASSETS)
    {
        u32 tocSize = archive->header.assetCount * sizeof(struct PkmnTocEntry);
        archive->toc = malloc(tocSize);
        if (!archive->toc)
            goto fail;

        fseek(archive->file, archive->header.tocOffset, SEEK_SET);
        if (fread(archive->toc, 1, tocSize, archive->file) != tocSize)
            goto fail;

        fseek(archive->file, 0, SEEK_END);
        u32 fileSize = ftell(archive->file);
        archive->stringTableSize = fileSize - archive->header.stringTableOffset;
        archive->stringTable = malloc(archive->stringTableSize);
        if (!archive->stringTable)
            goto fail;

        fseek(archive->file, archive->header.stringTableOffset, SEEK_SET);
        if (fread(archive->stringTable, 1, archive->stringTableSize, archive->file) != archive->stringTableSize)
            goto fail;
    }

    printf("pkmn_archive: loaded %.12s [%.4s] — %s%u assets, ROM %u bytes\n",
           archive->header.gameTitle, archive->header.gameCode,
           (archive->header.flags & PKMN_FLAG_EXTRACTED) ? "EXTRACTED, " : "",
           archive->header.assetCount, archive->header.romSize);

    return TRUE;

fail:
    PkmnArchive_Close(archive);
    return FALSE;
}

void PkmnArchive_Close(struct PkmnArchive *archive)
{
    if (archive->romData)
    {
        free(archive->romData);
        archive->romData = NULL;
    }
    if (archive->toc)
    {
        free(archive->toc);
        archive->toc = NULL;
    }
    if (archive->stringTable)
    {
        free(archive->stringTable);
        archive->stringTable = NULL;
    }
    if (archive->file)
    {
        fclose(archive->file);
        archive->file = NULL;
    }
}

const u8 *PkmnArchive_GetRomData(struct PkmnArchive *archive)
{
    return archive->romData;
}

u32 PkmnArchive_GetRomSize(struct PkmnArchive *archive)
{
    return archive->header.romSize;
}

const u8 *PkmnArchive_GetAsset(struct PkmnArchive *archive, const char *path, u32 *outSize)
{
    if (!archive->toc || !archive->stringTable)
        return NULL;

    for (u32 i = 0; i < archive->header.assetCount; i++)
    {
        const char *entryPath = &archive->stringTable[archive->toc[i].pathOffset];
        if (strcmp(entryPath, path) == 0)
        {
            if (outSize)
                *outSize = archive->toc[i].size;
            if (archive->romData && archive->toc[i].dataOffset < archive->header.romSize)
                return &archive->romData[archive->toc[i].dataOffset];
            return NULL;
        }
    }
    return NULL;
}

static u32 ComputeCrc32(const u8 *data, u32 size)
{
    static u32 table[256];
    static bool8 inited = FALSE;
    if (!inited)
    {
        for (u32 i = 0; i < 256; i++)
        {
            u32 c = i;
            for (int j = 0; j < 8; j++)
                c = (c >> 1) ^ (c & 1 ? 0xEDB88320 : 0);
            table[i] = c;
        }
        inited = TRUE;
    }
    u32 crc = 0xFFFFFFFF;
    for (u32 i = 0; i < size; i++)
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFF;
}

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(d) mkdir(d, 0755)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/stb_image_write.h"

// ---- ROM reading helpers ----
static u32 RomReadU32(const u8 *rom, u32 off) { return rom[off] | (rom[off+1]<<8) | (rom[off+2]<<16) | (rom[off+3]<<24); }
static u32 RomReadPtr(const u8 *rom, u32 off)
{
    u32 v = RomReadU32(rom, off);
    if (v >= 0x08000000 && v < 0x0A000000) return v & 0x01FFFFFF;
    return 0;
}

static int LZ77Decompress(const u8 *src, u32 srcMax, u8 *dst, u32 dstMax)
{
    if (src[0] != 0x10) return 0;
    u32 size = src[1] | (src[2]<<8) | (src[3]<<16);
    if (size == 0 || size > dstMax) return 0;
    u32 sp = 4, dp = 0;
    while (dp < size && sp < srcMax) {
        u8 flags = src[sp++];
        for (int i = 0; i < 8 && dp < size; i++) {
            if (flags & 0x80) {
                if (sp+1 >= srcMax) return dp;
                u8 b1 = src[sp++], b2 = src[sp++];
                int len = (b1 >> 4) + 3;
                int disp = ((b1 & 0xF) << 8) | b2;
                for (int j = 0; j < len && dp < size; j++) {
                    int ref = (int)dp - disp - 1;
                    dst[dp++] = (ref >= 0) ? dst[ref] : 0;
                }
            } else {
                if (sp >= srcMax) return dp;
                dst[dp++] = src[sp++];
            }
            flags <<= 1;
        }
    }
    return dp;
}

static void MkdirP(const char *path)
{
    char tmp[512];
    strncpy(tmp, path, sizeof(tmp)-1);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0'; MKDIR(tmp); *p = '/';
        }
    }
    MKDIR(tmp);
}

// Decode 4bpp tiles to RGBA pixels
static void Decode4bppToRGBA(const u8 *tiles, int tileCount, const u16 *palette, int widthTiles, u8 *rgba)
{
    int htiles = (tileCount + widthTiles - 1) / widthTiles;
    int w = widthTiles * 8, h = htiles * 8;
    memset(rgba, 0, w * h * 4);

    for (int ty = 0; ty < htiles; ty++) {
        for (int tx = 0; tx < widthTiles; tx++) {
            int ti = ty * widthTiles + tx;
            if (ti >= tileCount) continue;
            for (int py = 0; py < 8; py++) {
                for (int px = 0; px < 8; px += 2) {
                    int boff = ti * 32 + py * 4 + px / 2;
                    u8 byte = tiles[boff];
                    u8 ci0 = byte & 0xF, ci1 = (byte >> 4) & 0xF;
                    int coords[2][2] = {{tx*8+px, ty*8+py}, {tx*8+px+1, ty*8+py}};
                    u8 cis[2] = {ci0, ci1};
                    for (int k = 0; k < 2; k++) {
                        int idx = (coords[k][1] * w + coords[k][0]) * 4;
                        if (cis[k] == 0) { rgba[idx+3] = 0; }
                        else {
                            u16 c = palette[cis[k]];
                            rgba[idx+0] = (c & 0x1F) << 3;
                            rgba[idx+1] = ((c>>5) & 0x1F) << 3;
                            rgba[idx+2] = ((c>>10) & 0x1F) << 3;
                            rgba[idx+3] = 255;
                        }
                    }
                }
            }
        }
    }
}

static void WritePNG4bpp(const char *path, const u8 *data, int dataSize, const u16 *palette, int widthTiles)
{
    int tileCount = dataSize / 32;
    int htiles = (tileCount + widthTiles - 1) / widthTiles;
    int w = widthTiles * 8, h = htiles * 8;
    u8 *rgba = malloc(w * h * 4);
    if (!rgba) return;
    Decode4bppToRGBA(data, tileCount, palette, widthTiles, rgba);
    stbi_write_png(path, w, h, 4, rgba, w * 4);
    free(rgba);
}

static void WritePalettePNG(const char *path, const u16 *palette, int count)
{
    int w = count * 8, h = 8;
    u8 *rgba = malloc(w * h * 4);
    for (int i = 0; i < count; i++) {
        u16 c = palette[i];
        u8 r = (c & 0x1F) << 3, g = ((c>>5)&0x1F) << 3, b = ((c>>10)&0x1F) << 3;
        for (int y = 0; y < h; y++)
            for (int x = 0; x < 8; x++) {
                int idx = (y * w + i * 8 + x) * 4;
                rgba[idx] = r; rgba[idx+1] = g; rgba[idx+2] = b; rgba[idx+3] = 255;
            }
    }
    stbi_write_png(path, w, h, 4, rgba, w * 4);
    free(rgba);
}

static void WriteRawFile(const char *path, const void *data, u32 size)
{
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(data, 1, size, f); fclose(f); }
}

static void WriteJsonString(FILE *f, const u8 *rom, u32 off, int maxLen)
{
    for (int i = 0; i < maxLen; i++) {
        u8 c = rom[off + i];
        if (c == 0xFF) break;
        if (c >= 0xBB && c <= 0xD4) fputc('A' + c - 0xBB, f);
        else if (c >= 0xD5 && c <= 0xEE) fputc('a' + c - 0xD5, f);
        else if (c >= 0xA1 && c <= 0xAA) fputc('0' + c - 0xA1, f);
        else if (c == 0x00) fputc(' ', f);
        else if (c == 0xAE) fputc('-', f);
        else if (c == 0xB4) fputc('\'', f);
        else if (c == 0xAB) fputc('!', f);
        else if (c == 0xAC) fputc('?', f);
        else if (c == 0xAD) fputc('.', f);
        else fputc('_', f);
    }
}

// ---- FireRed ROM table offsets (auto-detected) ----
static u32 FindSpeciesNamesTable(const u8 *rom, u32 romSize)
{
    // Search for "BULBASAUR" in GBA encoding: BC CF C6 BC BB CD BB CF CC
    u8 target[] = {0xBC, 0xCF, 0xC6, 0xBC, 0xBB, 0xCD, 0xBB, 0xCF, 0xCC};
    for (u32 i = 0x240000; i < romSize - sizeof(target); i++) {
        if (memcmp(&rom[i], target, sizeof(target)) == 0)
            return i - 11; // Species 0 is 11 bytes before Bulbasaur
    }
    return 0;
}

static u32 FindPicTable(const u8 *rom, u32 romSize, u32 startSearch)
{
    // Find table of {ptr, 0x800} entries
    for (u32 base = startSearch; base < startSearch + 0x30000; base += 4) {
        int valid = 0;
        for (int i = 2; i < 10; i++) {
            u32 ptr = RomReadU32(rom, base + i*8);
            u32 size = RomReadU32(rom, base + i*8 + 4);
            if (size == 0x800 && (ptr & 0xFF000000) == 0x08000000) valid++;
        }
        if (valid >= 6) return base;
    }
    return 0;
}

// ---- Copy helper ----
static int CopyFile_(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return 0;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return 0; }
    char buf[8192];
    int total = 0, n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) { fwrite(buf, 1, n, out); total += n; }
    fclose(in); fclose(out);
    return total;
}

static int CopyTree(const char *srcDir, const char *dstDir, const char *exts[], int numExts)
{
    int count = 0;
#ifdef _WIN32
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", srcDir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (fd.cFileName[0] == '.') continue;
        char srcPath[512], dstPath[512];
        snprintf(srcPath, sizeof(srcPath), "%s/%s", srcDir, fd.cFileName);
        snprintf(dstPath, sizeof(dstPath), "%s/%s", dstDir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            MkdirP(dstPath);
            count += CopyTree(srcPath, dstPath, exts, numExts);
        } else {
            int match = (numExts == 0);
            for (int i = 0; i < numExts && !match; i++) {
                int el = strlen(exts[i]), fl = strlen(fd.cFileName);
                if (fl > el && strcmp(fd.cFileName + fl - el, exts[i]) == 0) match = 1;
            }
            if (match) { CopyFile_(srcPath, dstPath); count++; }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#endif
    return count;
}

// ---- Main extraction ----
#define SPECIES_NAME_LEN 11
#define NUM_SPECIES_MAX 412
#define DECOMP_BUF_SIZE 0x10000

static void ExtractFromSourceTreePrefixed(const char *root)
{
    char srcBuf[512];
    #define SRC(p) (snprintf(srcBuf, sizeof(srcBuf), "%s/%s", root, p), srcBuf)

    const char *gfxExts[] = {".png", ".4bpp", ".gbapal", ".pal", ".bin", ".1bpp", ".8bpp"};
    const char *dataExts[] = {".json", ".h", ".inc", ".bin"};
    const char *soundExts[] = {".s", ".inc", ".bin", ".pcm", ".aif"};
    const char *mapExts[] = {".json", ".inc", ".bin"};
    int total = 0;

    printf("  Extracting from source tree...\n");

    // Pokemon sprites
    if (MkdirP("extracted_assets/frlg/pokemon"), 1) {
        total += CopyTree(SRC("graphics/pokemon"), "extracted_assets/frlg/pokemon", gfxExts, 7);
        printf("    Pokemon: %d files\n", total);
    }

    // Trainers
    int n = 0;
    MkdirP("extracted_assets/frlg/trainers");
    n = CopyTree(SRC("graphics/trainers"), "extracted_assets/frlg/trainers", gfxExts, 7);
    printf("    Trainers: %d files\n", n); total += n;

    // Overworld
    MkdirP("extracted_assets/frlg/overworld");
    n = CopyTree(SRC("graphics/object_events"), "extracted_assets/frlg/overworld", gfxExts, 7);
    printf("    Overworld: %d files\n", n); total += n;

    // Tilesets
    MkdirP("extracted_assets/frlg/tilesets");
    n = CopyTree(SRC("data/tilesets"), "extracted_assets/frlg/tilesets", (const char*[]){".bin", ".4bpp", ".4bpp.lz", ".png", ".gbapal"}, 5);
    printf("    Tilesets: %d files\n", n); total += n;

    // All other graphics
    MkdirP("extracted_assets/frlg/graphics");
    n = CopyTree(SRC("graphics"), "extracted_assets/frlg/graphics", gfxExts, 7);
    printf("    Graphics: %d files\n", n); total += n;

    // Maps
    MkdirP("extracted_assets/frlg/maps");
    n = CopyTree(SRC("data/maps"), "extracted_assets/frlg/maps", mapExts, 4);
    printf("    Maps: %d files\n", n); total += n;

    // Sound
    MkdirP("extracted_assets/frlg/sound");
    n = CopyTree(SRC("sound"), "extracted_assets/frlg/sound", (const char*[]){".s", ".inc", ".bin", ".pcm", ".aif", ".mid"}, 6);
    printf("    Sound: %d files\n", n); total += n;

    // Data source files
    MkdirP("extracted_assets/frlg/data");
    n = CopyTree(SRC("src/data"), "extracted_assets/frlg/data/src", dataExts, 4);
    printf("    Data: %d files\n", n); total += n;

    // Generate JSON data using external script
    printf("  Generating JSON data...\n");
    fflush(stdout);
    {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "python3 \"%s/tools/gen_json.py\" \"%s\" extracted_assets/frlg/data", root, root);
        system(cmd);
    }

    printf("  Source tree extraction: %d total files\n", total);
}

static void ExtractAssetsFolder(const u8 *rom, u32 romSize)
{
    // Try source tree first (most reliable)
    // Check current dir, then parent (for deploy/ subfolder)
    const char *srcRoots[] = {".", "..", NULL};
    const char *srcRoot = NULL;
    for (int i = 0; srcRoots[i]; i++)
    {
        char testPath[512];
        snprintf(testPath, sizeof(testPath), "%s/graphics/pokemon/bulbasaur/front.png", srcRoots[i]);
        FILE *testFile = fopen(testPath, "rb");
        if (testFile) { fclose(testFile); srcRoot = srcRoots[i]; break; }
    }
    if (srcRoot)
    {
        printf("  Decomp source tree found at '%s' — using source extraction\n", srcRoot);
        ExtractFromSourceTreePrefixed(srcRoot);
        return;
    }

    printf("  No source tree — extracting from ROM data\n");

    char path[512];
    u8 *decompBuf = malloc(DECOMP_BUF_SIZE);
    u16 palBuf[16];

    MkdirP("extracted_assets/frlg/pokemon");
    MkdirP("extracted_assets/frlg/data");

    printf("  Finding ROM tables...\n");

    u32 namesOff = FindSpeciesNamesTable(rom, romSize);
    u32 frontPicTable = FindPicTable(rom, romSize, 0x230000);

    if (!namesOff) { printf("  Could not find species name table!\n"); free(decompBuf); return; }
    if (!frontPicTable) { printf("  Could not find front pic table!\n"); free(decompBuf); return; }

    printf("  Species names at 0x%06X, front pics at 0x%06X\n", namesOff, frontPicTable);

    // Back pic table is typically 0x670 bytes after front
    u32 backPicTable = frontPicTable + (NUM_SPECIES_MAX + 1) * 8;
    u32 palTable = backPicTable + (NUM_SPECIES_MAX + 1) * 8;
    u32 shinyPalTable = palTable + (NUM_SPECIES_MAX + 1) * 8;

    // ---- Extract Pokemon sprites ----
    printf("  Extracting Pokemon...\n");
    int pokemonCount = 0;

    for (int sp = 1; sp < NUM_SPECIES_MAX; sp++)
    {
        // Read name
        char name[32] = {0};
        int ni = 0;
        for (int j = 0; j < SPECIES_NAME_LEN; j++) {
            u8 c = rom[namesOff + sp * SPECIES_NAME_LEN + j];
            if (c == 0xFF) break;
            if (c >= 0xBB && c <= 0xD4) name[ni++] = 'a' + c - 0xBB;
            else if (c >= 0xD5 && c <= 0xEE) name[ni++] = 'a' + c - 0xD5;
            else if (c >= 0xA1 && c <= 0xAA) name[ni++] = '0' + c - 0xA1;
            else if (c == 0xAE) name[ni++] = '-';
            else if (c == 0x00) name[ni++] = '_';
            else name[ni++] = '_';
        }
        name[ni] = '\0';
        if (ni == 0 || name[0] == '_') continue;

        snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s", sp, name);
        MkdirP(path);

        // Front sprite
        u32 frontPtr = RomReadPtr(rom, frontPicTable + sp * 8);
        if (frontPtr && frontPtr < romSize && rom[frontPtr] == 0x10) {
            int sz = LZ77Decompress(&rom[frontPtr], romSize - frontPtr, decompBuf, DECOMP_BUF_SIZE);
            if (sz > 0) {
                // Get palette
                u32 palPtr = RomReadPtr(rom, palTable + sp * 8);
                if (palPtr && palPtr < romSize) {
                    int psz = LZ77Decompress(&rom[palPtr], romSize - palPtr, (u8*)palBuf, 32);
                    if (psz < 32) memcpy(palBuf, &rom[palPtr], 32);
                } else { for (int i=0;i<16;i++) palBuf[i] = i*0x0842; }

                snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/front.png", sp, name);
                WritePNG4bpp(path, decompBuf, sz, palBuf, 8);

                // Raw 4bpp
                snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/front.4bpp", sp, name);
                WriteRawFile(path, decompBuf, sz);

                // Normal palette
                snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/normal.gbapal", sp, name);
                WriteRawFile(path, palBuf, 32);

                // Shiny palette
                u32 shinyPtr = RomReadPtr(rom, shinyPalTable + sp * 8);
                if (shinyPtr && shinyPtr < romSize) {
                    u16 shinyPal[16];
                    int spsz = LZ77Decompress(&rom[shinyPtr], romSize - shinyPtr, (u8*)shinyPal, 32);
                    if (spsz < 32) memcpy(shinyPal, &rom[shinyPtr], 32);
                    snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/shiny.gbapal", sp, name);
                    WriteRawFile(path, shinyPal, 32);
                    snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/front_shiny.png", sp, name);
                    WritePNG4bpp(path, decompBuf, sz, shinyPal, 8);
                }
            }
        }

        // Back sprite
        u32 backPtr = RomReadPtr(rom, backPicTable + sp * 8);
        if (backPtr && backPtr < romSize && rom[backPtr] == 0x10) {
            int sz = LZ77Decompress(&rom[backPtr], romSize - backPtr, decompBuf, DECOMP_BUF_SIZE);
            if (sz > 0) {
                u32 palPtr = RomReadPtr(rom, palTable + sp * 8);
                if (palPtr && palPtr < romSize) {
                    int psz = LZ77Decompress(&rom[palPtr], romSize - palPtr, (u8*)palBuf, 32);
                    if (psz < 32) memcpy(palBuf, &rom[palPtr], 32);
                }
                snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/back.png", sp, name);
                WritePNG4bpp(path, decompBuf, sz, palBuf, 8);
                snprintf(path, sizeof(path), "extracted_assets/frlg/pokemon/%03d_%s/back.4bpp", sp, name);
                WriteRawFile(path, decompBuf, sz);
            }
        }

        pokemonCount++;
        if (pokemonCount % 50 == 0)
            printf("    %d Pokemon...\n", pokemonCount);
    }
    printf("  Extracted %d Pokemon\n", pokemonCount);

    // ---- Extract Pokemon data JSON ----
    printf("  Writing pokemon.json...\n");
    {
        // Find base stats table (28 bytes per entry, search near species names)
        // Base stats for Bulbasaur: HP=45, ATK=49, DEF=49
        u32 baseStatsOff = 0;
        for (u32 off = namesOff + 0x10000; off < romSize - 28 * 10; off += 4) {
            if (rom[off] == 45 && rom[off+1] == 49 && rom[off+2] == 49 &&
                rom[off+3] == 45 && rom[off+4] == 65 && rom[off+5] == 65) {
                baseStatsOff = off - 28; // species 0 is before Bulbasaur
                break;
            }
        }

        snprintf(path, sizeof(path), "extracted_assets/frlg/data/pokemon.json");
        FILE *jf = fopen(path, "w");
        if (jf && baseStatsOff) {
            fprintf(jf, "[\n");
            for (int sp = 0; sp < NUM_SPECIES_MAX; sp++) {
                u32 off = baseStatsOff + sp * 28;
                fprintf(jf, "  {\"id\":%d,\"name\":\"", sp);
                WriteJsonString(jf, rom, namesOff + sp * SPECIES_NAME_LEN, SPECIES_NAME_LEN);
                fprintf(jf, "\",\"hp\":%d,\"attack\":%d,\"defense\":%d,\"speed\":%d,\"sp_attack\":%d,\"sp_defense\":%d,\"type1\":%d,\"type2\":%d,\"catch_rate\":%d,\"exp_yield\":%d}%s\n",
                    rom[off], rom[off+1], rom[off+2], rom[off+3], rom[off+4], rom[off+5],
                    rom[off+6], rom[off+7], rom[off+8], rom[off+9],
                    sp < NUM_SPECIES_MAX-1 ? "," : "");
            }
            fprintf(jf, "]\n");
            fclose(jf);
            printf("  Written pokemon.json (%d entries)\n", NUM_SPECIES_MAX);
        }
    }

    // ---- Extract trainer sprites ----
    printf("  Extracting trainer sprites...\n");
    MkdirP("extracted_assets/frlg/trainers");
    {
        // Trainer front pic table: {ptr(4), size(u16), tag(u16)} entries
        // ptr points to LZ compressed data that decompresses to 0x800 bytes
        u32 trainerFrontTable = 0;
        u32 trainerPalTable = 0;
        for (u32 base = 0x233000; base < 0x245000; base += 2)
        {
            int valid = 0;
            for (int i = 0; i < 20; i++) {
                u32 ptr = RomReadU32(rom, base + i*8);
                if ((ptr & 0xFF000000) == 0x08000000) {
                    u32 roff = ptr & 0x01FFFFFF;
                    if (roff < romSize && rom[roff] == 0x10) {
                        u32 dsz = rom[roff+1] | (rom[roff+2]<<8) | (rom[roff+3]<<16);
                        if (dsz == 0x800) valid++;
                    }
                }
            }
            // Must not be a Pokemon pic table (those have RAM ptrs at index 0-1)
            u32 ptr0 = RomReadU32(rom, base);
            if (valid >= 12 && (ptr0 & 0xFF000000) == 0x08000000
                && base != frontPicTable && base != backPicTable) {
                trainerFrontTable = base;
                break;
            }
        }

        if (trainerFrontTable) {
            printf("  Trainer front table at 0x%06X\n", trainerFrontTable);
            // Palette table: {ptr(4), tag(u16)} = 8 bytes per entry, after front pics
            // Search for sequence of LZ-compressed palette pointers
            for (u32 base = trainerFrontTable + 0x400; base < trainerFrontTable + 0x3000; base += 2) {
                int pvalid = 0;
                for (int i = 0; i < 10; i++) {
                    u32 ptr = RomReadU32(rom, base + i * 8);
                    if ((ptr & 0xFF000000) == 0x08000000) {
                        u32 roff = ptr & 0x01FFFFFF;
                        if (roff < romSize && rom[roff] == 0x10) pvalid++;
                    }
                }
                if (pvalid >= 7) { trainerPalTable = base; break; }
            }

            int trainerCount = 0;
            for (int t = 0; t < 200; t++) {
                u32 ptr = RomReadPtr(rom, trainerFrontTable + t * 8);
                if (!ptr || ptr >= romSize) continue;
                if (rom[ptr] != 0x10) continue;

                int sz = LZ77Decompress(&rom[ptr], romSize - ptr, decompBuf, DECOMP_BUF_SIZE);
                if (sz <= 0) continue;

                // Get palette
                u16 tPal[16];
                memset(tPal, 0, sizeof(tPal));
                if (trainerPalTable) {
                    u32 pp = RomReadPtr(rom, trainerPalTable + t * 8);
                    if (pp && pp < romSize) {
                        int psz = LZ77Decompress(&rom[pp], romSize - pp, (u8*)tPal, 32);
                        if (psz < 32) memcpy(tPal, &rom[pp], 32);
                    }
                }

                snprintf(path, sizeof(path), "extracted_assets/frlg/trainers/%03d", t);
                MkdirP(path);
                snprintf(path, sizeof(path), "extracted_assets/frlg/trainers/%03d/front.png", t);
                WritePNG4bpp(path, decompBuf, sz, tPal, 8);
                snprintf(path, sizeof(path), "extracted_assets/frlg/trainers/%03d/front.4bpp", t);
                WriteRawFile(path, decompBuf, sz);
                snprintf(path, sizeof(path), "extracted_assets/frlg/trainers/%03d/palette.gbapal", t);
                WriteRawFile(path, tPal, 32);
                trainerCount++;
            }
            printf("  Extracted %d trainers\n", trainerCount);
        } else {
            printf("  Could not find trainer table\n");
        }
    }

    // ---- Extract move data JSON ----
    printf("  Writing moves.json...\n");
    {
        // Move names: search for "POUND" encoded = CA C9 CF C8 BE (P O U N D)
        u8 poundTarget[] = {0xCA, 0xC9, 0xCF, 0xC8, 0xBE};
        u32 moveNamesOff = 0;
        for (u32 off = 0x240000; off < romSize - sizeof(poundTarget); off++) {
            if (memcmp(&rom[off], poundTarget, sizeof(poundTarget)) == 0) {
                moveNamesOff = off - 13; // Move 0 (none) is 13 bytes before Pound
                break;
            }
        }
        // Battle moves data: 12 bytes per entry, search for Pound (effect=0, power=40, type=0, acc=100, pp=35)
        u32 moveDataOff = 0;
        for (u32 off = 0x240000; off < romSize - 12 * 10; off += 4) {
            // Move 1 (Pound): power=40, accuracy=100, pp=35
            // Move 2 (Karate Chop): power=50, accuracy=100, pp=25
            if (rom[off+12+1] == 40 && rom[off+12+3] == 100 && rom[off+12+4] == 35 &&
                rom[off+24+1] == 50 && rom[off+24+3] == 100 && rom[off+24+4] == 25) {
                moveDataOff = off;
                break;
            }
        }

        if (moveNamesOff && moveDataOff) {
            snprintf(path, sizeof(path), "extracted_assets/frlg/data/moves.json");
            FILE *jf = fopen(path, "w");
            if (jf) {
                fprintf(jf, "[\n");
                for (int m = 0; m < 355; m++) {
                    u32 doff = moveDataOff + m * 12;
                    fprintf(jf, "  {\"id\":%d,\"name\":\"", m);
                    WriteJsonString(jf, rom, moveNamesOff + m * 13, 13);
                    fprintf(jf, "\",\"effect\":%d,\"power\":%d,\"type\":%d,\"accuracy\":%d,\"pp\":%d,\"effect_chance\":%d,\"target\":%d,\"priority\":%d}%s\n",
                        rom[doff], rom[doff+1], rom[doff+2], rom[doff+3], rom[doff+4],
                        rom[doff+5], rom[doff+6], (s8)rom[doff+7], m < 354 ? "," : "");
                }
                fprintf(jf, "]\n");
                fclose(jf);
                printf("  Written moves.json (355 entries)\n");
            }
        } else {
            printf("  Could not find move tables (names=%08X data=%08X)\n", moveNamesOff, moveDataOff);
        }
    }

    // ---- Extract overworld sprites ----
    printf("  Extracting overworld sprites...\n");
    MkdirP("extracted_assets/frlg/overworld");
    {
        // Object event graphics are uncompressed 4bpp, referenced by a graphics info pointer table
        // The graphics info struct has: tileTag(2), paletteTag(2), reflPalTag(2), size(2), w(2), h(2), palSlot(1), shadow(1), ...images(ptr)
        // Each images entry is {data_ptr, size}
        // We scan for the gObjectEventGraphicsInfoPointers table: an array of ROM pointers
        // to graphics info structs, typically near 0x39B000-0x39C000
        // Find gObjectEventGraphicsInfoPointers: array of ROM ptrs to GraphicsInfo structs
        // GraphicsInfo has tileTag=0xFFFF for most entries
        u32 gfxInfoPtrTable = 0;
        for (u32 base = 0x398000; base < 0x3A2000; base += 4) {
            int valid = 0;
            for (int i = 0; i < 20; i++) {
                u32 ptr = RomReadPtr(rom, base + i * 4);
                if (!ptr || ptr >= romSize - 36) continue;
                u16 tileTag = rom[ptr] | (rom[ptr+1] << 8);
                if (tileTag == 0xFFFF) valid++;
            }
            if (valid >= 15) { gfxInfoPtrTable = base; break; }
        }

        if (gfxInfoPtrTable) {
            printf("  Overworld gfx table at 0x%06X\n", gfxInfoPtrTable);
            int owCount = 0;
            for (int g = 0; g < 256; g++) {
                u32 infoPtr = RomReadPtr(rom, gfxInfoPtrTable + g * 4);
                if (!infoPtr || infoPtr >= romSize - 36) continue;

                u16 size = rom[infoPtr+6] | (rom[infoPtr+7] << 8);
                if (size == 0 || size > 0x4000) continue;

                // images pointer is at offset +28 in the GraphicsInfo struct
                u32 imagesPtr = RomReadPtr(rom, infoPtr + 28);
                if (!imagesPtr || imagesPtr >= romSize - 8) continue;

                // First image entry: {data_ptr, size}
                u32 dataPtr = RomReadPtr(rom, imagesPtr);
                if (!dataPtr || dataPtr >= romSize) continue;

                u32 dataSize = RomReadU32(rom, imagesPtr + 4);
                if (dataSize == 0 || dataSize > 0x4000 || dataPtr + dataSize > romSize) continue;

                // Write the full sprite sheet (all frames)
                // Total size = GraphicsInfo.size (covers all animation frames)
                u32 totalSize = size;
                if (dataPtr + totalSize > romSize) totalSize = dataSize;
                snprintf(path, sizeof(path), "extracted_assets/frlg/overworld/%03d.4bpp", g);
                WriteRawFile(path, &rom[dataPtr], totalSize);
                owCount++;
            }
            printf("  Extracted %d overworld sprites\n", owCount);
        } else {
            printf("  Could not find overworld graphics table\n");
        }
    }

    // ---- Extract all compressed graphics + tileset data ----
    printf("  Scanning for all compressed data...\n");
    MkdirP("extracted_assets/frlg/graphics");
    MkdirP("extracted_assets/frlg/tilesets");
    MkdirP("extracted_assets/frlg/sound");
    {
        u32 count = 0;
        for (u32 offset = 0x100000; offset < romSize - 4; offset += 4) {
            if (rom[offset] != 0x10) continue;
            u32 decompSize = (rom[offset+3]<<16) | (rom[offset+2]<<8) | rom[offset+1];
            if (decompSize < 32 || decompSize > 0x20000) continue;

            int sz = LZ77Decompress(&rom[offset], romSize - offset, decompBuf, DECOMP_BUF_SIZE);
            if (sz > 0) {
                if (decompSize % 32 == 0) {
                    // Likely 4bpp graphics
                    snprintf(path, sizeof(path), "extracted_assets/frlg/graphics/0x%06X.4bpp", offset);
                } else if (decompSize == 32 || decompSize == 512) {
                    // Likely palette
                    snprintf(path, sizeof(path), "extracted_assets/frlg/graphics/0x%06X.gbapal", offset);
                } else {
                    snprintf(path, sizeof(path), "extracted_assets/frlg/graphics/0x%06X.bin", offset);
                }
                WriteRawFile(path, decompBuf, sz);
                count++;
                offset += 4;
            }
            if (count >= 8000) break;
        }
        printf("  Extracted %u compressed data blocks\n", count);
    }

    // ---- Extract raw music/sound data ----
    printf("  Extracting sound data...\n");
    {
        // Song table: search for structure of {header_ptr, music_player, unk, unk}
        // Each entry is 8 bytes. Search for a sequence of valid ROM pointers.
        u32 songTableOff = 0;
        for (u32 base = 0x400000; base < romSize - 800; base += 4) {
            int valid = 0;
            for (int i = 0; i < 100; i++) {
                u32 ptr = RomReadU32(rom, base + i * 8);
                if ((ptr & 0xFF000000) == 0x08000000 && (ptr & 0x01FFFFFF) < romSize)
                    valid++;
            }
            if (valid >= 80) { songTableOff = base; break; }
        }

        if (songTableOff) {
            int songCount = 0;
            for (int s = 0; s < 400; s++) {
                u32 headerPtr = RomReadPtr(rom, songTableOff + s * 8);
                if (!headerPtr || headerPtr >= romSize) continue;
                // Write raw song header data
                snprintf(path, sizeof(path), "extracted_assets/frlg/sound/song_%03d.bin", s);
                // Write 256 bytes of song header
                int writeSize = (headerPtr + 256 <= romSize) ? 256 : romSize - headerPtr;
                WriteRawFile(path, &rom[headerPtr], writeSize);
                songCount++;
            }
            printf("  Extracted %d song headers\n", songCount);
        } else {
            printf("  Could not find song table\n");
        }
    }

    // ---- Extract map data ----
    printf("  Extracting map headers...\n");
    MkdirP("extracted_assets/frlg/maps");
    {
        // Map groups table: array of pointers to map header arrays
        // Search for it by finding a cluster of ROM pointers near 0x350000
        u32 mapGroupTableOff = 0;
        for (u32 base = 0x340000; base < 0x360000; base += 4) {
            int valid = 0;
            for (int i = 0; i < 5; i++) {
                u32 ptr = RomReadU32(rom, base + i * 4);
                if ((ptr & 0xFF000000) == 0x08000000) {
                    u32 roff = ptr & 0x01FFFFFF;
                    if (roff < romSize) {
                        // Check if the group pointer points to another ROM pointer (map header)
                        u32 inner = RomReadU32(rom, roff);
                        if ((inner & 0xFF000000) == 0x08000000 && (inner & 0x01FFFFFF) < romSize)
                            valid++;
                    }
                }
            }
            if (valid >= 4) { mapGroupTableOff = base; break; }
        }

        if (mapGroupTableOff) {
            int mapCount = 0;
            for (int g = 0; g < 50; g++) {
                u32 groupPtr = RomReadPtr(rom, mapGroupTableOff + g * 4);
                if (!groupPtr || groupPtr >= romSize) continue;
                for (int m = 0; m < 50; m++) {
                    u32 mapPtr = RomReadPtr(rom, groupPtr + m * 4);
                    if (!mapPtr || mapPtr >= romSize || mapPtr < 0x100000) break;

                    snprintf(path, sizeof(path), "extracted_assets/frlg/maps/%d_%d", g, m);
                    MkdirP(path);

                    // Write raw map header (28 bytes)
                    snprintf(path, sizeof(path), "extracted_assets/frlg/maps/%d_%d/header.bin", g, m);
                    WriteRawFile(path, &rom[mapPtr], 28);

                    // Extract map layout if pointer is valid
                    u32 layoutPtr = RomReadPtr(rom, mapPtr);
                    if (layoutPtr && layoutPtr < romSize) {
                        // Layout: width(4) + height(4) + border(4) + map(4) + tileset1(4) + tileset2(4)
                        snprintf(path, sizeof(path), "extracted_assets/frlg/maps/%d_%d/layout.bin", g, m);
                        WriteRawFile(path, &rom[layoutPtr], 24);

                        // Extract map grid
                        u32 mapWidth = RomReadU32(rom, layoutPtr);
                        u32 mapHeight = RomReadU32(rom, layoutPtr + 4);
                        u32 mapDataPtr = RomReadPtr(rom, layoutPtr + 12);
                        if (mapWidth > 0 && mapWidth < 256 && mapHeight > 0 && mapHeight < 256 &&
                            mapDataPtr && mapDataPtr < romSize) {
                            u32 gridSize = mapWidth * mapHeight * 2;
                            if (mapDataPtr + gridSize <= romSize) {
                                snprintf(path, sizeof(path), "extracted_assets/frlg/maps/%d_%d/map.bin", g, m);
                                WriteRawFile(path, &rom[mapDataPtr], gridSize);
                            }
                        }
                    }
                    mapCount++;
                }
            }
            printf("  Extracted %d maps\n", mapCount);
        } else {
            printf("  Could not find map group table\n");
        }
    }

    free(decompBuf);
    printf("  Asset extraction complete!\n");
}

bool8 PkmnArchive_CreateFromRom(const char *romPath, const char *outPath)
{
    FILE *rf = fopen(romPath, "rb");
    if (!rf)
    {
        printf("Cannot open ROM: %s\n", romPath);
        return FALSE;
    }

    fseek(rf, 0, SEEK_END);
    u32 romSize = ftell(rf);
    fseek(rf, 0, SEEK_SET);

    if (romSize < 0x100 || romSize > 0x2000000)
    {
        printf("Invalid ROM size: %u\n", romSize);
        fclose(rf);
        return FALSE;
    }

    u8 *rom = malloc(romSize);
    if (!rom)
    {
        fclose(rf);
        return FALSE;
    }

    if (fread(rom, 1, romSize, rf) != romSize)
    {
        free(rom);
        fclose(rf);
        return FALSE;
    }
    fclose(rf);

    if (memcmp(&rom[0xAC], "BPRE", 4) != 0 && memcmp(&rom[0xAC], "BPGE", 4) != 0
     && memcmp(&rom[0xAC], "AXVE", 4) != 0 && memcmp(&rom[0xAC], "AXPE", 4) != 0)
    {
        printf("Not a supported Pokemon ROM. Game code: %.4s\n", (char *)&rom[0xAC]);
        free(rom);
        return FALSE;
    }

    printf("Extracting: %.12s [%.4s] (%u bytes)\n",
           (char *)&rom[0xA0], (char *)&rom[0xAC], romSize);

    struct PkmnHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = PKMN_MAGIC;
    header.version = PKMN_VERSION;
    header.flags = PKMN_FLAG_HAS_ROM | PKMN_FLAG_EXTRACTED;
    header.romSize = romSize;
    header.romCrc32 = ComputeCrc32(rom, romSize);
    header.assetCount = 0;
    header.tocOffset = 0;
    header.stringTableOffset = 0;
    memcpy(header.gameCode, &rom[0xAC], 4);
    memcpy(header.gameTitle, &rom[0xA0], 12);

    // Write v2 manifest with embedded ROM (for backwards compat + data access)
    FILE *of = fopen(outPath, "wb");
    if (!of)
    {
        printf("Cannot create: %s\n", outPath);
        free(rom);
        return FALSE;
    }

    fwrite(&header, sizeof(header), 1, of);
    fwrite(rom, 1, romSize, of);
    fclose(of);

    printf("Created %s (CRC32: %08X)\n", outPath, header.romCrc32);

    printf("Extracting game assets...\n");
    fflush(stdout);
    ExtractAssetsFolder(rom, romSize);
    free(rom);

    return TRUE;
}

#endif // PORTABLE
