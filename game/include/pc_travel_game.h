// PC port: a trainer visiting from another region looks like their own
// region's hero (shared/pc_travel.c). Outside the PC build these are the
// game's own choices.
#ifndef GUARD_PC_TRAVEL_GAME_H
#define GUARD_PC_TRAVEL_GAME_H

#ifdef PLATFORM_PC
#include "pc_travel.h"
#define PC_PLAYER_GFX(gender, gfx)        PcTravelPlayerGfx(gender, gfx)
#define PC_GFX_INFO(gfx, info)            ((const struct ObjectEventGraphicsInfo *)PcTravelGraphicsInfo(gfx, info))
#define PC_STEP_ANIMS(anims)              ((const union AnimCmd *const *)PcTravelBaseAnims(anims))
#define PC_PLAYER_BACK_PIC(gender, pic)   PcTravelBackPic(gender, pic)
#define PC_GAME_VERSION(version)          PcTravelGameVersion(version)
#define PC_CARD_PIC(gender, pic)          PcTravelCardPic(gender, pic)
#else
#define PC_PLAYER_GFX(gender, gfx)        (gfx)
#define PC_GFX_INFO(gfx, info)            (info)
#define PC_STEP_ANIMS(anims)              (anims)
#define PC_PLAYER_BACK_PIC(gender, pic)   (pic)
#define PC_GAME_VERSION(version)          (version)
#define PC_CARD_PIC(gender, pic)          (pic)
#endif

// Other online Kanto trainers while the player is a visitor (whose sprites use
// Red and Green's ids): the game's own Red / Green, under ids of their own.
#define OBJ_EVENT_GFX_PC_KANTO_RED   NUM_OBJ_EVENT_GFX
#define OBJ_EVENT_GFX_PC_KANTO_GREEN (NUM_OBJ_EVENT_GFX + 1)
// Other online trainers from Emerald: its own Brendan / May (the ported
// sprites, xgfx/), or Ruby/Sapphire's without them.
#define OBJ_EVENT_GFX_PC_HOENN_BRENDAN (NUM_OBJ_EVENT_GFX + 2)
#define OBJ_EVENT_GFX_PC_HOENN_MAY     (NUM_OBJ_EVENT_GFX + 3)

#endif // GUARD_PC_TRAVEL_GAME_H
