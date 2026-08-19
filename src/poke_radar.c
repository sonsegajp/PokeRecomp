#include "global.h"
#include "gflib.h"
#include <stdio.h>

const u32 gItemIcon_PokeRadar[] = {
    0x00012010, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00C00000,
    0x1C000000, 0x21C00000, 0x00000000, 0x00CC0000, 0x33C00000, 0xA5C0000C, 0xC000CC0C, 0xCC11CC54,
    0x00221C54, 0x5521A4C1, 0x4421A4A1, 0x00000000, 0x00000000, 0x00CC0000, 0x0C110000, 0x22000000,
    0x220000C1, 0x00000C12, 0x00C12225, 0x0C125524, 0x1C000000, 0x21C00012, 0x1C0000A1, 0x21C0BA12,
    0xC0008BA1, 0xC088BA21, 0x00988A21, 0x698A21C0, 0x98A121C0, 0x21BBAA00, 0x15A88B55, 0x89880022,
    0x9798115B, 0x790055B8, 0x76BB8967, 0x00889767, 0x88896776, 0x99889776, 0x12442500, 0x1255220C,
    0x2222000C, 0x22210C12, 0x15000CA1, 0x5B0CAA12, 0x0000CAA1, 0x000CAA5B, 0x0000CA5B, 0x1215C000,
    0x215C008A, 0xC00000A1, 0x00001215, 0x0000215C, 0x0015C000, 0x005C0000, 0xC0000000, 0x00000000,
    0x98897900, 0xC98898CC, 0x988A0000, 0xC9A1000C, 0x11000000, 0xAA00000C, 0x0000000C, 0x00000CCC,
    0x00000000, 0x000CCC00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000,
};
const u32 gItemIconPalette_PokeRadar[] = {
    0x00002010, 0xDE000000, 0x8C6F7B7B, 0x4A570037, 0x76693DEF, 0x5D0071E4, 0x0818D029, 0x0018C621,
    0x00001084, 0x00000000,
};

#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "item_menu.h"
#include "item_use.h"
#include "metatile_behavior.h"
#include "new_menu_helpers.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "wild_encounter.h"
#include "window.h"

#ifdef PORTABLE
// FireRed and LeafGreen ship different wild encounter tables, so the PC build
// splits gWildMonHeaders into _FR/_LG variants and picks one at runtime. Same
// shim as src/wild_encounter.c and src/wild_pokemon_area.c — this file was
// missed, which left an unresolved reference to the plain gWildMonHeaders
// symbol that -Wl,--warn-unresolved-symbols quietly papered over, so the
// radar's level-range scan walked a bogus pointer.
//
// Must come *after* wild_encounter.h: the struct has to be complete, and the
// macro would otherwise mangle that header's own declaration of the symbol.
#include "game_version.h"
extern const struct WildPokemonHeader gWildMonHeaders_FR[];
extern const struct WildPokemonHeader gWildMonHeaders_LG[];
#undef gWildMonHeaders
#define gWildMonHeaders (gIsLeafGreen ? gWildMonHeaders_LG : gWildMonHeaders_FR)
#endif
#include "constants/field_effects.h"
#include "constants/metatile_behaviors.h"
#include "constants/songs.h"
#include "constants/species.h"

#define RADAR_MAX_PATCHES 4
#define RADAR_CHAIN_MAX 40
#define RADAR_RECHARGE_STEPS 50
#define NET_LOCAL_ID_START 240

struct RadarPatch {
    s16 x;
    s16 y;
    bool8 isShiny;
};

struct PokeRadarState {
    bool8 active;
    u16 chainSpecies;
    u8 chainLength;
    u8 rechargeSteps;
    struct RadarPatch patches[RADAR_MAX_PATCHES];
    u8 numPatches;
    u8 levelMin;
    u8 levelMax;
    u8 markerTaskId;
};

static EWRAM_DATA struct PokeRadarState sRadarState = {0};

