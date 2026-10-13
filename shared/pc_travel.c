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
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/trainers.h"
#include "event_object_movement.h"
#include "pc_net.h"
#include "pc_port.h"
#include "pc_launcher.h"
#include "pc_travel.h"
#include "pc_rompack.h"
#include "../xgfx/pc_visitor_gfx.h"
#include "pc_travel_game.h"


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
    if (gPlayerPartyCount > 0)
        FlagSet(FLAG_SYS_POKEMON_GET);   // they came with Pokemon: the menu has POKEMON
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

// ---- a visitor's look -------------------------------------------------------------

// A visitor is drawn with their own game's player sprites in every state:
// Emerald's Brendan and May in FireRed/LeafGreen, FireRed's Red and Leaf in
// Emerald (xgfx/, from the player's copy of that game). They take the
// player's own sprite ids, so everything that shows the player shows them.
// Each game's animation numbers are asked for by this game's code; any the
// other game's table doesn't have become the nearest walking one.
#define VISITOR_ANIMS 48   // past every animation the player's states use
#define OBJ_EVENT_PAL_TAG_NONE 0x11FF   // as in event_object_movement.c

static const u8 sPlayerGfxStates[][3] = {   // this game's player ids: gender, PC_VISITOR_*
#ifdef EMERALD
    {OBJ_EVENT_GFX_BRENDAN_NORMAL,     MALE,   PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_BRENDAN_MACH_BIKE,  MALE,   PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE,  MALE,   PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_BRENDAN_SURFING,    MALE,   PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_BRENDAN_UNDERWATER, MALE,   PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE, MALE,   PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_BRENDAN_FISHING,    MALE,   PC_VISITOR_FISH},
    {OBJ_EVENT_GFX_BRENDAN_WATERING,   MALE,   PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_BRENDAN_DECORATING, MALE,   PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_MAY_NORMAL,         FEMALE, PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_MAY_MACH_BIKE,      FEMALE, PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_MAY_ACRO_BIKE,      FEMALE, PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_MAY_SURFING,        FEMALE, PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_MAY_UNDERWATER,     FEMALE, PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_MAY_FIELD_MOVE,     FEMALE, PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_MAY_FISHING,        FEMALE, PC_VISITOR_FISH},
    {OBJ_EVENT_GFX_MAY_WATERING,       FEMALE, PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_MAY_DECORATING,     FEMALE, PC_VISITOR_NORMAL},
#else
    {OBJ_EVENT_GFX_RED_NORMAL,         MALE,   PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_RED_BIKE,           MALE,   PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_RED_SURF,           MALE,   PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_RED_FIELD_MOVE,     MALE,   PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_RED_FISH,           MALE,   PC_VISITOR_FISH},
    {OBJ_EVENT_GFX_RED_VS_SEEKER,      MALE,   PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_RED_VS_SEEKER_BIKE, MALE,   PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_GREEN_NORMAL,       FEMALE, PC_VISITOR_NORMAL},
    {OBJ_EVENT_GFX_GREEN_BIKE,         FEMALE, PC_VISITOR_BIKE},
    {OBJ_EVENT_GFX_GREEN_SURF,         FEMALE, PC_VISITOR_SURF},
    {OBJ_EVENT_GFX_GREEN_FIELD_MOVE,   FEMALE, PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_GREEN_FISH,         FEMALE, PC_VISITOR_FISH},
    {OBJ_EVENT_GFX_GREEN_VS_SEEKER,    FEMALE, PC_VISITOR_FIELD_MOVE},
    {OBJ_EVENT_GFX_GREEN_VS_SEEKER_BIKE, FEMALE, PC_VISITOR_BIKE},
#endif
};

#ifdef EMERALD
#define HOST_NORMAL_GFX OBJ_EVENT_GFX_BRENDAN_NORMAL      // its 16x32 player sprite
#define HOST_BIG_GFX    OBJ_EVENT_GFX_BRENDAN_MACH_BIKE   // its 32x32 one
#else
#define HOST_NORMAL_GFX OBJ_EVENT_GFX_RED_NORMAL
#define HOST_BIG_GFX    OBJ_EVENT_GFX_RED_BIKE
#endif

extern const struct ObjectEventGraphicsInfo *const gObjectEventGraphicsInfoPointers[];

static struct ObjectEventGraphicsInfo sVisitorInfo[2][PC_VISITOR_STATE_COUNT];
static const union AnimCmd *sVisitorAnims[2][PC_VISITOR_STATE_COUNT][VISITOR_ANIMS];
static const union AnimCmd *const *sVisitorStepAnims[2][PC_VISITOR_STATE_COUNT];
static bool8 sVisitorReady[2][PC_VISITOR_STATE_COUNT];

int PcTravelPlayerGfx(int gender, int gfx)
{
    return gfx;   // the same ids: they're drawn as the visitor (PcTravelGraphicsInfo)
}

