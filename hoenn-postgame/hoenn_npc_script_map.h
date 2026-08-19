// Auto-generated NPC script mapping
// Maps (dirname, npc_index) -> script label

// AbandonedShip_Corridors_B1F NPC 0: AbandonedShip_Corridors_B1F_EventScript_TuberM
extern const u8 HoennScript_AbandonedShip_Corridors_B1F_0[];
// AbandonedShip_Corridors_B1F NPC 1: AbandonedShip_Corridors_B1F_EventScript_Duncan
extern const u8 HoennScript_AbandonedShip_Corridors_B1F_1[];

// Script assignment table
typedef struct { const char *map; u8 npcIdx; const u8 *script; } HoennNPCScriptEntry;
static const HoennNPCScriptEntry sHoennNPCScripts[] = {
    {"AbandonedShip_Corridors_B1F", 0, HoennScript_AbandonedShip_Corridors_B1F_0},
    {"AbandonedShip_Corridors_B1F", 1, HoennScript_AbandonedShip_Corridors_B1F_1},
    {NULL, 0, NULL},
};
