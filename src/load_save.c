#include "global.h"
#include "gflib.h"
#include "gba/flash_internal.h"
#include "load_save.h"
#ifdef PORTABLE
#include "platform.h"
#include <string.h>
#endif
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "item.h"
#include "save_location.h"
#include "berry_powder.h"
#include "overworld.h"
#include "quest_log.h"
#include "sloopsvc.h"

#define SAVEBLOCK_MOVE_RANGE    128

struct LoadedSaveData
{
 /*0x0000*/ struct ItemSlot items[BAG_ITEMS_COUNT];
 /*0x0078*/ struct ItemSlot keyItems[BAG_KEYITEMS_COUNT];
 /*0x00F0*/ struct ItemSlot pokeBalls[BAG_POKEBALLS_COUNT];
 /*0x0130*/ struct ItemSlot TMsHMs[BAG_TMHM_COUNT];
 /*0x0230*/ struct ItemSlot berries[BAG_BERRIES_COUNT];
 /*0x02E8*/ struct Mail mail[MAIL_COUNT];
};

// EWRAM DATA
EWRAM_DATA struct SaveBlock2 gSaveBlock2 = {0};
EWRAM_DATA u8 gSaveBlock2_DMA[SAVEBLOCK_MOVE_RANGE] = {0};

EWRAM_DATA struct SaveBlock1 gSaveBlock1 = {0};
EWRAM_DATA u8 gSaveBlock1_DMA[SAVEBLOCK_MOVE_RANGE] = {0};

EWRAM_DATA struct PokemonStorage gPokemonStorage = {0};
EWRAM_DATA u8 gSaveBlock3_DMA[SAVEBLOCK_MOVE_RANGE] = {0};

EWRAM_DATA struct LoadedSaveData gLoadedSaveData = {0};
EWRAM_DATA u32 gLastEncryptionKey = 0;

// IWRAM common
COMMON_DATA bool32 gFlashMemoryPresent = 0;
COMMON_DATA struct SaveBlock1 *gSaveBlock1Ptr = NULL;
COMMON_DATA struct SaveBlock2 *gSaveBlock2Ptr = NULL;
COMMON_DATA struct PokemonStorage *gPokemonStoragePtr = NULL;

#ifdef PORTABLE
static u16 Platform_EraseFlashSector(u16 sectorNum)
{
    u32 offset = sectorNum << gFlash->sector.shift;
    u32 size = 1 << gFlash->sector.shift;
    memset(&FLASH_BASE[offset], 0xFF, size);
    Platform_StoreSaveFile();
    return 0;
}

static u16 Platform_ProgramFlashSector(u16 sectorNum, void *src)
{
    u32 offset = sectorNum << gFlash->sector.shift;
    u32 size = 1 << gFlash->sector.shift;
    memcpy(&FLASH_BASE[offset], src, size);
    Platform_StoreSaveFile();
    return 0;
}

static u16 Platform_ProgramFlashByte(u16 sectorNum, u32 byteOffset, u8 data)
{
    u32 offset = (sectorNum << gFlash->sector.shift) + byteOffset;
    FLASH_BASE[offset] = data;
    return 0;
}
#endif

