#ifndef GUARD_PC_ONLINE_H
#define GUARD_PC_ONLINE_H

// PC port: online play (src/pc_online.c, data/scripts/pc_online.inc).

void Online_OverworldUpdate(void);
const u8 *Online_GetAvatarScript(u8 localId);
bool8 Online_OutbreakMon(u16 *species, u8 *level);
u8 Online_ShinyRerolls(u16 species);
void PcOnlineOnSave(void);
struct Pokemon;
void Online_ShinyEvent(u8 battler, struct Pokemon *mon, bool8 caught);

extern const u8 Online_EventScript_TalkToPlayer[];
extern const u8 Online_EventScript_Guide[];
extern const u8 Online_EventScript_IncomingRequest[];
extern const u8 Online_EventScript_DebugRequest[];
extern const u8 Online_EventScript_DebugAccept[];
extern const u8 Online_EventScript_JoinedMessage[];
extern const u8 Online_EventScript_LostServer[];

#endif // GUARD_PC_ONLINE_H
