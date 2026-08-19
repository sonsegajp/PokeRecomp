#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PKMN_MAGIC       0x4E4D4B50
#define PKMN_VERSION     1
#define PKMN_FLAG_HAS_ROM    (1 << 0)
#define PKMN_FLAG_HAS_ASSETS (1 << 1)

#define ROM_GAME_CODE_OFFSET  0xAC
#define ROM_GAME_TITLE_OFFSET 0xA0

#pragma pack(push, 1)

struct PkmnHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t flags;
    uint32_t romSize;
    uint32_t romCrc32;
    uint32_t assetCount;
    uint32_t tocOffset;
    uint32_t stringTableOffset;
    char gameCode[4];
    char gameTitle[12];
    uint8_t reserved[16];
};

struct PkmnTocEntry {
    uint32_t pathOffset;
    uint32_t dataOffset;
    uint32_t size;
    uint32_t compressedSize;
    uint16_t type;
    uint16_t flags;
};

#pragma pack(pop)

enum AssetType {
    ASSET_RAW      = 0,
    ASSET_GFX_4BPP = 1,
    ASSET_GFX_8BPP = 2,
    ASSET_PALETTE  = 3,
    ASSET_TILEMAP  = 4,
    ASSET_MUSIC    = 5,
    ASSET_SFX      = 6,
    ASSET_MAP      = 7,
    ASSET_TEXT     = 8,
};

static uint32_t crc32_table[256];
static int crc32_initialized = 0;

static void init_crc32(void)
{
    if (crc32_initialized) return;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (c & 1 ? 0xEDB88320 : 0);
        crc32_table[i] = c;
    }
    crc32_initialized = 1;
}

static uint32_t compute_crc32(const uint8_t *data, uint32_t size)
{
    init_crc32();
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < size; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFF;
}

struct KnownAsset {
    const char *path;
    uint32_t romOffset;
    uint32_t size;
    uint16_t type;
};

static uint32_t lz77_get_decompressed_size(const uint8_t *data)
{
    if ((data[0] & 0xF0) != 0x10) return 0;
    return (data[3] << 16) | (data[2] << 8) | data[1];
}

static uint32_t lz77_get_compressed_size(const uint8_t *data, uint32_t maxScan)
{
    uint32_t destSize = lz77_get_decompressed_size(data);
    if (destSize == 0) return 0;

    uint32_t srcPos = 4;
    uint32_t destPos = 0;

    while (destPos < destSize && srcPos < maxScan)
    {
        uint8_t flags = data[srcPos++];
        for (int i = 0; i < 8 && destPos < destSize; i++)
        {
            if (flags & 0x80)
            {
                if (srcPos + 1 >= maxScan) return srcPos;
                uint32_t blockSize = (data[srcPos] >> 4) + 3;
                srcPos += 2;
                destPos += blockSize;
            }
            else
            {
                srcPos++;
                destPos++;
            }
            flags <<= 1;
        }
    }
    return srcPos;
}

#define GBA_PTR_TO_OFFSET(ptr) ((ptr) & 0x01FFFFFF)
#define IS_GBA_PTR(ptr)        (((ptr) & 0xFE000000) == 0x08000000)

struct StringBuf {
    char *data;
    uint32_t size;
    uint32_t capacity;
};

static void strbuf_init(struct StringBuf *sb)
{
    sb->capacity = 4096;
    sb->data = malloc(sb->capacity);
    sb->size = 0;
}

static uint32_t strbuf_add(struct StringBuf *sb, const char *str)
{
    uint32_t offset = sb->size;
    uint32_t len = strlen(str) + 1;
    while (sb->size + len > sb->capacity)
    {
        sb->capacity *= 2;
        sb->data = realloc(sb->data, sb->capacity);
    }
    memcpy(sb->data + sb->size, str, len);
    sb->size += len;
    return offset;
}

struct AssetList {
    struct PkmnTocEntry *entries;
    uint32_t count;
    uint32_t capacity;
};