static const u8 sText_NothingFound[] = _("No POKéMON were found nearby.\p");
static const u8 sText_NotHere[] = _("This can't be used here.\nStand in tall grass!\p");
static const u8 sText_Recharging[] = _("The POKé RADAR needs to recharge.\nWalk around to charge it.\p");
static const u8 sText_NoCatch[] = _("Catch a POKéMON first, then the\nRADAR can scan for that species.\p");
static const u8 sText_Nearby[] = _("Nearby ");
static const u8 sText_Detected[] = _(" detected!\nChain: ");
static const u8 sText_Scanning[] = _("Scanning…\p");

static bool8 IsTileGrass(s16 x, s16 y)
{
    if (x < 0 || y < 0 || x >= gMapHeader.mapLayout->width || y >= gMapHeader.mapLayout->height)
        return FALSE;
    return MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
}

static void FindGrassPatches(void)
{
    s16 px = gSaveBlock1Ptr->pos.x;
    s16 py = gSaveBlock1Ptr->pos.y;
    s16 candidates[64][2];
    int numCandidates = 0;

    sRadarState.numPatches = 0;

    for (s16 dy = -4; dy <= 4; dy++)
    {
        for (s16 dx = -4; dx <= 4; dx++)
        {
            if (dx == 0 && dy == 0) continue;
            if (dx * dx + dy * dy < 4) continue;
            if (IsTileGrass(px + dx, py + dy) && numCandidates < 64)
            {
                candidates[numCandidates][0] = px + dx;
                candidates[numCandidates][1] = py + dy;
                numCandidates++;
            }
        }
    }

    for (int i = 0; i < RADAR_MAX_PATCHES && numCandidates > 0; i++)
    {
        int idx = Random() % numCandidates;
        sRadarState.patches[i].x = candidates[idx][0];
        sRadarState.patches[i].y = candidates[idx][1];

        u32 shinyRoll = Random() % 8192;
        u32 shinyThreshold = 0;
        if (sRadarState.chainLength >= RADAR_CHAIN_MAX)
            shinyThreshold = 40;
        else if (sRadarState.chainLength >= 30)
            shinyThreshold = 20;
        else if (sRadarState.chainLength >= 20)
            shinyThreshold = 8;
        else if (sRadarState.chainLength >= 10)
            shinyThreshold = 2;
        {
            extern int ImGuiBridge_IsShinyCharm(void);
            extern int ImGuiBridge_IsAlwaysShiny(void);
            if (ImGuiBridge_IsAlwaysShiny())
                shinyThreshold = 8192;
            else if (ImGuiBridge_IsShinyCharm())
                shinyThreshold *= 3;
        }
        sRadarState.patches[i].isShiny = (shinyRoll < shinyThreshold);

        sRadarState.numPatches++;
        candidates[idx][0] = candidates[numCandidates - 1][0];
        candidates[idx][1] = candidates[numCandidates - 1][1];
        numCandidates--;
    }
}

// Persistent grass-shaking task using the exclamation mark icon field effect
// re-triggered periodically so the patches keep animating
static void Task_RadarMarkers(u8 taskId)
{
    if (!sRadarState.active || sRadarState.numPatches == 0)
    {
        DestroyTask(taskId);
        sRadarState.markerTaskId = TASK_NONE;
        return;
    }

    s16 *timer = &gTasks[taskId].data[0];
    (*timer)++;
    if (*timer >= 30)
    {
        *timer = 0;
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        for (int i = 0; i < sRadarState.numPatches; i++)
        {
            s16 mapX = sRadarState.patches[i].x + MAP_OFFSET;
            s16 mapY = sRadarState.patches[i].y + MAP_OFFSET;

            gFieldEffectArguments[0] = mapX;
            gFieldEffectArguments[1] = mapY;
            gFieldEffectArguments[2] = player->previousElevation;
            gFieldEffectArguments[3] = 2;
            gFieldEffectArguments[4] = (NET_LOCAL_ID_START + i) << 8 | player->mapNum;
            gFieldEffectArguments[5] = player->mapGroup;
            gFieldEffectArguments[6] = (u8)gSaveBlock1Ptr->location.mapNum << 8 | (u8)gSaveBlock1Ptr->location.mapGroup;
            gFieldEffectArguments[7] = 1;
            FieldEffectStart(FLDEFF_JUMP_TALL_GRASS);

            if (sRadarState.patches[i].isShiny)
            {
                gFieldEffectArguments[0] = mapX;
                gFieldEffectArguments[1] = mapY;
                gFieldEffectArguments[2] = 0;
                FieldEffectStart(FLDEFF_SPARKLE);
            }
        }
    }
}

