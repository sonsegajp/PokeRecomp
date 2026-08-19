#ifdef PORTABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "platform/json_parser.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#endif

#define ASSET_BASE_DIR "extracted_assets/frlg"

// gSpeciesInfo and gBattleMoves are declared const, so the linker places them in
// read-only pages. The data overrides below write to them, which faults on every
// single field — 386 faults for species, MOVES_COUNT for moves, every launch.
// The crash guard used to swallow those faults, which meant the overrides silently
// never applied and the guard's fake-unwind slowly corrupted the stack.
// Make the pages writable up front instead.
static void AssetLoader_Unprotect(const void *addr, size_t size)
{
    if (addr == NULL || size == 0) return;
#ifdef _WIN32
    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)addr, size, PAGE_READWRITE, &oldProtect))
        printf("[ASSETS] VirtualProtect failed at %p (%u bytes), err=%lu\n",
               addr, (unsigned)size, GetLastError());
#else
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) pageSize = 4096;
    uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(pageSize - 1);
    size_t len = ((uintptr_t)addr + size) - start;
    if (mprotect((void *)start, len, PROT_READ | PROT_WRITE) != 0)
        printf("[ASSETS] mprotect failed at %p (%u bytes)\n", addr, (unsigned)size);
#endif
}

static bool8 sAssetLoaderInitialized = FALSE;
static bool8 sUseExtractedAssets = FALSE;

// Species name lookup table (built from ROM at init)
static char sSpeciesNames[NUM_SPECIES + 1][POKEMON_NAME_LENGTH + 1];

static void AssetLoader_LoadDataOverrides(void);

void AssetLoader_Init(void)
{
    if (sAssetLoaderInitialized) return;
    sAssetLoaderInitialized = TRUE;

    FILE *f = fopen(ASSET_BASE_DIR "/pokemon/bulbasaur/front.png", "rb");
    if (f)
    {
        fclose(f);
        sUseExtractedAssets = TRUE;
        printf("[ASSETS] Using extracted assets from %s/\n", ASSET_BASE_DIR);
        AssetLoader_LoadDataOverrides();
    }
    else
    {
        sUseExtractedAssets = FALSE;
        printf("[ASSETS] No extracted assets found, using ROM data\n");
    }
    fflush(stdout);
}

// ---- Pokemon data override ----
static void PokemonDataCallback(int index, const struct JsonObject *obj, void *userData)
{
    extern const struct SpeciesInfo gSpeciesInfo[];
    int id = json_get_int(obj, "id", -1);
    if (id <= 0 || id >= NUM_SPECIES) return;

    struct SpeciesInfo *info = (struct SpeciesInfo *)&gSpeciesInfo[id];
    int v;

    v = json_get_int(obj, "hp", -1);        if (v >= 0) info->baseHP = v;
    v = json_get_int(obj, "attack", -1);     if (v >= 0) info->baseAttack = v;
    v = json_get_int(obj, "defense", -1);    if (v >= 0) info->baseDefense = v;
    v = json_get_int(obj, "speed", -1);      if (v >= 0) info->baseSpeed = v;
    v = json_get_int(obj, "sp_attack", -1);  if (v >= 0) info->baseSpAttack = v;
    v = json_get_int(obj, "sp_defense", -1); if (v >= 0) info->baseSpDefense = v;
    v = json_get_int(obj, "type1", -1);      if (v >= 0) info->types[0] = v;
    v = json_get_int(obj, "type2", -1);      if (v >= 0) info->types[1] = v;
    v = json_get_int(obj, "catch_rate", -1); if (v >= 0) info->catchRate = v;
    v = json_get_int(obj, "exp_yield", -1);  if (v >= 0) info->expYield = v;
}

// ---- Move data override ----
static void MoveDataCallback(int index, const struct JsonObject *obj, void *userData)
{
    extern const struct BattleMove gBattleMoves[];
    int id = json_get_int(obj, "id", -1);
    if (id < 0 || id >= MOVES_COUNT) return;

    struct BattleMove *move = (struct BattleMove *)&gBattleMoves[id];
    int v;

    v = json_get_int(obj, "effect", -1);         if (v >= 0) move->effect = v;
    v = json_get_int(obj, "power", -1);           if (v >= 0) move->power = v;
    v = json_get_int(obj, "type", -1);            if (v >= 0) move->type = v;
    v = json_get_int(obj, "accuracy", -1);        if (v >= 0) move->accuracy = v;
    v = json_get_int(obj, "pp", -1);              if (v >= 0) move->pp = v;
    v = json_get_int(obj, "effect_chance", -1);   if (v >= 0) move->secondaryEffectChance = v;
    v = json_get_int(obj, "target", -1);          if (v >= 0) move->target = v;
    v = json_get_int(obj, "priority", -999);      if (v != -999) move->priority = v;
}

