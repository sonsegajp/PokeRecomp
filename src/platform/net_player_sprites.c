#include "global.h"
#include "gflib.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/field_effects.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "overworld.h"
#include "sprite.h"
#include "menu.h"
#include "platform/net_client.h"
#include "../../server/protocol.h"
#include <stdio.h>

void NetClient_SetPlayerGender(u8 gender)
{
    gSaveBlock2Ptr->playerGender = gender;
}

void NetPlayerSprites_ClearBeforeSave(void)
{
    extern struct ObjectEvent gObjectEvents[];
    for (int i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (gObjectEvents[i].active && gObjectEvents[i].localId >= 240)
        {
            gObjectEvents[i].active = FALSE;
        }
    }
}

#define NET_LOCAL_ID_START 240
#define MAX_NET_SPRITES 8

static const u8 sNetPlayerGraphics[] = {
    OBJ_EVENT_GFX_RED_NORMAL,       // 0: Red
    OBJ_EVENT_GFX_GREEN_NORMAL,     // 1: Leaf
    OBJ_EVENT_GFX_RS_BRENDAN,       // 2: Brendan
    OBJ_EVENT_GFX_RS_MAY,           // 3: May
};

struct NetSprite {
    int active;
    int spawned;
    u8 playerId;
    u8 objectEventId;
    s16 currentX;
    s16 currentY;
    s16 targetX;
    s16 targetY;
    u8 direction;
    u8 graphicsId;
    u8 isRunning;
    u8 isBusy;
    u8 wasBusy;
    u8 busyBubbleTimer;
};

static struct NetSprite sNetSprites[MAX_NET_SPRITES];
static bool8 sInitialized = FALSE;

void NetInteraction_CheckIncomingRequests(void);

int NetPlayerSprites_GetNametagCount(void)
{
    int count = 1; // local player always included
    for (int i = 0; i < MAX_NET_SPRITES; i++)
        if (sNetSprites[i].active && sNetSprites[i].spawned)
            count++;
    return count;
}

int NetPlayerSprites_GetNametag(int index, int *screenX, int *screenY, char *nameOut, int nameMax)
{
    if (index == 0)
    {
        u8 sprId = gPlayerAvatar.spriteId;
        if (sprId < MAX_SPRITES && gSprites[sprId].inUse)
        {
            *screenX = gSprites[sprId].x + gSprites[sprId].x2;
            *screenY = gSprites[sprId].y + gSprites[sprId].y2 + gSprites[sprId].centerToCornerVecY - 8;
        }
        else
        {
            *screenX = -100;
            *screenY = -100;
        }
        extern const char *NetClient_GetMyUsername(void);
        const char *myName = NetClient_GetMyUsername();
        if (myName)
            strncpy(nameOut, myName, nameMax - 1);
        else
            nameOut[0] = '\0';
        nameOut[nameMax - 1] = '\0';
        return 1;
    }

    int found = 0;
    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        struct NetSprite *ns = &sNetSprites[i];
        if (!ns->active || !ns->spawned)
            continue;
        if (found + 1 == index)
        {
            if (ns->objectEventId < OBJECT_EVENTS_COUNT
                && gObjectEvents[ns->objectEventId].active)
            {
                u8 sprId = gObjectEvents[ns->objectEventId].spriteId;
                *screenX = gSprites[sprId].x + gSprites[sprId].x2;
                *screenY = gSprites[sprId].y + gSprites[sprId].y2 + gSprites[sprId].centerToCornerVecY - 8;
            }
            else
            {
                *screenX = -100;
                *screenY = -100;
            }
            struct RemotePlayer *players = NetClient_GetRemotePlayers();
            int pid = ns->playerId;
            if (pid >= 0 && pid < MAX_PLAYERS && players[pid].active)
                strncpy(nameOut, players[pid].username, nameMax - 1);
            else
                nameOut[0] = '\0';
            nameOut[nameMax - 1] = '\0';
            return 1;
        }
        found++;
    }
    return 0;
}