static void assetlist_init(struct AssetList *al)
{
    al->capacity = 1024;
    al->entries = malloc(al->capacity * sizeof(struct PkmnTocEntry));
    al->count = 0;
}

static void assetlist_add(struct AssetList *al, struct PkmnTocEntry entry)
{
    if (al->count >= al->capacity)
    {
        al->capacity *= 2;
        al->entries = realloc(al->entries, al->capacity * sizeof(struct PkmnTocEntry));
    }
    al->entries[al->count++] = entry;
}

static void extract_pokemon_sprites(const uint8_t *rom, uint32_t romSize,
                                    struct AssetList *assets, struct StringBuf *strings)
{
    /*
     * Pokemon front/back pic tables are arrays of {ptr, size, tag}.
     * Each entry is 8 bytes: u32 data_ptr, u16 size, u16 tag.
     * We scan for known table patterns.
     *
     * For Phase 1, we store entire ROM sections as assets.
     * Individual sprite extraction will be added in Phase 2.
     */
    char path[256];

    /* Scan ROM for LZ77-compressed graphics blocks.
     * LZ77 header: byte 0 = 0x10, bytes 1-3 = decompressed size (LE). */
    uint32_t gfxCount = 0;
    for (uint32_t offset = 0x200000; offset < romSize - 4 && gfxCount < 2000; offset += 4)
    {
        if (rom[offset] != 0x10) continue;
        uint32_t decompSize = lz77_get_decompressed_size(&rom[offset]);
        if (decompSize < 32 || decompSize > 0x10000) continue;
        if (decompSize % 32 != 0) continue;

        uint32_t compSize = lz77_get_compressed_size(&rom[offset], romSize - offset);
        if (compSize < 8 || compSize > decompSize) continue;

        snprintf(path, sizeof(path), "gfx/rom/0x%06X.4bpp.lz", offset);

        struct PkmnTocEntry entry;
        entry.pathOffset = strbuf_add(strings, path);
        entry.dataOffset = offset;
        entry.size = compSize;
        entry.compressedSize = compSize;
        entry.type = ASSET_GFX_4BPP;
        entry.flags = 0;
        assetlist_add(assets, entry);
        gfxCount++;

        offset += ((compSize + 3) & ~3) - 4;
    }
    printf("  Found %u compressed graphics blocks\n", gfxCount);
}

static void extract_palette_data(const uint8_t *rom, uint32_t romSize,
                                 struct AssetList *assets, struct StringBuf *strings)
{
    char path[256];
    uint32_t palCount = 0;

    for (uint32_t offset = 0x200000; offset < romSize - 4 && palCount < 1000; offset += 4)
    {
        if (rom[offset] != 0x10) continue;
        uint32_t decompSize = lz77_get_decompressed_size(&rom[offset]);
        if (decompSize != 32 && decompSize != 64 && decompSize != 512) continue;

        uint32_t compSize = lz77_get_compressed_size(&rom[offset], romSize - offset);
        if (compSize < 4 || compSize > decompSize) continue;

        snprintf(path, sizeof(path), "pal/rom/0x%06X.gbapal.lz", offset);

        struct PkmnTocEntry entry;
        entry.pathOffset = strbuf_add(strings, path);
        entry.dataOffset = offset;
        entry.size = compSize;
        entry.compressedSize = compSize;
        entry.type = ASSET_PALETTE;
        entry.flags = 0;
        assetlist_add(assets, entry);
        palCount++;

        offset += ((compSize + 3) & ~3) - 4;
    }
    printf("  Found %u compressed palette blocks\n", palCount);
}

static void extract_music_data(const uint8_t *rom, uint32_t romSize,
                               struct AssetList *assets, struct StringBuf *strings)
{
    /* Song table is at a known offset in FireRed US.
     * Each entry is a pointer to a song header. */
    const uint32_t SONG_TABLE_OFFSET = 0x4A0AA0;
    char path[256];
    uint32_t songCount = 0;

    if (SONG_TABLE_OFFSET + 4 > romSize) return;