void CheckForFlashMemory(void)
{
#ifdef PORTABLE
    IdentifyFlash();
    gFlashMemoryPresent = TRUE;
    EraseFlashSector = Platform_EraseFlashSector;
    ProgramFlashSector = Platform_ProgramFlashSector;
    ProgramFlashByte = Platform_ProgramFlashByte;
#else
    if (!IdentifyFlash())
    {
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    else
    {
        gFlashMemoryPresent = FALSE;
    }
#endif
}

void ClearSav2(void)
{
#ifdef PORTABLE
    memset(&gSaveBlock2, 0, sizeof(struct SaveBlock2) + sizeof(gSaveBlock2_DMA));
#else
    CpuFill16(0, &gSaveBlock2, sizeof(struct SaveBlock2) + sizeof(gSaveBlock2_DMA));
#endif
}

void ClearSav1(void)
{
#ifdef PORTABLE
    memset(&gSaveBlock1, 0, sizeof(struct SaveBlock1) + sizeof(gSaveBlock1_DMA));
#else
    CpuFill16(0, &gSaveBlock1, sizeof(struct SaveBlock1) + sizeof(gSaveBlock1_DMA));
#endif
}

void SetSaveBlocksPointers(void)
{
    u32 offset;
    struct SaveBlock1** sav1_LocalVar = &gSaveBlock1Ptr;
    void *oldSave = (void *)gSaveBlock1Ptr;

#ifdef PORTABLE
    offset = 0;
#else
    offset = (Random()) & ((SAVEBLOCK_MOVE_RANGE - 1) & ~3);
#endif

    gSaveBlock2Ptr = (void *)(&gSaveBlock2) + offset;
    *sav1_LocalVar = (void *)(&gSaveBlock1) + offset;
    gPokemonStoragePtr = (void *)(&gPokemonStorage) + offset;

#ifdef PORTABLE
    printf("[SetSaveBlocksPointers] sb2=%p (&gSaveBlock2=%p offset=%u)\n",
           gSaveBlock2Ptr, &gSaveBlock2, offset);
#endif
    SetBagPocketsPointers();
    QL_AddASLROffset(oldSave);
}

void MoveSaveBlocks_ResetHeap(void)
{
    void *vblankCB, *hblankCB;
    u32 encryptionKey;

    // save interrupt functions and turn them off
    vblankCB = gMain.vblankCallback;
    hblankCB = gMain.hblankCallback;
    gMain.vblankCallback = NULL;
    gMain.hblankCallback = NULL;
    gMain.vblankCounter1 = NULL;

#ifdef PORTABLE
    gSaveBlock2Ptr = &gSaveBlock2;
    gSaveBlock1Ptr = &gSaveBlock1;
    gPokemonStoragePtr = &gPokemonStorage;
    SetBagPocketsPointers();
#else
    {
    struct SaveBlock2 *saveBlock2Copy;
    struct SaveBlock1 *saveBlock1Copy;
    struct PokemonStorage *pokemonStorageCopy;

    saveBlock2Copy = (struct SaveBlock2 *)(gHeap);
    saveBlock1Copy = (struct SaveBlock1 *)(gHeap + sizeof(struct SaveBlock2));
    pokemonStorageCopy = (struct PokemonStorage *)(gHeap + sizeof(struct SaveBlock2) + sizeof(struct SaveBlock1));

    *saveBlock2Copy = *gSaveBlock2Ptr;
    *saveBlock1Copy = *gSaveBlock1Ptr;
    *pokemonStorageCopy = *gPokemonStoragePtr;

    SetSaveBlocksPointers();

    *gSaveBlock2Ptr = *saveBlock2Copy;
    *gSaveBlock1Ptr = *saveBlock1Copy;
    *gPokemonStoragePtr = *pokemonStorageCopy;
    }
#endif

#ifdef PORTABLE
    {
        extern void ClearDMAList(void);
        extern void ResetSpriteData(void);
        extern void ResetQuestLog(void);
        extern struct Sprite gSprites[];
        ClearDMAList();
        ResetSpriteData();
        ResetQuestLog();
        for (int i = 0; i < 64; i++) {
            gSprites[i].template = NULL;
            gSprites[i].callback = NULL;
            gSprites[i].inUse = FALSE;
        }
    }
#endif

    InitHeap(gHeap, HEAP_SIZE);


    // restore interrupt functions
    gMain.hblankCallback = hblankCB;
    gMain.vblankCallback = vblankCB;

    // create a new encryption key
    encryptionKey = (Random() << 0x10) + (Random());
    ApplyNewEncryptionKeyToAllEncryptedData(encryptionKey);
    gSaveBlock2Ptr->encryptionKey = encryptionKey;
#if REVISION >= 0xA
    svc_SetSaveBlock2(gSaveBlock2Ptr);
#endif
}

u32 UseContinueGameWarp(void)
{
    return gSaveBlock2Ptr->specialSaveWarpFlags & CONTINUE_GAME_WARP;
}

void ClearContinueGameWarpStatus(void)
{
    gSaveBlock2Ptr->specialSaveWarpFlags &= ~CONTINUE_GAME_WARP;
}

void SetContinueGameWarpStatus(void)
{
    gSaveBlock2Ptr->specialSaveWarpFlags |= CONTINUE_GAME_WARP;
}

void SetContinueGameWarpStatusToDynamicWarp(void)
{
    SetContinueGameWarpToDynamicWarp(0);
    gSaveBlock2Ptr->specialSaveWarpFlags |= CONTINUE_GAME_WARP;
}

void ClearContinueGameWarpStatus2(void)
{
    gSaveBlock2Ptr->specialSaveWarpFlags &= ~CONTINUE_GAME_WARP;
}

void SavePlayerParty(void)
{
    int i;

    gSaveBlock1Ptr->playerPartyCount = gPlayerPartyCount;

    for (i = 0; i < PARTY_SIZE; i++)
        gSaveBlock1Ptr->playerParty[i] = gPlayerParty[i];
}

void LoadPlayerParty(void)
{
    int i;

    gPlayerPartyCount = gSaveBlock1Ptr->playerPartyCount;

    for (i = 0; i < PARTY_SIZE; i++)
        gPlayerParty[i] = gSaveBlock1Ptr->playerParty[i];
}

void SaveObjectEvents(void)
{
    int i;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        gSaveBlock1Ptr->objectEvents[i] = gObjectEvents[i];

#ifdef PORTABLE
    {
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        {
            u8 lid = gSaveBlock1Ptr->objectEvents[i].localId;
            if (lid >= 240 && lid != 255)
                memset(&gSaveBlock1Ptr->objectEvents[i], 0, sizeof(struct ObjectEvent));
        }
    }
#endif
}

void LoadObjectEvents(void)
{
    int i;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        gObjectEvents[i] = gSaveBlock1Ptr->objectEvents[i];
}

void SaveSerializedGame(void)
{
    SavePlayerParty();
    SaveObjectEvents();
}

void LoadSerializedGame(void)
{
    LoadPlayerParty();
    LoadObjectEvents();
}

void LoadPlayerBag(void)
{
    int i;

    // load player items.
    for (i = 0; i < BAG_ITEMS_COUNT; i++)
        gLoadedSaveData.items[i] = gSaveBlock1Ptr->bagPocket_Items[i];

    // load player key items.
    for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
        gLoadedSaveData.keyItems[i] = gSaveBlock1Ptr->bagPocket_KeyItems[i];

    // load player pokeballs.
    for (i = 0; i < BAG_POKEBALLS_COUNT; i++)
        gLoadedSaveData.pokeBalls[i] = gSaveBlock1Ptr->bagPocket_PokeBalls[i];

    // load player TMs and HMs.
    for (i = 0; i < BAG_TMHM_COUNT; i++)
        gLoadedSaveData.TMsHMs[i] = gSaveBlock1Ptr->bagPocket_TMHM[i];

    // load player berries.
    for (i = 0; i < BAG_BERRIES_COUNT; i++)
        gLoadedSaveData.berries[i] = gSaveBlock1Ptr->bagPocket_Berries[i];

    // load mail.
    for (i = 0; i < MAIL_COUNT; i++)
        gLoadedSaveData.mail[i] = gSaveBlock1Ptr->mail[i];

    gLastEncryptionKey = gSaveBlock2Ptr->encryptionKey;
}

void SavePlayerBag(void)
{
    int i;
    u32 encryptionKeyBackup;

    // save player items.
    for (i = 0; i < BAG_ITEMS_COUNT; i++)
        gSaveBlock1Ptr->bagPocket_Items[i] = gLoadedSaveData.items[i];

    // save player key items.
    for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
        gSaveBlock1Ptr->bagPocket_KeyItems[i] = gLoadedSaveData.keyItems[i];

    // save player pokeballs.
    for (i = 0; i < BAG_POKEBALLS_COUNT; i++)
        gSaveBlock1Ptr->bagPocket_PokeBalls[i] = gLoadedSaveData.pokeBalls[i];

    // save player TMs and HMs.
    for (i = 0; i < BAG_TMHM_COUNT; i++)
        gSaveBlock1Ptr->bagPocket_TMHM[i] = gLoadedSaveData.TMsHMs[i];

    // save player berries.
    for (i = 0; i < BAG_BERRIES_COUNT; i++)
        gSaveBlock1Ptr->bagPocket_Berries[i] = gLoadedSaveData.berries[i];

    // save mail.
    for (i = 0; i < MAIL_COUNT; i++)
        gSaveBlock1Ptr->mail[i] = gLoadedSaveData.mail[i];

    encryptionKeyBackup = gSaveBlock2Ptr->encryptionKey;
    gSaveBlock2Ptr->encryptionKey = gLastEncryptionKey;
    ApplyNewEncryptionKeyToBagItems(encryptionKeyBackup);
    gSaveBlock2Ptr->encryptionKey = encryptionKeyBackup;
}

void ApplyNewEncryptionKeyToHword(u16 *hWord, u32 newKey)
{
    *hWord ^= gSaveBlock2Ptr->encryptionKey;
    *hWord ^= newKey;
}

void ApplyNewEncryptionKeyToWord(u32 *word, u32 newKey)
{
    *word ^= gSaveBlock2Ptr->encryptionKey;
    *word ^= newKey;
}

void ApplyNewEncryptionKeyToAllEncryptedData(u32 encryptionKey)
{
    int i;

    for(i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
        ApplyNewEncryptionKeyToWord(&gSaveBlock1Ptr->trainerTower[i].bestTime, encryptionKey);

    ApplyNewEncryptionKeyToGameStats(encryptionKey);
    ApplyNewEncryptionKeyToBagItems_(encryptionKey);
    ApplyNewEncryptionKeyToBerryPowder(encryptionKey);
    ApplyNewEncryptionKeyToWord(&gSaveBlock1Ptr->money, encryptionKey);
    ApplyNewEncryptionKeyToHword(&gSaveBlock1Ptr->coins, encryptionKey);
}