static void InitNetSprites(void)
{
    memset(sNetSprites, 0, sizeof(sNetSprites));
    sInitialized = TRUE;
}

static int FindNetSpriteByPlayerId(u8 playerId)
{
    for (int i = 0; i < MAX_NET_SPRITES; i++)
        if (sNetSprites[i].active && sNetSprites[i].playerId == playerId)
            return i;
    return -1;
}

static int FindFreeNetSprite(void)
{
    for (int i = 0; i < MAX_NET_SPRITES; i++)
        if (!sNetSprites[i].active)
            return i;
    return -1;
}

static u8 GetGraphicsForSprite(u8 trainerSprite)
{
    if (trainerSprite < sizeof(sNetPlayerGraphics))
        return sNetPlayerGraphics[trainerSprite];
    return OBJ_EVENT_GFX_COOLTRAINER_M;
}

static void SpawnNetSprite(int slot)
{
    struct NetSprite *ns = &sNetSprites[slot];

    u8 localId = NET_LOCAL_ID_START + slot;
    int objId = SpawnSpecialObjectEventParameterized(
        ns->graphicsId,
        MOVEMENT_TYPE_NONE,
        localId,
        ns->targetX,
        ns->targetY,
        0
    );

    if (objId < OBJECT_EVENTS_COUNT)
    {
        ns->objectEventId = objId;
        ns->spawned = 1;
        ns->currentX = ns->targetX;
        ns->currentY = ns->targetY;
        ObjectEventTurn(&gObjectEvents[objId], ns->direction);
    }
}

static void DespawnNetSprite(int slot)
{
    struct NetSprite *ns = &sNetSprites[slot];
    if (ns->spawned)
    {
        RemoveObjectEventByLocalIdAndMap(
            NET_LOCAL_ID_START + slot,
            gSaveBlock1Ptr->location.mapNum,
            gSaveBlock1Ptr->location.mapGroup
        );
        ns->spawned = 0;
    }
    ns->active = 0;
}

