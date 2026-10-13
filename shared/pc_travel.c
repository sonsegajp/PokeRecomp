// PC port: travelling between regions (see port/include/pc_travel.h).
//
// A trainer picks another region's world in the ONLINE CLUB. Their game saves,
// writes travel.dat and exits; the runner starts the other region's game,
// which boots straight into the field as that same trainer (their name,
// gender, ID and party) on a save of its own, the "visitor save":
//   <home>_visit_<game>.sav next to the home save.
// A first visit starts the region's story in its first town. Coming back works
// the same way the other direction, and the party goes home with them.
#include "global.h"
#ifdef PLATFORM_PC
#ifdef EMERALD
#include "main.h"
#include "palette.h"
#include "sprite.h"
#include "constants/heal_locations.h"
#else
#include "gflib.h"
#include "quest_log.h"
#endif
#include "event_data.h"
#include "event_scripts.h"
#include "string_util.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "save.h"
#include "script.h"
#include "task.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/vars.h"
#include "pc_net.h"
#include "pc_port.h"
#include "pc_launcher.h"
#include "pc_travel.h"


STATIC_ASSERT(sizeof(struct Pokemon) == 100, PokemonIs100Bytes);

int gPcTravelArrive;

enum { ROLE_NONE, ROLE_HOME, ROLE_VISITOR };

static struct PcTravelFile sTravel;
static u8 sRole;
static bool8 sPending;      // the party still has to be handed over in the field
static bool8 sNewVisitor;   // this boot started the visitor's story
static bool8 sJoinTaken;

#if defined(EMERALD)
static const char sGameId[] = "emerald";
#elif defined(LEAFGREEN)
static const char sGameId[] = "leafgreen";
#else
static const char sGameId[] = "firered";
#endif

static const char *const sGameIds[] = {
    [PC_NET_GAME_FIRERED] = "firered",
    [PC_NET_GAME_LEAFGREEN] = "leafgreen",
    [PC_NET_GAME_EMERALD] = "emerald",
    [PC_NET_GAME_RUBY] = "ruby",
    [PC_NET_GAME_SAPPHIRE] = "sapphire",
};

// Next to the runner: games\<id>.dll and <id>.pkmn (the player's own ROM).
static bool8 GameInstalled(int game)
{
    if (game < 0 || game >= (int)ARRAY_COUNT(sGameIds) || !sGameIds[game])
        return FALSE;
    return PcTravelGameInstalled(sGameIds[game]);
}

// The game a trainer travels to in a region: Kanto is FireRed (or LeafGreen),
// Hoenn is Emerald.
static int RegionGame(int region)
{
    static const u8 kanto[] = {PC_NET_GAME_FIRERED, PC_NET_GAME_LEAFGREEN};
    static const u8 hoenn[] = {PC_NET_GAME_EMERALD, PC_NET_GAME_RUBY, PC_NET_GAME_SAPPHIRE};
    const u8 *games = region == PC_NET_REGION_HOENN ? hoenn : kanto;
    int i, n = region == PC_NET_REGION_HOENN ? ARRAY_COUNT(hoenn) : ARRAY_COUNT(kanto);

    for (i = 0; i < n; i++)
        if (GameInstalled(games[i]))
            return games[i];
    return -1;
}

static int GameRegion(int game)
{
    return game == PC_NET_GAME_FIRERED || game == PC_NET_GAME_LEAFGREEN ? PC_NET_REGION_KANTO : PC_NET_REGION_HOENN;
}

static void CopyText(char *dst, int size, const char *src)
{
    int i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

// Who this game is to the trainer in travel.dat, if anyone.
static void LoadTravel(void)
{
    sRole = ROLE_NONE;
    if (!PcTravelRead(&sTravel))
        return;
    sTravel.homeSave[sizeof sTravel.homeSave - 1] = sTravel.destSave[sizeof sTravel.destSave - 1] = 0;
    sTravel.server[sizeof sTravel.server - 1] = 0;
    if (sTravel.homeGame == PC_NET_GAME && PcTravelSamePath(sTravel.homeSave, PcSaveGetPath()))
        sRole = ROLE_HOME;
    else if (sTravel.destGame == PC_NET_GAME && sTravel.state == PC_TRAVEL_AWAY
          && PcTravelSamePath(sTravel.destSave, PcSaveGetPath()))
        sRole = ROLE_VISITOR;
}

int PcTravelVisitor(void)
{
    return sRole == ROLE_VISITOR ? sTravel.homeGame : -1;
}

// ---- boot ---------------------------------------------------------------------

// Called once the save has loaded at boot (intro.c). Arriving by travel goes
// straight to the field: the visitor's own save, or their story's start.
int PcTravelBoot(void)
{
    LoadTravel();
    if (sRole == ROLE_NONE)
        return FALSE;
    sPending = TRUE;   // the party is handed over once in the field
    PcNetSetGame(sRole == ROLE_VISITOR ? sTravel.homeGame : PC_NET_GAME);   // others see their own hero
    if (!gPcTravelArrive)
        return FALSE;  // the home game started normally: title screen as usual

    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPaletteFade();
    gMain.state = 0;
    if (sRole == ROLE_VISITOR && gSaveFileStatus != SAVE_STATUS_OK)
    {
        // a first visit: this trainer, new to the region
        memcpy(gSaveBlock2Ptr->playerName, sTravel.name, PLAYER_NAME_LENGTH + 1);
        gSaveBlock2Ptr->playerGender = sTravel.gender;
#ifndef EMERALD
        // Oak's intro would have named the rival: the game's own first choice
#ifdef LEAFGREEN
        StringCopy(gSaveBlock1Ptr->rivalName, gNameChoice_Red);
#else
        StringCopy(gSaveBlock1Ptr->rivalName, gNameChoice_Green);
#endif
#endif
        sNewVisitor = TRUE;
        SetMainCallback2(CB2_NewGame);
    }
    else
    {
        SetMainCallback2(CB2_ContinueSavedGame);
    }
    return TRUE;
}

// CB2_NewGame, after NewGameInitData: a visitor skips the intro and starts
// out in the region's first town, the story ready to go from there.
int PcTravelNewGame(void)
{
    if (!sNewVisitor)
        return FALSE;
    memcpy(gSaveBlock2Ptr->playerTrainerId, sTravel.otId, 4);   // the same trainer: their Pokemon obey
#ifdef EMERALD
    // as if they had stepped off the moving truck and set the clock upstairs
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    FlagSet(FLAG_SET_WALL_CLOCK);
    if (sTravel.gender == MALE)
    {
        SetLastHealLocationWarp(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL);
        VarSet(VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 1);
        SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), WARP_ID_NONE, 5, 9);
    }
    else
    {
        SetLastHealLocationWarp(HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING);
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL);
        VarSet(VAR_LITTLEROOT_HOUSES_STATE_MAY, 1);
        SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), WARP_ID_NONE, 14, 9);
    }