    for (uint32_t i = 0; i < 500; i++)
    {
        uint32_t entryOff = SONG_TABLE_OFFSET + i * 8;
        if (entryOff + 8 > romSize) break;

        uint32_t songPtr = *(uint32_t *)&rom[entryOff];
        if (!IS_GBA_PTR(songPtr)) continue;

        uint32_t songOff = GBA_PTR_TO_OFFSET(songPtr);
        if (songOff >= romSize) continue;

        snprintf(path, sizeof(path), "music/song_%03u", i);

        struct PkmnTocEntry entry;
        entry.pathOffset = strbuf_add(strings, path);
        entry.dataOffset = songOff;
        entry.size = 0;
        entry.compressedSize = 0;
        entry.type = ASSET_MUSIC;
        entry.flags = 0;
        assetlist_add(assets, entry);
        songCount++;
    }
    printf("  Found %u music entries\n", songCount);
}

static void extract_map_data(const uint8_t *rom, uint32_t romSize,
                             struct AssetList *assets, struct StringBuf *strings)
{
    /* Map groups table pointer at known FireRed offset */
    const uint32_t MAP_GROUPS_PTR_OFFSET = 0x05524C;
    char path[256];
    uint32_t mapCount = 0;

    if (MAP_GROUPS_PTR_OFFSET + 4 > romSize) return;

    uint32_t mapGroupsPtr = *(uint32_t *)&rom[MAP_GROUPS_PTR_OFFSET];
    if (!IS_GBA_PTR(mapGroupsPtr)) return;

    uint32_t mapGroupsOff = GBA_PTR_TO_OFFSET(mapGroupsPtr);
    if (mapGroupsOff >= romSize) return;

    for (uint32_t group = 0; group < 50; group++)
    {
        uint32_t groupEntryOff = mapGroupsOff + group * 4;
        if (groupEntryOff + 4 > romSize) break;

        uint32_t groupPtr = *(uint32_t *)&rom[groupEntryOff];
        if (!IS_GBA_PTR(groupPtr)) break;

        uint32_t groupOff = GBA_PTR_TO_OFFSET(groupPtr);
        if (groupOff >= romSize) continue;

        for (uint32_t map = 0; map < 100; map++)
        {
            uint32_t mapEntryOff = groupOff + map * 4;
            if (mapEntryOff + 4 > romSize) break;

            uint32_t mapPtr = *(uint32_t *)&rom[mapEntryOff];
            if (!IS_GBA_PTR(mapPtr)) break;

            uint32_t mapOff = GBA_PTR_TO_OFFSET(mapPtr);
            if (mapOff >= romSize || mapOff < 0x1000) break;

            snprintf(path, sizeof(path), "maps/group%u/map%u", group, map);

            struct PkmnTocEntry entry;
            entry.pathOffset = strbuf_add(strings, path);
            entry.dataOffset = mapOff;
            entry.size = 0;
            entry.compressedSize = 0;
            entry.type = ASSET_MAP;
            entry.flags = 0;
            assetlist_add(assets, entry);
            mapCount++;
        }
    }
    printf("  Found %u map entries\n", mapCount);
}