void NetPlayerSprites_Update(void)
{
    if (!sInitialized) InitNetSprites();
    if (!NetClient_IsConnected()) return;

    struct RemotePlayer *players = NetClient_GetRemotePlayers();
    u16 myMapGroup = gSaveBlock1Ptr->location.mapGroup;
    u16 myMapNum = gSaveBlock1Ptr->location.mapNum;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!players[i].active) continue;

        int sprIdx = FindNetSpriteByPlayerId(i);
        bool8 onSameMap = (players[i].mapGroup == myMapGroup && players[i].mapNum == myMapNum);

        if (onSameMap && sprIdx < 0)
        {
            int slot = FindFreeNetSprite();
            if (slot >= 0)
            {
                sNetSprites[slot].active = 1;
                sNetSprites[slot].spawned = 0;
                sNetSprites[slot].playerId = i;
                sNetSprites[slot].targetX = players[i].x;
                sNetSprites[slot].targetY = players[i].y;
                sNetSprites[slot].currentX = players[i].x;
                sNetSprites[slot].currentY = players[i].y;
                sNetSprites[slot].direction = players[i].direction;
                sNetSprites[slot].graphicsId = GetGraphicsForSprite(players[i].trainerSprite);
                sNetSprites[slot].isRunning = players[i].isRunning;
                sNetSprites[slot].isBusy = players[i].isBusy;
                sNetSprites[slot].wasBusy = players[i].isBusy;
                sNetSprites[slot].busyBubbleTimer = 0;
                SpawnNetSprite(slot);
            }
        }
        else if (onSameMap && sprIdx >= 0)
        {
            struct NetSprite *ns = &sNetSprites[sprIdx];
            ns->targetX = players[i].x;
            ns->targetY = players[i].y;
            ns->direction = players[i].direction;
            ns->isRunning = players[i].isRunning;
            ns->isBusy = players[i].isBusy;

            if (ns->isBusy && !ns->wasBusy && ns->spawned
                && ns->objectEventId < OBJECT_EVENTS_COUNT
                && gObjectEvents[ns->objectEventId].active)
            {
                ObjectEventGetLocalIdAndMap(&gObjectEvents[ns->objectEventId],
                    &gFieldEffectArguments[0], &gFieldEffectArguments[1], &gFieldEffectArguments[2]);
                FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);
            }
            ns->wasBusy = ns->isBusy;

            if (ns->spawned
                && (ns->objectEventId >= OBJECT_EVENTS_COUNT
                    || !gObjectEvents[ns->objectEventId].active))
            {
                ns->spawned = 0;
            }

            if (!ns->spawned)
            {
                SpawnNetSprite(sprIdx);
            }
            else if (ns->objectEventId < OBJECT_EVENTS_COUNT
                     && gObjectEvents[ns->objectEventId].active)
            {
                struct ObjectEvent *obj = &gObjectEvents[ns->objectEventId];

                if (ns->targetX != ns->currentX || ns->targetY != ns->currentY)
                {
                    s16 dx = ns->targetX - ns->currentX;
                    s16 dy = ns->targetY - ns->currentY;

                    // If more than 3 tiles away, teleport
                    if (dx > 3 || dx < -3 || dy > 3 || dy < -3)
                    {
                        obj->currentCoords.x = ns->targetX;
                        obj->currentCoords.y = ns->targetY;
                        ns->currentX = ns->targetX;
                        ns->currentY = ns->targetY;
                    }
                    else
                    {
                        // Walk one tile toward target
                        if (ObjectEventClearHeldMovementIfFinished(obj))
                        {
                            // Previous walk finished — sync current pos
                            ns->currentX = obj->currentCoords.x;
                            ns->currentY = obj->currentCoords.y;

                            // Still need to move? Start next walk
                            dx = ns->targetX - ns->currentX;
                            dy = ns->targetY - ns->currentY;
                            if (dx != 0 || dy != 0)
                            {
                                u8 moveAction;
                                if (ns->isRunning)
                                {
                                    if (dy < 0)      moveAction = MOVEMENT_ACTION_PLAYER_RUN_UP;
                                    else if (dy > 0) moveAction = MOVEMENT_ACTION_PLAYER_RUN_DOWN;
                                    else if (dx < 0) moveAction = MOVEMENT_ACTION_PLAYER_RUN_LEFT;
                                    else             moveAction = MOVEMENT_ACTION_PLAYER_RUN_RIGHT;
                                }
                                else
                                {
                                    if (dy < 0)      moveAction = MOVEMENT_ACTION_WALK_NORMAL_UP;
                                    else if (dy > 0) moveAction = MOVEMENT_ACTION_WALK_NORMAL_DOWN;
                                    else if (dx < 0) moveAction = MOVEMENT_ACTION_WALK_NORMAL_LEFT;
                                    else             moveAction = MOVEMENT_ACTION_WALK_NORMAL_RIGHT;
                                }
                                ObjectEventSetHeldMovement(obj, moveAction);
                            }
                        }
                    }
                }
                else
                {
                    ObjectEventClearHeldMovementIfFinished(obj);
                    ObjectEventTurn(obj, ns->direction);
                }
            }
        }
        else if (!onSameMap && sprIdx >= 0)
        {
            DespawnNetSprite(sprIdx);
        }
    }

    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        if (sNetSprites[i].active && !players[sNetSprites[i].playerId].active)
            DespawnNetSprite(i);
    }

    NetInteraction_CheckIncomingRequests();
}

int NetPlayerSprites_GetCount(void)
{
    int count = 0;
    for (int i = 0; i < MAX_NET_SPRITES; i++)
        if (sNetSprites[i].active) count++;
    return count;
}

void NetPlayerSprites_GetInfo(int index, s16 *x, s16 *y, u8 *dir, const char **username)
{
    int count = 0;
    struct RemotePlayer *players = NetClient_GetRemotePlayers();
    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        if (sNetSprites[i].active)
        {
            if (count == index)
            {
                *x = sNetSprites[i].targetX;
                *y = sNetSprites[i].targetY;
                *dir = sNetSprites[i].direction;
                *username = players[sNetSprites[i].playerId].username;
                return;
            }
            count++;
        }
    }
}

