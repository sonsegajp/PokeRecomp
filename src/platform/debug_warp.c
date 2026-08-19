#ifdef PORTABLE
#include "global.h"
#include "overworld.h"
#include "field_player_avatar.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "event_data.h"
#include "item.h"
#include "pokedex.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/flags.h"
#include <stdio.h>

void Debug_WarpToMap(int group, int map)
{
    printf("[DEBUG] Warping to group=%d map=%d\n", group, map);
    fflush(stdout);

    extern void SetWarpDestination(s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
    SetWarpDestination(group, map, -1, 5, 5);

    extern void WarpIntoMap(void);
    WarpIntoMap();

    extern void SetMainCallback1(MainCallback cb);
    extern void CB1_Overworld(void);
    extern void CB2_LoadMap(void);
    SetMainCallback1(CB1_Overworld);
    SetMainCallback2(CB2_LoadMap);
}

void Debug_SetPartyPidgeot(void)
{
    printf("[DEBUG] Setting party to Pidgeot with Fly\n");
    fflush(stdout);

    ZeroPlayerPartyMons();
    gPlayerPartyCount = 1;

    CreateMon(&gPlayerParty[0], SPECIES_PIDGEOT, 75, 31, FALSE, 0, OT_ID_PLAYER_ID, 0);

    u16 move;
    move = MOVE_FLY;        SetMonData(&gPlayerParty[0], MON_DATA_MOVE1, &move);
    move = MOVE_AERIAL_ACE; SetMonData(&gPlayerParty[0], MON_DATA_MOVE2, &move);
    move = MOVE_RETURN;     SetMonData(&gPlayerParty[0], MON_DATA_MOVE3, &move);
    move = MOVE_STEEL_WING; SetMonData(&gPlayerParty[0], MON_DATA_MOVE4, &move);

    u8 pp;
    pp = 15; SetMonData(&gPlayerParty[0], MON_DATA_PP1, &pp);
    pp = 20; SetMonData(&gPlayerParty[0], MON_DATA_PP2, &pp);
    pp = 20; SetMonData(&gPlayerParty[0], MON_DATA_PP3, &pp);
    pp = 25; SetMonData(&gPlayerParty[0], MON_DATA_PP4, &pp);

    CalculateMonStats(&gPlayerParty[0]);
}

void Debug_FillPCBoxes(void)
{
    printf("[DEBUG] Filling PC boxes with all 386 Pokemon\n"); fflush(stdout);
    for (int sp = 1; sp <= SPECIES_DEOXYS; sp++)
    {
        int box = (sp - 1) / IN_BOX_COUNT;
        int slot = (sp - 1) % IN_BOX_COUNT;
        if (box < TOTAL_BOXES_COUNT)
            CreateBoxMon(&gPokemonStoragePtr->boxes[box][slot], sp, 50, 31, FALSE, 0, OT_ID_PLAYER_ID, 0);
    }
}

void Debug_GiveAllHMs(void)
{
    printf("[DEBUG] Giving all HMs\n"); fflush(stdout);
    AddBagItem(ITEM_HM01, 1);
    AddBagItem(ITEM_HM02, 1);
    AddBagItem(ITEM_HM03, 1);
    AddBagItem(ITEM_HM04, 1);
    AddBagItem(ITEM_HM05, 1);
    AddBagItem(ITEM_HM06, 1);
    AddBagItem(ITEM_HM07, 1);
}

void Debug_CompletePokedex(void)
{
    printf("[DEBUG] Completing Pokedex\n"); fflush(stdout);
    for (int sp = 1; sp <= SPECIES_DEOXYS; sp++)
    {
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_CAUGHT);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_SEEN);
    }
    // Enable national dex
    FlagSet(FLAG_SYS_NATIONAL_DEX);
    FlagSet(FLAG_SYS_POKEDEX_GET);
}

void Debug_AllBadgesE4(void)
{
    printf("[DEBUG] Setting all badges + E4 clear\n"); fflush(stdout);
    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_BADGE03_GET);
    FlagSet(FLAG_BADGE04_GET);
    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_BADGE06_GET);
    FlagSet(FLAG_BADGE07_GET);
    FlagSet(FLAG_BADGE08_GET);
    FlagSet(FLAG_SYS_GAME_CLEAR);
}

#endif