static void AssetLoader_LoadDataOverrides(void)
{
    char *data;

    {
        extern const struct SpeciesInfo gSpeciesInfo[];
        extern const struct BattleMove gBattleMoves[];
        AssetLoader_Unprotect(gSpeciesInfo, sizeof(struct SpeciesInfo) * (NUM_SPECIES + 1));
        AssetLoader_Unprotect(gBattleMoves, sizeof(struct BattleMove) * MOVES_COUNT);
    }

    // Pokemon stats
    data = json_load_file(ASSET_BASE_DIR "/data/pokemon.json");
    if (data)
    {
        int count = json_parse_array(data, PokemonDataCallback, NULL);
        printf("[ASSETS] Loaded %d Pokemon data overrides\n", count);
        free(data);
    }

    // Move data
    data = json_load_file(ASSET_BASE_DIR "/data/moves.json");
    if (data)
    {
        int count = json_parse_array(data, MoveDataCallback, NULL);
        printf("[ASSETS] Loaded %d move data overrides\n", count);
        free(data);
    }

    fflush(stdout);
}

bool8 AssetLoader_IsActive(void)
{
    return sUseExtractedAssets;
}

// Build a lowercase sanitized name from the GBA-encoded species name
static void BuildSpeciesPath(char *out, int outSize, u16 species, const char *filename)
{
    // Get species name from gSpeciesNames
    extern const u8 gSpeciesNames[][POKEMON_NAME_LENGTH + 1];
    char name[POKEMON_NAME_LENGTH + 1];
    int j = 0;

    for (int i = 0; i < POKEMON_NAME_LENGTH; i++)
    {
        u8 c = gSpeciesNames[species][i];
        if (c == 0xFF) break;
        if (c >= 0xBB && c <= 0xD4) name[j++] = 'a' + (c - 0xBB); // A-Z -> a-z
        else if (c >= 0xD5 && c <= 0xEE) name[j++] = 'a' + (c - 0xD5); // a-z
        else if (c >= 0xA1 && c <= 0xAA) name[j++] = '0' + (c - 0xA1);
        else if (c == 0x00) name[j++] = '_';
        else if (c == 0xAE) name[j++] = '-';
    }
    name[j] = '\0';

    snprintf(out, outSize, ASSET_BASE_DIR "/pokemon/%s/%s", name, filename);
}

// Try to load a raw file into a buffer. Returns size or 0 on failure.
int AssetLoader_LoadFile(const char *path, void *dest, int maxSize)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > maxSize) size = maxSize;
    int read = fread(dest, 1, size, f);
    fclose(f);
    return read;
}

// Try to load a Pokemon sprite from extracted assets
// Returns TRUE if loaded, FALSE to fall back to ROM
bool8 AssetLoader_LoadPokePic(u16 species, void *dest, bool8 isFrontPic)
{
    if (!sUseExtractedAssets || species == 0 || species > NUM_SPECIES)
        return FALSE;

    char path[256];
    BuildSpeciesPath(path, sizeof(path), species, isFrontPic ? "front.4bpp" : "back.4bpp");

    int size = AssetLoader_LoadFile(path, dest, 0x2000);
    return size > 0;
}

// Try to load a Pokemon palette from extracted assets
bool8 AssetLoader_LoadPokePalette(u16 species, void *dest, bool8 isShiny)
{
    if (!sUseExtractedAssets || species == 0 || species > NUM_SPECIES)
        return FALSE;

    char path[256];
    BuildSpeciesPath(path, sizeof(path), species, isShiny ? "shiny.gbapal" : "normal.gbapal");

    int size = AssetLoader_LoadFile(path, dest, 32);
    return size > 0;
}

// Try to load a Pokemon icon from extracted assets
bool8 AssetLoader_LoadPokeIcon(u16 species, void *dest)
{
    if (!sUseExtractedAssets || species == 0 || species > NUM_SPECIES)
        return FALSE;

    char path[256];
    BuildSpeciesPath(path, sizeof(path), species, "icon.4bpp");

    int size = AssetLoader_LoadFile(path, dest, 0x1000);
    return size > 0;
}

// Try to load a trainer front sprite from extracted assets
bool8 AssetLoader_LoadTrainerFrontPic(u16 trainerPicId, void *dest)
{
    if (!sUseExtractedAssets) return FALSE;
    char path[256];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/trainers/%03d/front.4bpp", trainerPicId);
    int size = AssetLoader_LoadFile(path, dest, 0x2000);
    return size > 0;
}

bool8 AssetLoader_LoadTrainerPalette(u16 trainerPicId, void *dest)
{
    if (!sUseExtractedAssets) return FALSE;
    char path[256];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/trainers/%03d/palette.gbapal", trainerPicId);
    int size = AssetLoader_LoadFile(path, dest, 32);
    return size > 0;
}

bool8 AssetLoader_LoadOverworldSprite(u16 gfxId, void *dest, int maxSize)
{
    if (!sUseExtractedAssets) return FALSE;
    char path[256];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/overworld/%03d.4bpp", gfxId);
    int size = AssetLoader_LoadFile(path, dest, maxSize);
    return size > 0;
}