void NetPlayerSprites_ClearAll(void)
{
    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        if (sNetSprites[i].spawned)
            DespawnNetSprite(i);
    }
    memset(sNetSprites, 0, sizeof(sNetSprites));
}

int NetPlayerSprites_GetScreenPositions(int *outX, int *outY, const char **outNames, int maxOut)
{
    int count = 0;
    struct RemotePlayer *players = NetClient_GetRemotePlayers();

    for (int i = 0; i < MAX_NET_SPRITES && count < maxOut; i++)
    {
        if (!sNetSprites[i].active || !sNetSprites[i].spawned)
            continue;
        u8 objId = sNetSprites[i].objectEventId;
        if (objId >= OBJECT_EVENTS_COUNT || !gObjectEvents[objId].active)
            continue;

        u8 spriteId = gObjectEvents[objId].spriteId;
        if (spriteId >= MAX_SPRITES || !gSprites[spriteId].inUse)
            continue;

        struct Sprite *spr = &gSprites[spriteId];
        int sx, sy;
        if (spr->coordOffsetEnabled)
        {
            extern s16 gSpriteCoordOffsetX, gSpriteCoordOffsetY;
            sx = spr->x + spr->x2 + spr->centerToCornerVecX + gSpriteCoordOffsetX;
            sy = spr->y + spr->y2 + spr->centerToCornerVecY + gSpriteCoordOffsetY;
        }
        else
        {
            sx = spr->x + spr->x2 + spr->centerToCornerVecX;
            sy = spr->y + spr->y2 + spr->centerToCornerVecY;
        }
        // Center horizontally on sprite, position above head
        outX[count] = sx + 8;  // center of 16px wide sprite
        outY[count] = sy - 4;  // above sprite top
        outNames[count] = players[sNetSprites[i].playerId].username;
        count++;
    }
    return count;
}

// Check if player is facing a net sprite. Returns player ID or -1.
int NetPlayerSprites_GetFacingPlayerId(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 facingX = player->currentCoords.x;
    s16 facingY = player->currentCoords.y;

    switch (player->facingDirection)
    {
    case DIR_SOUTH: facingY++; break;
    case DIR_NORTH: facingY--; break;
    case DIR_WEST:  facingX--; break;
    case DIR_EAST:  facingX++; break;
    }

    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        if (!sNetSprites[i].active || !sNetSprites[i].spawned) continue;
        if (sNetSprites[i].currentX == facingX && sNetSprites[i].currentY == facingY)
            return sNetSprites[i].playerId;
    }
    return -1;
}

// Check if a remote player is busy (in battle/dialogue)
int NetPlayerSprites_IsPlayerBusy(int playerId)
{
    struct RemotePlayer *players = NetClient_GetRemotePlayers();
    if (playerId >= 0 && playerId < MAX_PLAYERS && players[playerId].active)
        return players[playerId].isBusy;
    return 0;
}

void NetPlayerSprites_SetPlayerGraphics(u8 trainerSprite)
{
    u8 gfxId = GetGraphicsForSprite(trainerSprite);
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventSetGraphicsId(player, gfxId);
}