int main(int argc, char *argv[])
{
    printf("pkmn_extract — Pokemon FireRed Asset Extractor\n");
    printf("Generates .pkmn archive from ROM\n\n");

    if (argc < 2)
    {
        printf("Usage: pkmn_extract <rom.gba> [output.pkmn]\n");
        return 1;
    }

    const char *romPath = argv[1];
    const char *outPath = argc > 2 ? argv[2] : "pokefirered.pkmn";

    FILE *romFile = fopen(romPath, "rb");
    if (!romFile)
    {
        printf("Error: Cannot open ROM file '%s'\n", romPath);
        return 1;
    }

    fseek(romFile, 0, SEEK_END);
    uint32_t romSize = ftell(romFile);
    fseek(romFile, 0, SEEK_SET);

    printf("ROM file: %s (%u bytes)\n", romPath, romSize);

    if (romSize < 0x100 || romSize > 0x2000000)
    {
        printf("Error: Invalid ROM size\n");
        fclose(romFile);
        return 1;
    }

    uint8_t *rom = malloc(romSize);
    if (!rom)
    {
        printf("Error: Out of memory\n");
        fclose(romFile);
        return 1;
    }

    if (fread(rom, 1, romSize, romFile) != romSize)
    {
        printf("Error: Failed to read ROM\n");
        free(rom);
        fclose(romFile);
        return 1;
    }
    fclose(romFile);

    if (memcmp(&rom[ROM_GAME_CODE_OFFSET], "BPRE", 4) != 0)
    {
        printf("Error: Not a Pokemon FireRed (US) ROM. Game code: %.4s\n",
               (char *)&rom[ROM_GAME_CODE_OFFSET]);
        free(rom);
        return 1;
    }

    printf("Game code: %.4s\n", (char *)&rom[ROM_GAME_CODE_OFFSET]);
    printf("Title: %.12s\n", (char *)&rom[ROM_GAME_TITLE_OFFSET]);

    uint32_t romCrc = compute_crc32(rom, romSize);
    printf("CRC32: %08X\n\n", romCrc);

    struct AssetList assets;
    struct StringBuf strings;
    assetlist_init(&assets);
    strbuf_init(&strings);

    printf("Extracting assets...\n");
    extract_pokemon_sprites(rom, romSize, &assets, &strings);
    extract_palette_data(rom, romSize, &assets, &strings);
    extract_music_data(rom, romSize, &assets, &strings);
    extract_map_data(rom, romSize, &assets, &strings);
    printf("\nTotal assets cataloged: %u\n\n", assets.count);

    struct PkmnHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = PKMN_MAGIC;
    header.version = PKMN_VERSION;
    header.flags = PKMN_FLAG_HAS_ROM | PKMN_FLAG_HAS_ASSETS;
    header.romSize = romSize;
    header.romCrc32 = romCrc;
    header.assetCount = assets.count;
    memcpy(header.gameCode, &rom[ROM_GAME_CODE_OFFSET], 4);
    memcpy(header.gameTitle, &rom[ROM_GAME_TITLE_OFFSET], 12);

    uint32_t romDataOffset = sizeof(struct PkmnHeader);
    uint32_t tocOffset = romDataOffset + romSize;
    tocOffset = (tocOffset + 15) & ~15;
    uint32_t tocSize = assets.count * sizeof(struct PkmnTocEntry);
    uint32_t stringTableOffset = tocOffset + tocSize;
    stringTableOffset = (stringTableOffset + 15) & ~15;

    header.tocOffset = tocOffset;
    header.stringTableOffset = stringTableOffset;

    printf("Writing %s...\n", outPath);
    FILE *outFile = fopen(outPath, "wb");
    if (!outFile)
    {
        printf("Error: Cannot create output file '%s'\n", outPath);
        free(rom);
        free(assets.entries);
        free(strings.data);
        return 1;
    }

    fwrite(&header, sizeof(header), 1, outFile);

    fwrite(rom, 1, romSize, outFile);

    uint32_t padding = tocOffset - (romDataOffset + romSize);
    if (padding > 0)
    {
        uint8_t zeroes[16] = {0};
        fwrite(zeroes, 1, padding, outFile);
    }

    fwrite(assets.entries, sizeof(struct PkmnTocEntry), assets.count, outFile);

    padding = stringTableOffset - (tocOffset + tocSize);
    if (padding > 0)
    {
        uint8_t zeroes[16] = {0};
        fwrite(zeroes, 1, padding, outFile);
    }

    fwrite(strings.data, 1, strings.size, outFile);

    uint32_t totalSize = stringTableOffset + strings.size;
    printf("Done! Output: %s (%u bytes)\n", outPath, totalSize);
    printf("  ROM data:     %u bytes (offset 0x%X)\n", romSize, romDataOffset);
    printf("  TOC:          %u entries (offset 0x%X)\n", assets.count, tocOffset);
    printf("  String table: %u bytes (offset 0x%X)\n", strings.size, stringTableOffset);

    fclose(outFile);
    free(rom);
    free(assets.entries);
    free(strings.data);

    return 0;
}
