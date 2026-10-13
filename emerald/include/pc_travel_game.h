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
#else
#define PC_PLAYER_GFX(gender, gfx)        (gfx)
#define PC_GFX_INFO(gfx, info)            (info)
#define PC_STEP_ANIMS(anims)              (anims)
#define PC_PLAYER_BACK_PIC(gender, pic)   (pic)
#define PC_GAME_VERSION(version)          (version)
#endif

#endif // GUARD_PC_TRAVEL_GAME_H