void NetPlayerSprites_OnMapLoad(void)
{
    if (!sInitialized) InitNetSprites();

    for (int i = 0; i < MAX_NET_SPRITES; i++)
    {
        if (sNetSprites[i].spawned)
            DespawnNetSprite(i);
        else
        {
            sNetSprites[i].spawned = 0;
            sNetSprites[i].active = 0;
        }
    }

    if (!NetClient_IsConnected()) return;

    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    extern void NetClient_SendPosition(u16, u16, s16, s16, u8, u8);
    NetClient_SendPosition(
        gSaveBlock1Ptr->location.mapGroup,
        gSaveBlock1Ptr->location.mapNum,
        player->currentCoords.x,
        player->currentCoords.y,
        player->facingDirection,
        0
    );

    extern uint8_t NetClient_GetMyTrainerSprite(void);
    u8 trSpr = NetClient_GetMyTrainerSprite();
    u8 gfxId = GetGraphicsForSprite(trSpr);
    ObjectEventSetGraphicsId(player, gfxId);

    struct RemotePlayer *players = NetClient_GetRemotePlayers();
    u16 myMapGroup = gSaveBlock1Ptr->location.mapGroup;
    u16 myMapNum = gSaveBlock1Ptr->location.mapNum;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!players[i].active) continue;
        if (players[i].mapGroup == myMapGroup && players[i].mapNum == myMapNum)
        {
            int slot = FindFreeNetSprite();
            if (slot >= 0)
            {
                sNetSprites[slot].active = 1;
                sNetSprites[slot].spawned = 0;
                sNetSprites[slot].playerId = i;
                sNetSprites[slot].targetX = players[i].x;
                sNetSprites[slot].targetY = players[i].y;
                sNetSprites[slot].currentX = players[i].x;
                sNetSprites[slot].currentY = players[i].y;
                sNetSprites[slot].direction = players[i].direction;
                sNetSprites[slot].graphicsId = GetGraphicsForSprite(players[i].trainerSprite);
                sNetSprites[slot].isBusy = players[i].isBusy;
                sNetSprites[slot].wasBusy = players[i].isBusy;
            }
        }
    }
}

// --- Player Interaction ---
#include "task.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "window.h"
#include "text.h"
#include "string_util.h"
#include "event_object_lock.h"
#include "script.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "constants/battle.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "pokemon.h"
#include "sound.h"
#include "link.h"
#include "field_weather.h"
#include "palette.h"

static const u8 sText_Battle[] = _("BATTLE");
static const u8 sText_Trade[] = _("TRADE");
static const u8 sText_Cancel[] = _("CANCEL");

static EWRAM_DATA int sInteractPlayerId = -1;
static EWRAM_DATA int sNetBattleOpponentPic = -1;

static const struct MenuAction sInteractMenuActions[] = {
    {sText_Battle, {.void_u8 = NULL}},
    {sText_Trade, {.void_u8 = NULL}},
    {sText_Cancel, {.void_u8 = NULL}},
};

static const struct WindowTemplate sInteractWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 1,
    .width = 8,
    .height = 6,
    .paletteNum = 15,
    .baseBlock = 0x1D,
};

static void Task_WaitNetBattleTransition(u8 taskId);
static void CB2_EndNetBattle(void);
static void Task_WaitDeclinedMsg(u8 taskId);
static void Task_WaitResponseMsg(u8 taskId);
static void Task_IncomingRequestTextDone(u8 taskId);
static void Task_IncomingRequest(u8 taskId);
static void Task_InteractMenu(u8 taskId);
static void Task_SendPartyAndWait(u8 taskId);
void NetInteraction_CheckIncomingRequests(void);

static void AsciiToGameStr(u8 *dst, const char *src, int maxLen)
{
    int i;
    for (i = 0; i < maxLen - 1 && src[i]; i++)
    {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') dst[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') dst[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') dst[i] = 0xA1 + (c - '0');
        else if (c == ' ') dst[i] = 0x00;
        else if (c == '!') dst[i] = 0xAB;
        else if (c == '?') dst[i] = 0xAC;
        else if (c == '.') dst[i] = 0xAD;
        else dst[i] = 0x00;
    }
    dst[i] = 0xFF;
}

