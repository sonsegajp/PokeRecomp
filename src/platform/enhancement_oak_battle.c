#include "global.h"
#include "constants/flags.h"

#ifdef PORTABLE
#include "event_data.h"
#include "item.h"
#include "pokedex.h"
#include "constants/items.h"
#include <stdio.h>

extern int ImGuiBridge_IsShinyCharm(void);
extern bool16 HasAllKantoMons(void);

bool16 Special_ShouldOakBattle(void)
{
    if (!ImGuiBridge_IsShinyCharm())
        return FALSE;
    if (FlagGet(FLAG_OAK_BATTLE_DONE))
        return FALSE;
    if (!HasAllKantoMons())
        return FALSE;
    return TRUE;
}

void Special_OakBattleWon(void)
{
    FlagSet(FLAG_OAK_BATTLE_DONE);
    AddBagItem(ITEM_SHINY_CHARM, 1);
    printf("[OAK] Player defeated Prof. Oak! Shiny Charm obtained.\n");
    fflush(stdout);
}

#else

bool16 Special_ShouldOakBattle(void)
{
    return FALSE;
}

void Special_OakBattleWon(void)
{
}

#endif
