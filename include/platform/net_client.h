#ifndef GUARD_NET_CLIENT_H
#define GUARD_NET_CLIENT_H

#include "../../server/protocol.h"

struct RemotePlayer {
    int active;
    uint8_t id;
    char username[MAX_USERNAME_LEN];
    uint8_t trainerSprite;
    uint16_t mapGroup;
    uint16_t mapNum;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint8_t isRunning;
    uint8_t isBusy;
    int needsSpawn;
    int needsDespawn;
};

struct ChatEntry {
    char username[MAX_USERNAME_LEN];
    char message[MAX_CHAT_LEN];
    int timer;
};

int NetClient_Connect(const char *ip, int port, const char *username, uint8_t trainerSprite,
                      uint16_t mapGroup, uint16_t mapNum, int16_t x, int16_t y, uint8_t direction);
uint8_t NetClient_GetMyTrainerSprite(void);
void NetClient_Disconnect(void);
void NetClient_SendPosition(uint16_t mapGroup, uint16_t mapNum, int16_t x, int16_t y, uint8_t direction, uint8_t isRunning);
void NetClient_SendChat(const char *message);
void NetClient_Poll(void);
int NetClient_IsConnected(void);
uint8_t NetClient_GetPlayerId(void);
struct RemotePlayer *NetClient_GetRemotePlayers(void);
int NetClient_GetChatCount(void);
struct ChatEntry *NetClient_GetChatHistory(void);
void NetClient_TickChat(void);

void NetClient_SendBattleRequest(uint8_t targetPlayerId);
void NetClient_SendTradeRequest(uint8_t targetPlayerId);
void NetClient_SendInteractResponse(uint8_t targetPlayerId, uint8_t accepted, uint8_t type);
void NetClient_SendPartyData(const void *partyData, int partyCount);
void NetClient_SendTradeSelection(uint8_t slot);

int NetClient_GetInteractState(void);
int NetClient_GetInteractPartnerId(void);
int NetClient_GetInteractType(void);
const char *NetClient_GetInteractPartnerName(void);
void *NetClient_GetReceivedParty(void);
int NetClient_GetReceivedPartyCount(void);
int NetClient_GetPartnerTradeSlot(void);
void NetClient_ClearInteraction(void);

#define NET_INTERACT_NONE        0
#define NET_INTERACT_REQUESTING  1
#define NET_INTERACT_REQUESTED   2
#define NET_INTERACT_BATTLE_PREP 3
#define NET_INTERACT_BATTLE_READY 4
#define NET_INTERACT_TRADE_PREP  5
#define NET_INTERACT_TRADE_SELECT 6
#define NET_INTERACT_TRADE_WAIT  7
#define NET_INTERACT_TRADE_READY 8

#endif // GUARD_NET_CLIENT_H