static void CloseInteractMenu(u8 taskId)
{
    ClearStdWindowAndFrame(gTasks[taskId].data[0], TRUE);
    RemoveWindow(gTasks[taskId].data[0]);
    DestroyTask(taskId);
    ClearPlayerHeldMovementAndUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

// --- Waiting for response after sending request ---
static void Task_WaitResponse(u8 taskId);
static void Task_WaitResponseMsg(u8 taskId)
{
    if (!RunTextPrinters_CheckPrinter0Active())
    {
        gTasks[taskId].func = Task_WaitResponse;
    }
}

static void Task_StartNetBattleFade(u8 taskId)
{
    switch (gTasks[taskId].data[0])
    {
    case 0:
        gTasks[taskId].data[0]++;
        gTasks[taskId].data[2] = 0;
        break;
    case 1:
        if (++gTasks[taskId].data[2] > 2)
            gTasks[taskId].data[0]++;
        break;
    case 2:
    {
        int partnerId = gTasks[taskId].data[1];
        struct RemotePlayer *players = NetClient_GetRemotePlayers();
        u8 opSprite = (partnerId >= 0 && partnerId < MAX_PLAYERS) ? players[partnerId].trainerSprite : 0;
        switch (opSprite)
        {
        default:
        case 0: sNetBattleOpponentPic = TRAINER_PIC_RED; break;
        case 1: sNetBattleOpponentPic = TRAINER_PIC_LEAF; break;
        case 2: sNetBattleOpponentPic = TRAINER_PIC_RS_BRENDAN_1; break;
        case 3: sNetBattleOpponentPic = TRAINER_PIC_RS_MAY_1; break;
        }

        u8 myId = NetClient_GetPlayerId();
        bool8 isMaster = (myId < partnerId);

        extern void NetLink_Init(u8 partnerId, bool8 isMaster);
        NetLink_Init(partnerId, isMaster);

        gBattleTypeFlags = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER
                         | (isMaster ? BATTLE_TYPE_IS_MASTER : 0);
        gTrainerBattleOpponent_A = TRAINER_LINK_OPPONENT;
        gLinkType = LINKTYPE_BATTLE;
        PlayMapChosenOrBattleBGM(0);
        CleanupOverworldWindowsAndTilemaps();
        gMain.savedCallback = CB2_EndNetBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
        break;
    }
    }
}

static void FieldCB_AfterNetTrade(void)
{
    UnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    ScriptContext_Enable();
    Overworld_PlaySpecialMapMusic();
}

static void CB2_EndNetTrade(void)
{
    extern void NetLink_Cleanup(void);
    NetLink_Cleanup();
    NetClient_ClearInteraction();
    extern void (*gFieldCallback)(void);
    gFieldCallback = FieldCB_AfterNetTrade;
    extern void CB1_Overworld(void);
    extern void SetMainCallback1(MainCallback cb);
    SetMainCallback1(CB1_Overworld);
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_StartNetTradeFade(u8 taskId)
{
    switch (gTasks[taskId].data[0])
    {
    case 0:
        gTasks[taskId].data[0]++;
        gTasks[taskId].data[2] = 0;
        break;
    case 1:
        if (++gTasks[taskId].data[2] > 2)
            gTasks[taskId].data[0]++;
        break;
    case 2:
    {
        int partnerId = gTasks[taskId].data[1];
        u8 myId = NetClient_GetPlayerId();
        bool8 isMaster = (myId < partnerId);

        extern void NetLink_Init(u8 partnerId, bool8 isMaster);
        NetLink_Init(partnerId, isMaster);

        gLinkType = LINKTYPE_TRADE;
        gLinkPlayers[0].linkType = LINKTYPE_TRADE;
        gLinkPlayers[1].linkType = LINKTYPE_TRADE;
        CleanupOverworldWindowsAndTilemaps();
        gMain.savedCallback = CB2_EndNetTrade;

        extern void CB2_StartCreateTradeMenu(void);
        SetMainCallback2(CB2_StartCreateTradeMenu);
        DestroyTask(taskId);
        break;
    }
    }
}

static void StartNetTrade(u8 taskId)
{
    ClearDialogWindowAndFrame(0, TRUE);
    ClearPlayerHeldMovementAndUnfreezeObjectEvents();
    UnlockPlayerFieldControls();

    int partnerId = NetClient_GetInteractPartnerId();
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = partnerId;
    gTasks[taskId].func = Task_StartNetTradeFade;
}

static void StartNetBattle(u8 taskId)
{
    ClearDialogWindowAndFrame(0, TRUE);
    ClearPlayerHeldMovementAndUnfreezeObjectEvents();
    UnlockPlayerFieldControls();

    int partnerId = NetClient_GetInteractPartnerId();
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = partnerId;
    gTasks[taskId].func = Task_StartNetBattleFade;
}

static void Task_WaitResponse(u8 taskId)
{
    int state = NetClient_GetInteractState();
    if ((state == NET_INTERACT_BATTLE_PREP || state == NET_INTERACT_TRADE_PREP
      || state == NET_INTERACT_BATTLE_READY || state == NET_INTERACT_TRADE_SELECT)
        && gTasks[taskId].data[2] == 0)
    {
        gTasks[taskId].data[2] = 1;
        extern u8 gPlayerPartyCount;
        extern struct Pokemon gPlayerParty[];
        NetClient_SendPartyData(gPlayerParty, gPlayerPartyCount);
        u8 *text = gStringVar4;
        AsciiToGameStr(text, "Preparing...", 32);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_WaitResponseMsg);
        return;
    }
    if (state == NET_INTERACT_BATTLE_READY)
    {
        StartNetBattle(taskId);
        return;
    }
    if (state == NET_INTERACT_TRADE_SELECT)
    {
        StartNetTrade(taskId);
        return;
    }
    if (state == NET_INTERACT_NONE)
    {
        u8 *text = gStringVar4;
        AsciiToGameStr(text, "Declined.", 32);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_WaitDeclinedMsg);
        return;
    }
    if (gMain.newKeys & B_BUTTON)
    {
        NetClient_ClearInteraction();
        ClearDialogWindowAndFrame(0, TRUE);
        ClearPlayerHeldMovementAndUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
    }
}