#define NET_LOCAL_ID_START 240

static void StartMarkerTask(void)
{
    if (sRadarState.markerTaskId != TASK_NONE)
        return;
    u8 taskId = CreateTask(Task_RadarMarkers, 80);
    gTasks[taskId].data[0] = 0;
    sRadarState.markerTaskId = taskId;
}

static void StopMarkerTask(void)
{
    if (sRadarState.markerTaskId != TASK_NONE)
    {
        DestroyTask(sRadarState.markerTaskId);
        sRadarState.markerTaskId = TASK_NONE;
    }
}

static void BuildRadarResultMessage(void)
{
    u8 *dst = gStringVar4;
    dst = StringCopy(dst, sText_Nearby);
    GetSpeciesName(gStringVar1, sRadarState.chainSpecies);
    dst = StringCopy(dst, gStringVar1);
    dst = StringCopy(dst, sText_Detected);
    dst = ConvertIntToDecimalStringN(dst, sRadarState.chainLength, STR_CONV_MODE_LEFT_ALIGN, 3);
    dst[0] = EXT_CTRL_CODE_BEGIN;
    dst[1] = EXT_CTRL_CODE_PAUSE_UNTIL_PRESS;
    dst[2] = EOS;
}

static void GetAreaLevelRange(void)
{
    sRadarState.levelMin = 2;
    sRadarState.levelMax = 5;

    for (int h = 0; gWildMonHeaders[h].mapGroup != 0xFF; h++)
    {
        if (gWildMonHeaders[h].mapGroup == gSaveBlock1Ptr->location.mapGroup
         && gWildMonHeaders[h].mapNum == gSaveBlock1Ptr->location.mapNum
         && gWildMonHeaders[h].landMonsInfo != NULL)
        {
            const struct WildPokemon *mons = gWildMonHeaders[h].landMonsInfo->wildPokemon;
            sRadarState.levelMin = 255;
            sRadarState.levelMax = 0;
            for (int i = 0; i < LAND_WILD_COUNT; i++)
            {
                if (mons[i].minLevel < sRadarState.levelMin)
                    sRadarState.levelMin = mons[i].minLevel;
                if (mons[i].maxLevel > sRadarState.levelMax)
                    sRadarState.levelMax = mons[i].maxLevel;
            }
            break;
        }
    }
}

// --- Public API ---

u8 PokeRadar_GetChainLength(void) { return sRadarState.chainLength; }
u16 PokeRadar_GetChainSpecies(void) { return sRadarState.chainSpecies; }
bool8 PokeRadar_IsActive(void) { return sRadarState.active; }

void PokeRadar_BreakChain(void)
{
    StopMarkerTask();
    sRadarState.chainLength = 0;
    sRadarState.chainSpecies = SPECIES_NONE;
    sRadarState.active = FALSE;
    sRadarState.numPatches = 0;
    sRadarState.rechargeSteps = RADAR_RECHARGE_STEPS;
}

