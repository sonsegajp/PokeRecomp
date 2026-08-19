#ifndef GUARD_PLATFORM_PKMN_ARCHIVE_H
#define GUARD_PLATFORM_PKMN_ARCHIVE_H

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PKMN_MAGIC       0x4E4D4B50  /* "PKMN" */
#define PKMN_VERSION     2
#define PKMN_VERSION_V1  1
#define PKMN_MAX_PATH    256

#define PKMN_FLAG_HAS_ROM      (1 << 0)
#define PKMN_FLAG_HAS_ASSETS   (1 << 1)
#define PKMN_FLAG_EXTRACTED    (1 << 2)  // v2: assets are in extracted_assets/ folder

enum PkmnAssetType {
    PKMN_ASSET_RAW        = 0,
    PKMN_ASSET_GFX_4BPP   = 1,
    PKMN_ASSET_GFX_8BPP   = 2,
    PKMN_ASSET_PALETTE     = 3,
    PKMN_ASSET_TILEMAP     = 4,
    PKMN_ASSET_MUSIC       = 5,
    PKMN_ASSET_SFX         = 6,
    PKMN_ASSET_MAP         = 7,
    PKMN_ASSET_TEXT        = 8,
};

#pragma pack(push, 1)

struct PkmnHeader {
    u32 magic;
    u32 version;
    u32 flags;
    u32 romSize;
    u32 romCrc32;
    u32 assetCount;
    u32 tocOffset;
    u32 stringTableOffset;
    char gameCode[4];
    char gameTitle[12];
    u8 reserved[16];
};

struct PkmnTocEntry {
    u32 pathOffset;
    u32 dataOffset;
    u32 size;
    u32 compressedSize;
    u16 type;
    u16 flags;
};

#pragma pack(pop)

struct PkmnArchive {
    struct PkmnHeader header;
    u8 *romData;
    struct PkmnTocEntry *toc;
    char *stringTable;
    u32 stringTableSize;
    FILE *file;
};

bool8 PkmnArchive_Open(struct PkmnArchive *archive, const char *path);
void PkmnArchive_Close(struct PkmnArchive *archive);
const u8 *PkmnArchive_GetRomData(struct PkmnArchive *archive);
u32 PkmnArchive_GetRomSize(struct PkmnArchive *archive);
const u8 *PkmnArchive_GetAsset(struct PkmnArchive *archive, const char *path, u32 *outSize);
bool8 PkmnArchive_CreateFromRom(const char *romPath, const char *outPath);

#ifdef __cplusplus
}
#endif

#endif // GUARD_PLATFORM_PKMN_ARCHIVE_H