static void Task_WaitDeclinedMsg(u8 taskId)
{
    if (gMain.newKeys & (A_BUTTON | B_BUTTON))
    {
        ClearDialogWindowAndFrame(0, TRUE);
        ClearPlayerHeldMovementAndUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
    }
}

int NetBattle_GetOpponentTrainerPic(void)
{
    return sNetBattleOpponentPic;
}

static void CB2_EndNetBattle(void)
{
    sNetBattleOpponentPic = -1;
    extern void NetLink_Cleanup(void);
    NetLink_Cleanup();
    NetClient_ClearInteraction();
    gBattleTypeFlags = 0;
    gMain.inBattle = FALSE;
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_WaitNetBattleTransition(u8 taskId)
{
    if (IsBattleTransitionDone())
    {
        gMain.savedCallback = CB2_EndNetBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// --- Sending party data after accept ---
static void Task_SendPartyAndWait(u8 taskId)
{
    switch (gTasks[taskId].data[1])
    {
    case 0:
    {
        extern u8 gPlayerPartyCount;
        extern struct Pokemon gPlayerParty[];
        NetClient_SendPartyData(gPlayerParty, gPlayerPartyCount);
        u8 *text = gStringVar4;
        AsciiToGameStr(text, "Preparing...", 32);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_WaitResponseMsg);
        gTasks[taskId].data[1]++;
        break;
    }
    case 1:
        gTasks[taskId].func = Task_WaitResponse;
        break;
    }
}

// --- Interaction menu ---
static void Task_InteractMenu(u8 taskId)
{
    switch (gTasks[taskId].data[1])
    {
    case 0:
        LoadStdWindowFrameGfx();
        gTasks[taskId].data[1]++;
        break;
    case 1:
    {
        u8 windowId = AddWindow(&sInteractWindowTemplate);
        gTasks[taskId].data[0] = windowId;
        DrawStdWindowFrame(windowId, FALSE);
        PrintTextArray(windowId, FONT_NORMAL, GetMenuCursorDimensionByFont(FONT_NORMAL, 0), 2, 16, 3, sInteractMenuActions);
        Menu_InitCursor(windowId, FONT_NORMAL, 0, 2, 16, 3, 0);
        CopyWindowToVram(windowId, COPYWIN_FULL);
        gTasks[taskId].data[1]++;
        break;
    }
    case 2:
    {
        s8 input = Menu_ProcessInput();
        if (input == MENU_B_PRESSED || input == 2)
        {
            CloseInteractMenu(taskId);
        }
        else if (input == 0)
        {
            ClearStdWindowAndFrame(gTasks[taskId].data[0], TRUE);
            RemoveWindow(gTasks[taskId].data[0]);
            NetClient_SendBattleRequest(sInteractPlayerId);
            u8 *text = gStringVar4;
            AsciiToGameStr(text, "Waiting...", 32);
            DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_WaitResponseMsg);
        }
        else if (input == 1)
        {
            ClearStdWindowAndFrame(gTasks[taskId].data[0], TRUE);
            RemoveWindow(gTasks[taskId].data[0]);
            NetClient_SendTradeRequest(sInteractPlayerId);
            u8 *text = gStringVar4;
            AsciiToGameStr(text, "Waiting...", 32);
            DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_WaitResponseMsg);
        }
        break;
    }
    }
}

