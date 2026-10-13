// PC port: names that differ between the games our shared code (shared/*.c)
// is built into. Include after the game's own headers.
#ifndef GUARD_PC_GAME_H
#define GUARD_PC_GAME_H

#ifdef EMERALD
#define JOY_REPT JOY_REPEAT
// bag pockets: Emerald calls the TM case and berry pouch pockets these
#define POCKET_TM_CASE     POCKET_TM_HM
#define POCKET_BERRY_POUCH POCKET_BERRIES
// pokeemerald renamed the item getters
#define ItemId_GetPocket     GetItemPocket
#define ItemId_GetImportance GetItemImportance
#define ItemId_GetPrice      GetItemPrice
#define ItemId_GetName       GetItemName
#endif

// save flag: NURSE JOY has met this trainer on a Pokemon Center 2F (a flag
// both games leave unused)
#if defined(EMERALD)
#define PC_FLAG_ONLINE_GUIDE FLAG_UNUSED_0x020
#else
#define PC_FLAG_ONLINE_GUIDE FLAG_UNUSED_0x4A7
#endif

// save flag: this trainer has joined a server before (the "other trainers are
// real players" explanation is only for the first time)
#if defined(EMERALD)
#define PC_FLAG_ONLINE_JOINED FLAG_UNUSED_0x021
#else
#define PC_FLAG_ONLINE_JOINED FLAG_UNUSED_0x4A8
#endif

#ifndef LAND_WILD_COUNT
#define LAND_WILD_COUNT 12   // every Gen 3 game: 12 grass slots
#endif

#endif // GUARD_PC_GAME_H
