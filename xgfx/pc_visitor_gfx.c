// PC port: the other region's player sprites (xgfx/pc_visitor_gfx.h). Built
// into each game from the other decomp's graphics: Emerald's Brendan and May
// into FireRed and LeafGreen, FireRed's Red and Leaf into Emerald.
#include "global.h"
#ifdef PLATFORM_PC
#ifdef EMERALD
#include "sprite.h"
#else
#include "gflib.h"
#endif
#include "constants/event_object_movement.h"
#include "pc_visitor_gfx.h"

#ifdef EMERALD
// FireRed's animations that Emerald has no use for
#ifndef ANIM_SPIN_SOUTH
#define ANIM_SPIN_SOUTH (ANIM_STD_COUNT + 4)
#define ANIM_SPIN_NORTH (ANIM_STD_COUNT + 5)
#define ANIM_SPIN_WEST  (ANIM_STD_COUNT + 6)
#define ANIM_SPIN_EAST  (ANIM_STD_COUNT + 7)
#endif
#ifndef ANIM_RAISE_HAND
#define ANIM_RAISE_HAND (ANIM_STD_COUNT + 0)
#endif
#ifndef ANIM_SHAKE_HEAD_OR_WALK_IN_PLACE
#define ANIM_SHAKE_HEAD_OR_WALK_IN_PLACE (ANIM_STD_COUNT + 8)
#endif
#include "visitor_firered.h"
#else
#include "visitor_emerald.h"
#endif
#endif