void NetInteraction_ShowMenu(int playerId)
{
    sInteractPlayerId = playerId;
    LockPlayerFieldControls();
    FreezeObjectEvents();
    u8 taskId = CreateTask(Task_InteractMenu, 80);
    gTasks[taskId].data[1] = 0;
}

// --- Incoming request handling ---
static const struct WindowTemplate sYesNoWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 6,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x125,
};

static void Task_IncomingRequest(u8 taskId)
{
    switch (gTasks[taskId].data[1])
    {
    case 0:
    {
        LockPlayerFieldControls();
        FreezeObjectEvents();
        LoadStdWindowFrameGfx();
        const char *name = NetClient_GetInteractPartnerName();
        int type = NetClient_GetInteractType();
        char msg[64];
        if (type == MSG_C_BATTLE_REQUEST)
            snprintf(msg, sizeof(msg), "%s wants to battle!", name);
        else
            snprintf(msg, sizeof(msg), "%s wants to trade!", name);
        u8 *text = gStringVar4;
        AsciiToGameStr(text, msg, 64);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, text, Task_IncomingRequestTextDone);
        gTasks[taskId].data[1]++;
        break;
    }
    case 1:
        break;
    case 2:
        CreateYesNoMenu(&sYesNoWindowTemplate, FONT_NORMAL, 0, 2, 0x214, 14, 0);
        gTasks[taskId].data[1]++;
        break;
    case 3:
    {
        s8 input = Menu_ProcessInputNoWrapClearOnChoose();
        if (input == 0)
        {
            int partnerId = NetClient_GetInteractPartnerId();
            int type = NetClient_GetInteractType();
            NetClient_SendInteractResponse(partnerId, 1, type);
            ClearDialogWindowAndFrame(0, TRUE);
            gTasks[taskId].data[1] = 0;
            gTasks[taskId].func = Task_SendPartyAndWait;
        }
        else if (input == 1 || input == MENU_B_PRESSED)
        {
            int partnerId = NetClient_GetInteractPartnerId();
            int type = NetClient_GetInteractType();
            NetClient_SendInteractResponse(partnerId, 0, type);
            ClearDialogWindowAndFrame(0, TRUE);
            ClearPlayerHeldMovementAndUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
    }
}

static void Task_IncomingRequestTextDone(u8 taskId)
{
    gTasks[taskId].data[1] = 2;
    gTasks[taskId].func = Task_IncomingRequest;
}

void NetInteraction_CheckIncomingRequests(void)
{
    if (NetClient_GetInteractState() == NET_INTERACT_REQUESTED)
    {
        if (!FuncIsActiveTask(Task_IncomingRequest)
            && !FuncIsActiveTask(Task_InteractMenu)
            && !FuncIsActiveTask(Task_SendPartyAndWait)
            && !FuncIsActiveTask(Task_WaitResponse)
            && !FuncIsActiveTask(Task_StartNetBattleFade)
            && !FuncIsActiveTask(Task_StartNetTradeFade))
        {
            u8 taskId = CreateTask(Task_IncomingRequest, 80);
            gTasks[taskId].data[1] = 0;
        }
    }
}