static void BuildVisitorInfo(int gender, int state, const struct ObjectEventGraphicsInfo *own)
{
    const struct PcVisitorState *v = &gPcVisitorStates[gender][state];
    const struct ObjectEventGraphicsInfo *shape = gObjectEventGraphicsInfoPointers[v->width == 16 ? HOST_NORMAL_GFX : HOST_BIG_GFX];
    struct ObjectEventGraphicsInfo *info = &sVisitorInfo[gender][state];
    const union AnimCmd **anims = sVisitorAnims[gender][state];
    int i;

    *info = *own;                       // how this game treats the state (reflections, tracks, ...)
    info->size = v->size;
    info->width = v->width;
    info->height = v->height;
    info->oam = shape->oam;             // sprite shapes for that size
    info->subspriteTables = shape->subspriteTables;
    info->images = v->images;
    info->paletteTag = PC_VISITOR_PAL_TAG(gender);
    if (info->reflectionPaletteTag != OBJ_EVENT_PAL_TAG_NONE)
        info->reflectionPaletteTag = PC_VISITOR_REFLECTION_PAL_TAG(gender);
    for (i = 0; i < VISITOR_ANIMS; i++)
    {
        int dir = (i - ANIM_STD_COUNT) % 4;   // the extra animations come in S, N, W, E sets
        // the bike sheets use the plain table: nothing past the standard set
        bool8 has = i < v->animCount && v->anims[i] != NULL && (state != PC_VISITOR_BIKE || i < ANIM_STD_COUNT);
        if (has)
            anims[i] = v->anims[i];
        else if (i < ANIM_STD_COUNT)
            anims[i] = v->anims[ANIM_STD_FACE_SOUTH + (i & 3)];
        else if (i < ANIM_STD_COUNT + 4)
            anims[i] = v->anims[ANIM_STD_GO_FAST_SOUTH + dir];
        else
            anims[i] = v->anims[ANIM_STD_FACE_SOUTH + dir];
    }
    info->anims = anims;
    sVisitorStepAnims[gender][state] = shape->anims;   // walk-step positions are the same in every set
    sVisitorReady[gender][state] = TRUE;
}

// Other online trainers from the other region, walking around: the same sprites.
#ifdef EMERALD
#define REMOTE_GFX_MALE   OBJ_EVENT_GFX_RED
#define REMOTE_GFX_FEMALE OBJ_EVENT_GFX_LEAF
#else
#define REMOTE_GFX_MALE   OBJ_EVENT_GFX_PC_HOENN_BRENDAN
#define REMOTE_GFX_FEMALE OBJ_EVENT_GFX_PC_HOENN_MAY
#endif

const void *PcTravelGraphicsInfo(int gfx, const void *info)
{
    int i;

    if (!PcRomPackForeignReady())
        return info;
    if (gfx == REMOTE_GFX_MALE || gfx == REMOTE_GFX_FEMALE)
    {
        int gender = gfx == REMOTE_GFX_MALE ? MALE : FEMALE;
        if (!sVisitorReady[gender][PC_VISITOR_NORMAL])
            BuildVisitorInfo(gender, PC_VISITOR_NORMAL, gObjectEventGraphicsInfoPointers[HOST_NORMAL_GFX]);
        return &sVisitorInfo[gender][PC_VISITOR_NORMAL];
    }
    if (sRole != ROLE_VISITOR)
        return info;
    for (i = 0; i < (int)ARRAY_COUNT(sPlayerGfxStates); i++)
    {
        if (sPlayerGfxStates[i][0] == gfx)
        {
            int gender = sPlayerGfxStates[i][1], state = sPlayerGfxStates[i][2];
            if (!sVisitorReady[gender][state])
                BuildVisitorInfo(gender, state, info);
            return &sVisitorInfo[gender][state];
        }
    }
    return info;
}

const void *PcTravelBaseAnims(const void *anims)
{
    int g, s;
    for (g = 0; g < 2; g++)
        for (s = 0; s < PC_VISITOR_STATE_COUNT; s++)
            if (sVisitorReady[g][s] && anims == (const void *)sVisitorAnims[g][s])
                return sVisitorStepAnims[g][s];
    return anims;
}

int PcTravelBackPic(int gender, int pic)
{
    if (sRole != ROLE_VISITOR)
        return pic;
#ifdef EMERALD
    return TRAINER_BACK_PIC_RED + gender;              // Emerald carries FireRed's
#else
    if (PcRomPackForeignReady())
        return TRAINER_BACK_PIC_PC_VISITOR(gender);    // Emerald's Brendan / May (xgfx)
    return TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN + gender;
#endif
}

int PcTravelCardPic(int gender, int pic)
{
#ifndef EMERALD
    if (sRole == ROLE_VISITOR && PcRomPackForeignReady())
        return TRAINER_PIC_PC_VISITOR(gender);
#endif
    return pic;
}

int PcTravelGameVersion(int version)
{
    static const u8 versions[] = {
        [PC_NET_GAME_FIRERED] = VERSION_FIRE_RED,
        [PC_NET_GAME_LEAFGREEN] = VERSION_LEAF_GREEN,
        [PC_NET_GAME_EMERALD] = VERSION_EMERALD,
        [PC_NET_GAME_RUBY] = VERSION_RUBY,
        [PC_NET_GAME_SAPPHIRE] = VERSION_SAPPHIRE,
    };
    if (sRole != ROLE_VISITOR || sTravel.homeGame >= ARRAY_COUNT(versions))
        return version;
    return versions[sTravel.homeGame];
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