bool8 PokeRadar_TryStartEncounter(void)
{
    if (!sRadarState.active) return FALSE;

    s16 px = gSaveBlock1Ptr->pos.x;
    s16 py = gSaveBlock1Ptr->pos.y;

    for (int i = 0; i < sRadarState.numPatches; i++)
    {
        if (sRadarState.patches[i].x == px && sRadarState.patches[i].y == py)
        {
            u8 level = sRadarState.levelMin + (Random() % (sRadarState.levelMax - sRadarState.levelMin + 1));
            ZeroEnemyPartyMons();
            if (sRadarState.patches[i].isShiny)
            {
                u32 pokemon_personality;
                u32 pokemon_otId = gSaveBlock2Ptr->playerTrainerId[0]
                    | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
                    | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
                    | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
                do {
                    pokemon_personality = (Random() << 16) | Random();
                } while ((pokemon_personality ^ pokemon_otId) >= 8);
                CreateMon(&gEnemyParty[0], sRadarState.chainSpecies, level, USE_RANDOM_IVS, TRUE, pokemon_personality, FALSE, 0);
            }
            else
            {
                CreateMonWithNature(&gEnemyParty[0], sRadarState.chainSpecies, level, USE_RANDOM_IVS, Random() % NUM_NATURES);
            }

            StopMarkerTask();
            sRadarState.active = FALSE;
            sRadarState.numPatches = 0;
            sRadarState.rechargeSteps = RADAR_RECHARGE_STEPS;
            if (sRadarState.chainLength < 255)
                sRadarState.chainLength++;
            StartWildBattle();
            return TRUE;
        }
    }
    return FALSE;
}

void PokeRadar_Step(void)
{
    if (sRadarState.rechargeSteps > 0)
    {
        sRadarState.rechargeSteps--;
        if (sRadarState.rechargeSteps == 0)
            PlaySE(SE_POKENAV_ON);
    }
}

// --- Field Use (bag item) ---

static void Task_RadarCloseMessage(u8 taskId)
{
    ClearDialogWindowAndFrame(0, 1);
    DestroyTask(taskId);
    ClearPlayerHeldMovementAndUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

static void Task_RadarShowResults(u8 taskId)
{
    FindGrassPatches();
    if (sRadarState.numPatches == 0)
    {
        DisplayItemMessageOnField(taskId, FONT_NORMAL, sText_NothingFound, Task_RadarCloseMessage);
        return;
    }
    sRadarState.active = TRUE;
    GetAreaLevelRange();
    PlaySE(SE_PC_LOGIN);
    StartMarkerTask();
    BuildRadarResultMessage();
    DisplayItemMessageOnField(taskId, FONT_NORMAL, gStringVar4, Task_RadarCloseMessage);
}

void FieldUseFunc_PokeRadar(u8 taskId)
{
    s16 x = gSaveBlock1Ptr->pos.x + MAP_OFFSET;
    s16 y = gSaveBlock1Ptr->pos.y + MAP_OFFSET;
    if (!MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x, y)))
    {
        DisplayItemMessageOnField(taskId, FONT_NORMAL, sText_NotHere, Task_RadarCloseMessage);
        return;
    }
    if (sRadarState.rechargeSteps > 0)
    {
        DisplayItemMessageOnField(taskId, FONT_NORMAL, sText_Recharging, Task_RadarCloseMessage);
        return;
    }

    // Check if we caught something new to start/change the chain
    u16 lastCaught = gBattleResults.caughtMonSpecies;
    if (lastCaught != SPECIES_NONE && lastCaught != sRadarState.chainSpecies)
    {
        sRadarState.chainSpecies = lastCaught;
        sRadarState.chainLength = 1;
    }

    // Need a species to scan for
    if (sRadarState.chainSpecies == SPECIES_NONE)
    {
        DisplayItemMessageOnField(taskId, FONT_NORMAL, sText_NoCatch, Task_RadarCloseMessage);
        return;
    }

    PlaySE(SE_DEX_SEARCH);
    DisplayItemMessageOnField(taskId, FONT_NORMAL, sText_Scanning, Task_RadarShowResults);
}