// Load a graphics asset by its source path (e.g., "graphics/pokemon/bulbasaur/front.4bpp")
// This allows any INCBIN path to be overridden
bool8 AssetLoader_LoadBySourcePath(const char *sourcePath, void *dest, int maxSize)
{
    if (!sUseExtractedAssets) return FALSE;

    char path[512];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/%s", sourcePath);
    int size = AssetLoader_LoadFile(path, dest, maxSize);
    return size > 0;
}

// Load a compressed palette from file (raw gbapal, no LZ)
bool8 AssetLoader_LoadPaletteBySourcePath(const char *sourcePath, void *dest)
{
    if (!sUseExtractedAssets) return FALSE;

    char path[512];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/%s", sourcePath);
    int size = AssetLoader_LoadFile(path, dest, 512);
    return size > 0;
}

// Generic asset load by path relative to extracted_assets/
int AssetLoader_Load(const char *relativePath, void *dest, int maxSize)
{
    if (!sUseExtractedAssets) return 0;

    char path[512];
    snprintf(path, sizeof(path), ASSET_BASE_DIR "/%s", relativePath);
    return AssetLoader_LoadFile(path, dest, maxSize);
}

// ---- Asset Override Registry ----
// Maps compiled-in data pointers to file paths.
// When the engine tries to decompress or copy data from a known pointer,
// the registry is checked first for an override file.

#define MAX_OVERRIDES 256
static struct {
    const void *originalPtr;
    char path[128];
} sOverrides[MAX_OVERRIDES];
static int sOverrideCount = 0;

void AssetLoader_RegisterOverride(const void *dataPtr, const char *filePath)
{
    if (sOverrideCount >= MAX_OVERRIDES) return;
    sOverrides[sOverrideCount].originalPtr = dataPtr;
    strncpy(sOverrides[sOverrideCount].path, filePath, 127);
    sOverrides[sOverrideCount].path[127] = '\0';
    sOverrideCount++;
}

// Check if a data pointer has an override file. Returns loaded size or 0.
int AssetLoader_TryOverride(const void *dataPtr, void *dest, int maxSize)
{
    if (!sUseExtractedAssets) return 0;
    for (int i = 0; i < sOverrideCount; i++)
    {
        if (sOverrides[i].originalPtr == dataPtr)
        {
            char path[512];
            snprintf(path, sizeof(path), ASSET_BASE_DIR "/%s", sOverrides[i].path);
            return AssetLoader_LoadFile(path, dest, maxSize);
        }
    }
    return 0;
}

// ---- Hoenn Music Lookup ----
#define HOENN_MUSIC_BASE 347
static const char *sHoennMusicNames[] = {
    "mus_littleroot",   // 347
    "mus_oldale",       // 348
    "mus_petalburg",    // 349
    "mus_rustboro",     // 350
    "mus_dewford",      // 351
    "mus_slateport",    // 352
    "mus_cycling",      // 353
    "mus_verdanturf",   // 354
    "mus_fallarbor",    // 355
    "mus_route113",     // 356
    "mus_fortree",      // 357
    "mus_lilycove",     // 358
    "mus_route119",     // 359
    "mus_sootopolis",   // 360
    "mus_ever_grande",  // 361
    "mus_route101",     // 362
    "mus_route104",     // 363
    "mus_route110",     // 364
    "mus_route120",     // 365
    "mus_route122",     // 366
    "mus_surf",         // 367
    "mus_mt_chimney",   // 368
    "mus_gym",              // 369
    "mus_poke_center",      // 370
    "mus_poke_mart",        // 371
    "mus_vs_wild",          // 372
    "mus_vs_trainer",       // 373
    "mus_vs_gym_leader",    // 374
    "mus_vs_champion",      // 375
    "mus_vs_elite_four",    // 376
    "mus_vs_aqua_magma",    // 377
    "mus_vs_rival",         // 378
    "mus_victory_trainer",  // 379
    "mus_victory_aqua_magma", // 380
};
#define HOENN_MUSIC_COUNT (sizeof(sHoennMusicNames) / sizeof(sHoennMusicNames[0]))

// ---- Music Override ----
bool8 AssetLoader_GetMusicOverridePath(u16 songId, char *outPath, int outSize)
{
    // Hoenn music plays natively through m4a engine via rb_ prefixed song data
    // No MIDI override needed

    if (!sUseExtractedAssets) return FALSE;

    snprintf(outPath, outSize, ASSET_BASE_DIR "/sound/songs/midi/mus_%03d.ogg", songId);
    FILE *f = fopen(outPath, "rb");
    if (f) { fclose(f); return TRUE; }

    snprintf(outPath, outSize, ASSET_BASE_DIR "/sound/songs/midi/mus_%03d.wav", songId);
    f = fopen(outPath, "rb");
    if (f) { fclose(f); return TRUE; }

    snprintf(outPath, outSize, ASSET_BASE_DIR "/sound/songs/midi/mus_%03d.mid", songId);
    f = fopen(outPath, "rb");
    if (f) { fclose(f); return TRUE; }

    return FALSE;
}

#endif // PORTABLE