#else
    // in front of their house in Pallet Town
    SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN), WARP_ID_NONE, 6, 8);
#endif
    WarpIntoMap();
    return TRUE;
}

// ---- in the field ----------------------------------------------------------------

static void TakeParty(void)
{
    int i;

    memcpy(gPlayerParty, sTravel.party, sizeof(struct Pokemon) * PARTY_SIZE);
    CalculatePlayerPartyCount();
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL);
        if (species != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG, NULL))
        {
            u16 dex = SpeciesToNationalPokedexNum(species);
            GetSetPokedexFlag(dex, FLAG_SET_SEEN);
            GetSetPokedexFlag(dex, FLAG_SET_CAUGHT);
        }
    }
}

// The first free moment in the field after starting: the travelling party
// arrives (or comes home), then it's saved where it is now.
void PcTravelFieldUpdate(void)
{
    if (!sPending || !PcFieldIsOnScreen() || ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled()
     || gPaletteFade.active)
        return;
#ifndef EMERALD
    if (gQuestLogState != 0)
        return;
#endif
    sPending = FALSE;
    TakeParty();
    if (sRole == ROLE_HOME)
    {
        // home again: the party is back in the home save, and the trip is over
        TrySavingData(SAVE_NORMAL);
        PcTravelDelete();
        sRole = ROLE_NONE;
        PcNetSetGame(PC_NET_GAME);
    }
    else
    {
        TrySavingData(SAVE_NORMAL);
    }
}

const char *PcTravelTakeJoin(void)
{
    if (sJoinTaken || !gPcTravelArrive || sPending || !sTravel.server[0])
        return NULL;
    sJoinTaken = TRUE;
    return sTravel.server;
}

// A visitor saving keeps travel.dat's party current: that's where it lives
// until they go home.
void PcTravelOnSave(void)
{
    if (sRole != ROLE_VISITOR || sPending)
        return;
    memcpy(sTravel.party, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    sTravel.partyCount = gPlayerPartyCount;
    PcTravelWrite(&sTravel);
}

// ---- leaving --------------------------------------------------------------------

// The ONLINE CLUB, a world in the other region: save, write the trip and quit;
// the runner starts the other game. A visitor picking their home region's
// world goes home. Returns FALSE if the other region's game isn't installed.
int PcTravelDepart(int serverIndex)
{
    int region = PcNetServerRegion(serverIndex);

    if (sRole == ROLE_VISITOR && region == GameRegion(sTravel.homeGame))
    {
        sTravel.state = PC_TRAVEL_RETURNING;
    }
    else
    {
        int dest = RegionGame(region);
        if (dest < 0 || region == PC_NET_REGION)
            return FALSE;
        memset(&sTravel, 0, sizeof sTravel);
        memcpy(sTravel.magic, PC_TRAVEL_MAGIC, 8);
        sTravel.state = PC_TRAVEL_AWAY;
        sTravel.homeGame = PC_NET_GAME;
        sTravel.destGame = dest;
        sTravel.gender = gSaveBlock2Ptr->playerGender;
        CopyText(sTravel.homeId, sizeof sTravel.homeId, sGameId);
        CopyText(sTravel.destId, sizeof sTravel.destId, sGameIds[dest]);
        PcTravelFullPath(PcSaveGetPath(), sTravel.homeSave, sizeof sTravel.homeSave);
        // the visitor save, beside the home save in the data directory: <home>_visit_<game>.sav
        PcTravelFullPath(".", sTravel.destSave, sizeof sTravel.destSave - 40);
        strcat(sTravel.destSave, "\\");
        strcat(sTravel.destSave, sGameId);
        strcat(sTravel.destSave, "_visit_");
        strcat(sTravel.destSave, sGameIds[dest]);
        strcat(sTravel.destSave, ".sav");
        memcpy(sTravel.name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH + 1);
        memcpy(sTravel.otId, gSaveBlock2Ptr->playerTrainerId, 4);
    }
    CopyText(sTravel.server, sizeof sTravel.server, PcNetServerName(serverIndex));
    memcpy(sTravel.party, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    sTravel.partyCount = gPlayerPartyCount;
    if (sRole == ROLE_VISITOR)
        sRole = ROLE_NONE;   // the trip below is written as it stands, not overwritten on save
    TrySavingData(SAVE_NORMAL);
    if (!PcTravelWrite(&sTravel))
        return FALSE;
    PcNetDisconnect();
    PcLauncherFinish(PC_EXIT_TRAVEL, NULL);
    return TRUE;
}
#endif
