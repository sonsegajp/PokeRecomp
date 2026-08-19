#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <stdio.h>
#include <string.h>
#include "../../server/protocol.h"

static SOCKET sSocket = INVALID_SOCKET;
static int sConnected = 0;
static int sNeedsInitialPosition = 0;
static uint8_t sMyPlayerId = 0;
static char sMyUsername[MAX_USERNAME_LEN] = {0};

static char sNewChatUser[MAX_USERNAME_LEN] = {0};
static char sNewChatMsg[MAX_CHAT_LEN] = {0};
static int sHasNewChat = 0;

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

static struct RemotePlayer sRemotePlayers[MAX_PLAYERS];

// Chat message buffer
#define MAX_CHAT_HISTORY 8
struct ChatEntry {
    char username[MAX_USERNAME_LEN];
    char message[MAX_CHAT_LEN];
    int timer;
};
static struct ChatEntry sChatHistory[MAX_CHAT_HISTORY];
static int sChatCount = 0;

static uint8_t sMyTrainerSprite = 0;
static uint16_t sServerShinyRate = 8192;

#define NET_INTERACT_NONE        0
#define NET_INTERACT_REQUESTING  1
#define NET_INTERACT_REQUESTED   2
#define NET_INTERACT_BATTLE_PREP 3
#define NET_INTERACT_BATTLE_READY 4
#define NET_INTERACT_TRADE_PREP  5
#define NET_INTERACT_TRADE_SELECT 6
#define NET_INTERACT_TRADE_WAIT  7
#define NET_INTERACT_TRADE_READY 8

static int sInteractState = NET_INTERACT_NONE;
static int sInteractPartnerId = -1;
static int sInteractType = 0;
static uint8_t sReceivedPartyData[600];
static int sReceivedPartyCount = 0;
static int sReceivedChunks = 0;
static int sMyTradeSlot = -1;
static int sPartnerTradeSlot = -1;

int NetClient_Connect(const char *ip, int port, const char *username, uint8_t trainerSprite,
                      uint16_t mapGroup, uint16_t mapNum, int16_t x, int16_t y, uint8_t direction,
                      const uint8_t *passwordHash)
{
    sMyTrainerSprite = trainerSprite;
    {
        extern void NetClient_SetPlayerGender(uint8_t gender);
        NetClient_SetPlayerGender((trainerSprite == 1 || trainerSprite == 3) ? 1 : 0);
    }
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    memset(sRemotePlayers, 0, sizeof(sRemotePlayers));
    strncpy(sMyUsername, username, MAX_USERNAME_LEN - 1);

    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == INVALID_SOCKET)
    {
        printf("[NET] Failed to create socket\n");
        return 0;
    }
    { int one = 1; setsockopt(sSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one)); }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    printf("[NET] Connecting to %s:%d...\n", ip, port);
    fflush(stdout);

    if (connect(sSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("[NET] Connection failed\n");
        closesocket(sSocket);
        sSocket = INVALID_SOCKET;
        return 0;
    }

    // Send connect packet with current position
    struct ConnectPacket pkt = {
        .header = { .type = MSG_C_CONNECT, .size = sizeof(struct ConnectPacket) },
        .protocolVersion = PROTOCOL_VERSION,
        .trainerSprite = trainerSprite,
        .mapGroup = mapGroup,
        .mapNum = mapNum,
        .x = x,
        .y = y,
        .direction = direction
    };
    strncpy(pkt.username, username, MAX_USERNAME_LEN - 1);
    if (passwordHash)
        memcpy(pkt.passwordHash, passwordHash, PASSWORD_HASH_LEN);
    else
        memset(pkt.passwordHash, 0, PASSWORD_HASH_LEN);
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);

    // Wait for login result (blocking mode)
    {
        struct LoginResultPacket loginResult;
        int received = recv(sSocket, (char *)&loginResult, sizeof(loginResult), 0);
        if (received < (int)sizeof(loginResult) || loginResult.header.type != MSG_S_LOGIN_RESULT)
        {
            printf("[NET] No login response received\n");
            closesocket(sSocket); sSocket = INVALID_SOCKET; return 0;
        }
        if (loginResult.result == LOGIN_BAD_PASSWORD)
        {
            printf("[NET] Bad password for user '%s'\n", username);
            closesocket(sSocket); sSocket = INVALID_SOCKET; return 0;
        }
        sMyPlayerId = loginResult.playerId;
        sConnected = 1;
        int isNewAccount = (loginResult.result == LOGIN_NEW_ACCOUNT);
        printf("[NET] Connected! Player ID=%d, %d online%s\n",
            sMyPlayerId, loginResult.playerCount, isNewAccount ? " (new account)" : "");
        fflush(stdout);

        // Receive save data chunks (blocking)
        extern uint8_t FLASH_BASE[];
        if (!isNewAccount)
        {
            int totalChunks = (SAVE_TOTAL_SIZE + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
            for (int i = 0; i < totalChunks; i++)
            {
                struct SaveDataPacket savePkt;
                int got = 0;
                while (got < (int)sizeof(savePkt))
                {
                    int r = recv(sSocket, (char *)&savePkt + got, sizeof(savePkt) - got, 0);
                    if (r <= 0) break;
                    got += r;
                }
                if (got >= (int)sizeof(struct PacketHeader) && savePkt.header.type == MSG_S_SAVE_DATA)
                {
                    int offset = savePkt.chunkIndex * SAVE_CHUNK_SIZE;
                    int copySize = savePkt.dataSize;
                    if (offset + copySize <= SAVE_TOTAL_SIZE)
                        memcpy(&FLASH_BASE[offset], savePkt.data, copySize);
                }
            }
            {
                int nonFF = 0;
                for (int k = 0; k < 256; k++)
                    if (FLASH_BASE[k] != 0xFF) nonFF++;
                printf("[NET] Save data received (%d non-FF in first 256 bytes)\n", nonFF);
                fflush(stdout);
            }
            extern uint8_t LoadGameSave(uint8_t saveType);
            LoadGameSave(0);
            extern void SetMainCallback2(void (*cb)(void));
            extern void SetMainCallback1(void (*cb)(void));
            extern void CB1_Overworld(void);
            extern void CB2_LoadMap(void);
            extern void LoadSaveblockMapHeader(void);
            extern void LoadSaveblockObjEventScripts(void);
            extern void WarpIntoMap(void);

            LoadSaveblockMapHeader();
            LoadSaveblockObjEventScripts();
            SetMainCallback1(CB1_Overworld);
            SetMainCallback2(CB2_LoadMap);
        }
        else
        {
            memset(FLASH_BASE, 0xFF, SAVE_TOTAL_SIZE);
            printf("[NET] New account, starting fresh\n"); fflush(stdout);
            extern void SetMainCallback2(void (*cb)(void));
            extern void CB2_InitCopyrightScreenAfterBootup(void);
            SetMainCallback2(CB2_InitCopyrightScreenAfterBootup);
        }
    }

    // NOW set non-blocking for future reads
#ifdef _WIN32
    {
        unsigned long mode = 1;
        ioctlsocket(sSocket, FIONBIO, &mode);
    }
#endif

    // Receive remaining data (player list, config, motd)
    {
        char buf[4096];
        int received = recv(sSocket, buf, sizeof(buf), 0);
        if (received > 0)
        {
            int offset = 0;
            while (offset + (int)sizeof(struct PacketHeader) <= received)
            {
                struct PacketHeader *h = (struct PacketHeader *)(buf + offset);
                if (offset + h->size > received) break;

                if (h->type == MSG_S_PLAYER_JOIN)
                {
                    struct PlayerJoinPacket *join = (struct PlayerJoinPacket *)(buf + offset);
                    int id = join->playerId;
                    if (id < MAX_PLAYERS)
                    {
                        sRemotePlayers[id].active = 1;
                        sRemotePlayers[id].id = id;
                        sRemotePlayers[id].trainerSprite = join->trainerSprite;
                        sRemotePlayers[id].mapGroup = join->mapGroup;
                        sRemotePlayers[id].mapNum = join->mapNum;
                        sRemotePlayers[id].x = join->x;
                        sRemotePlayers[id].y = join->y;
                        sRemotePlayers[id].direction = join->direction;
                        sRemotePlayers[id].needsSpawn = 1;
                        strncpy(sRemotePlayers[id].username, join->username, MAX_USERNAME_LEN);
                        printf("[NET] Player online: %s (id=%d)\n", join->username, id);
                    }
                }
                else if (h->type == MSG_S_SERVER_CONFIG)
                {
                    struct ServerConfigDataPacket *cfg = (struct ServerConfigDataPacket *)(buf + offset);
                    sServerShinyRate = cfg->shinyRate;
                    if (sServerShinyRate == 0) sServerShinyRate = 8192;
                }
                else if (h->type == MSG_S_SERVER_MSG)
                {
                    struct ServerMsgPacket *msg = (struct ServerMsgPacket *)(buf + offset);
                    if (sChatCount < MAX_CHAT_HISTORY)
                    {
                        strncpy(sChatHistory[sChatCount].username, "[Server]", MAX_USERNAME_LEN);
                        strncpy(sChatHistory[sChatCount].message, msg->message, MAX_CHAT_LEN);
                        sChatHistory[sChatCount].timer = 8000;
                        sChatCount++;
                    }
                }
                offset += h->size;
            }
        }
    }

    sNeedsInitialPosition = 1;
    return 0;
}

void NetClient_Disconnect(void)
{
    if (!sConnected) return;

    struct PacketHeader pkt = { .type = MSG_C_DISCONNECT, .size = sizeof(struct PacketHeader) };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);

    closesocket(sSocket);
    sSocket = INVALID_SOCKET;
    sConnected = 0;
    memset(sRemotePlayers, 0, sizeof(sRemotePlayers));
    printf("[NET] Disconnected\n");
}

void NetClient_SendPosition(uint16_t mapGroup, uint16_t mapNum, int16_t x, int16_t y, uint8_t direction, uint8_t statusFlags)
{
    if (!sConnected) return;

    struct PositionPacket pkt = {
        .header = { .type = MSG_C_POSITION, .size = sizeof(struct PositionPacket) },
        .playerId = sMyPlayerId,
        .mapGroup = mapGroup,
        .mapNum = mapNum,
        .x = x,
        .y = y,
        .direction = direction,
        .movementAction = statusFlags
    };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
}

void NetClient_SendChat(const char *message)
{
    if (!sConnected) return;

    struct ChatPacket pkt = {
        .header = { .type = MSG_C_CHAT, .size = sizeof(struct ChatPacket) },
        .playerId = sMyPlayerId
    };
    strncpy(pkt.username, sMyUsername, MAX_USERNAME_LEN);
    strncpy(pkt.message, message, MAX_CHAT_LEN - 1);
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
}

static char sRecvBuf[8192];
static int sRecvBufLen = 0;

void NetClient_Poll(void)
{
    if (!sConnected) return;

    int space = sizeof(sRecvBuf) - sRecvBufLen;
    if (space <= 0) { sRecvBufLen = 0; return; }
    int received = recv(sSocket, sRecvBuf + sRecvBufLen, space, 0);
    if (received > 0) sRecvBufLen += received;
    if (sRecvBufLen <= 0) return;

    int offset = 0;
    while (offset < sRecvBufLen)
    {
        if (sRecvBufLen - offset < (int)sizeof(struct PacketHeader)) break;
        struct PacketHeader *h = (struct PacketHeader *)(sRecvBuf + offset);
        if (h->size == 0 || h->size > 4096) { sRecvBufLen = 0; return; }
        if (offset + h->size > sRecvBufLen) break;

        switch (h->type)
        {
        case MSG_S_PLAYER_JOIN:
        {
            struct PlayerJoinPacket *join = (struct PlayerJoinPacket *)(sRecvBuf + offset);
            int id = join->playerId;
            if (id < MAX_PLAYERS && id != sMyPlayerId)
            {
                sRemotePlayers[id].active = 1;
                sRemotePlayers[id].id = id;
                sRemotePlayers[id].trainerSprite = join->trainerSprite;
                sRemotePlayers[id].mapGroup = join->mapGroup;
                sRemotePlayers[id].mapNum = join->mapNum;
                sRemotePlayers[id].x = join->x;
                sRemotePlayers[id].y = join->y;
                sRemotePlayers[id].direction = join->direction;
                sRemotePlayers[id].needsSpawn = 1;
                strncpy(sRemotePlayers[id].username, join->username, MAX_USERNAME_LEN);
                printf("[NET] %s joined\n", join->username);
            }
            break;
        }
        case MSG_S_PLAYER_LEAVE:
        {
            struct PlayerLeavePacket *leave = (struct PlayerLeavePacket *)(sRecvBuf + offset);
            int id = leave->playerId;
            if (id < MAX_PLAYERS)
            {
                printf("[NET] %s left\n", sRemotePlayers[id].username);
                sRemotePlayers[id].needsDespawn = 1;
                sRemotePlayers[id].active = 0;
            }
            break;
        }
        case MSG_S_POSITION:
        {
            struct PositionPacket *pos = (struct PositionPacket *)(sRecvBuf + offset);
            int id = pos->playerId;
            if (id < MAX_PLAYERS && sRemotePlayers[id].active)
            {
                sRemotePlayers[id].mapGroup = pos->mapGroup;
                sRemotePlayers[id].mapNum = pos->mapNum;
                sRemotePlayers[id].x = pos->x;
                sRemotePlayers[id].y = pos->y;
                sRemotePlayers[id].direction = pos->direction;
                sRemotePlayers[id].isRunning = pos->movementAction & 1;
                sRemotePlayers[id].isBusy = (pos->movementAction >> 1) & 1;
            }
            break;
        }
        case MSG_S_CHAT:
        {
            struct ChatPacket *chat = (struct ChatPacket *)(sRecvBuf + offset);
            if (sChatCount < MAX_CHAT_HISTORY)
            {
                strncpy(sChatHistory[sChatCount].username, chat->username, MAX_USERNAME_LEN);
                strncpy(sChatHistory[sChatCount].message, chat->message, MAX_CHAT_LEN);
                sChatHistory[sChatCount].timer = 5000;
                sChatCount++;
            }
            strncpy(sNewChatUser, chat->username, MAX_USERNAME_LEN - 1);
            strncpy(sNewChatMsg, chat->message, MAX_CHAT_LEN - 1);
            sHasNewChat = 1;
            break;
        }
        case MSG_S_SERVER_MSG:
        {
            struct ServerMsgPacket *msg = (struct ServerMsgPacket *)(sRecvBuf + offset);
            if (sChatCount < MAX_CHAT_HISTORY)
            {
                strncpy(sChatHistory[sChatCount].username, "[Server]", MAX_USERNAME_LEN);
                strncpy(sChatHistory[sChatCount].message, msg->message, MAX_CHAT_LEN);
                sChatHistory[sChatCount].timer = 8000;
                sChatCount++;
            }
            break;
        }
        case MSG_S_BATTLE_REQUEST:
        {
            struct InteractPacket *req = (struct InteractPacket *)(sRecvBuf + offset);
            if (sInteractState == NET_INTERACT_NONE)
            {
                sInteractState = NET_INTERACT_REQUESTED;
                sInteractPartnerId = req->fromPlayerId;
                sInteractType = MSG_C_BATTLE_REQUEST;
                printf("[NET] Battle request from player %d\n", req->fromPlayerId); fflush(stdout);
            }
            break;
        }
        case MSG_S_TRADE_REQUEST:
        {
            struct InteractPacket *req = (struct InteractPacket *)(sRecvBuf + offset);
            if (sInteractState == NET_INTERACT_NONE)
            {
                sInteractState = NET_INTERACT_REQUESTED;
                sInteractPartnerId = req->fromPlayerId;
                sInteractType = MSG_C_TRADE_REQUEST;
                printf("[NET] Trade request from player %d\n", req->fromPlayerId); fflush(stdout);
            }
            break;
        }
        case MSG_S_BATTLE_START:
        {
            if (sInteractState != NET_INTERACT_REQUESTING && sInteractState != NET_INTERACT_REQUESTED)
                break;
            sInteractState = NET_INTERACT_BATTLE_PREP;
            printf("[NET] Battle starting, sending party data\n"); fflush(stdout);
            break;
        }
        case MSG_S_TRADE_START:
        {
            if (sInteractState != NET_INTERACT_REQUESTING && sInteractState != NET_INTERACT_REQUESTED)
                break;
            sInteractState = NET_INTERACT_TRADE_PREP;
            printf("[NET] Trade starting, sending party data\n"); fflush(stdout);
            break;
        }
        case MSG_S_INTERACT_CANCEL:
        {
            printf("[NET] Interaction cancelled\n"); fflush(stdout);
            sInteractState = NET_INTERACT_NONE;
            sInteractPartnerId = -1;
            break;
        }
        case MSG_S_PARTY_DATA:
        {
            struct PartyDataPacket *pd = (struct PartyDataPacket *)(sRecvBuf + offset);
            int ci = pd->chunkIndex;
            if (ci >= 0 && ci < 3)
            {
                memcpy(sReceivedPartyData + ci * 200, pd->monData, 200);
                sReceivedChunks |= (1 << ci);
                sReceivedPartyCount = pd->totalPartyCount;
                int neededChunks = (sReceivedPartyCount + 1) / 2;
                int allReceived = 1;
                for (int c = 0; c < neededChunks; c++)
                    if (!(sReceivedChunks & (1 << c))) allReceived = 0;
                if (allReceived)
                {
                    printf("[NET] All party data received (%d mons)\n", sReceivedPartyCount); fflush(stdout);
                    if (sInteractState == NET_INTERACT_BATTLE_PREP)
                        sInteractState = NET_INTERACT_BATTLE_READY;
                    else if (sInteractState == NET_INTERACT_TRADE_PREP)
                        sInteractState = NET_INTERACT_TRADE_SELECT;
                }
            }
            break;
        }
        case MSG_S_TRADE_SELECT:
        {
            struct TradeSelectPacket *ts = (struct TradeSelectPacket *)(sRecvBuf + offset);
            sPartnerTradeSlot = ts->selectedSlot;
            printf("[NET] Partner selected trade slot %d\n", ts->selectedSlot); fflush(stdout);
            if (sInteractState == NET_INTERACT_TRADE_WAIT)
                sInteractState = NET_INTERACT_TRADE_READY;
            break;
        }
        case MSG_S_LINK_DATA:
        {
            struct LinkDataPacket *ld = (struct LinkDataPacket *)(sRecvBuf + offset);
            extern void NetLink_OnReceiveBlock(const void *data, uint16_t size);
            NetLink_OnReceiveBlock(ld->data, ld->dataSize);
            break;
        }
        case MSG_S_SERVER_CONFIG:
        {
            struct ServerConfigDataPacket *cfg = (struct ServerConfigDataPacket *)(sRecvBuf + offset);
            sServerShinyRate = cfg->shinyRate;
            if (sServerShinyRate == 0) sServerShinyRate = 8192;
            printf("[NET] Server shiny rate: 1/%d\n", sServerShinyRate);
            fflush(stdout);
            break;
        }
        }

        offset += h->size;
    }
    if (offset > 0 && offset < sRecvBufLen)
    {
        memmove(sRecvBuf, sRecvBuf + offset, sRecvBufLen - offset);
        sRecvBufLen -= offset;
    }
    else
    {
        sRecvBufLen = 0;
    }
}

uint16_t NetClient_GetServerShinyRate(void) { return sServerShinyRate; }
int NetClient_IsConnected(void) { return sConnected; }
uint8_t NetClient_GetPlayerId(void) { return sMyPlayerId; }
struct RemotePlayer *NetClient_GetRemotePlayers(void) { return sRemotePlayers; }
int NetClient_GetChatCount(void) { return sChatCount; }
struct ChatEntry *NetClient_GetChatHistory(void) { return sChatHistory; }

uint8_t NetClient_GetMyTrainerSprite(void) { return sMyTrainerSprite; }
const char *NetClient_GetMyUsername(void) { return sMyUsername; }

int NetClient_PopNewChat(char *userOut, int userMax, char *msgOut, int msgMax)
{
    if (!sHasNewChat) return 0;
    sHasNewChat = 0;
    strncpy(userOut, sNewChatUser, userMax - 1);
    userOut[userMax - 1] = '\0';
    strncpy(msgOut, sNewChatMsg, msgMax - 1);
    msgOut[msgMax - 1] = '\0';
    return 1;
}

int NetClient_NeedsInitialPosition(void)
{
    if (sNeedsInitialPosition)
    {
        sNeedsInitialPosition = 0;
        return 1;
    }
    return 0;
}

void NetClient_TickChat(void)
{
    for (int i = 0; i < sChatCount; i++)
    {
        if (sChatHistory[i].timer > 0)
            sChatHistory[i].timer--;
    }
    while (sChatCount > 0 && sChatHistory[0].timer <= 0)
    {
        for (int i = 0; i < sChatCount - 1; i++)
            sChatHistory[i] = sChatHistory[i + 1];
        sChatCount--;
    }
}

void NetClient_SendBattleRequest(uint8_t targetPlayerId)
{
    if (!sConnected) return;
    struct InteractPacket pkt = {
        .header = { .type = MSG_C_BATTLE_REQUEST, .size = sizeof(struct InteractPacket) },
        .fromPlayerId = sMyPlayerId,
        .toPlayerId = targetPlayerId,
        .interactType = MSG_C_BATTLE_REQUEST
    };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
    sInteractState = NET_INTERACT_REQUESTING;
    sInteractPartnerId = targetPlayerId;
    sInteractType = MSG_C_BATTLE_REQUEST;
}

void NetClient_SendTradeRequest(uint8_t targetPlayerId)
{
    if (!sConnected) return;
    struct InteractPacket pkt = {
        .header = { .type = MSG_C_TRADE_REQUEST, .size = sizeof(struct InteractPacket) },
        .fromPlayerId = sMyPlayerId,
        .toPlayerId = targetPlayerId,
        .interactType = MSG_C_TRADE_REQUEST
    };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
    sInteractState = NET_INTERACT_REQUESTING;
    sInteractPartnerId = targetPlayerId;
    sInteractType = MSG_C_TRADE_REQUEST;
}

void NetClient_SendInteractResponse(uint8_t targetPlayerId, uint8_t accepted, uint8_t type)
{
    if (!sConnected) return;
    if (sInteractState != NET_INTERACT_REQUESTED) return;
    uint8_t msgType = (type == MSG_C_BATTLE_REQUEST) ? MSG_C_BATTLE_ACCEPT : MSG_C_TRADE_ACCEPT;
    struct InteractResponsePacket pkt = {
        .header = { .type = msgType, .size = sizeof(struct InteractResponsePacket) },
        .fromPlayerId = sMyPlayerId,
        .toPlayerId = targetPlayerId,
        .accepted = accepted
    };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
    if (!accepted)
    {
        sInteractState = NET_INTERACT_NONE;
        sInteractPartnerId = -1;
    }
    else
    {
        sInteractState = NET_INTERACT_REQUESTING;
    }
}

static int sPartySent = 0;
void NetClient_SendPartyData(const void *partyData, int partyCount)
{
    if (!sConnected) return;
    if (sPartySent) return;
    sPartySent = 1;
    int monSize = 100;
    for (int chunk = 0; chunk < 3; chunk++)
    {
        int startMon = chunk * 2;
        if (startMon >= partyCount) break;
        struct PartyDataPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.header.type = MSG_C_PARTY_DATA;
        pkt.header.size = sizeof(struct PartyDataPacket);
        pkt.fromPlayerId = sMyPlayerId;
        pkt.chunkIndex = chunk;
        pkt.totalPartyCount = partyCount;
        int count = (startMon + 2 <= partyCount) ? 2 : 1;
        memcpy(pkt.monData, (const uint8_t *)partyData + startMon * monSize, count * monSize);
        send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
    }
    sReceivedChunks = 0;
    sReceivedPartyCount = 0;
}

void NetClient_SendTradeSelection(uint8_t slot)
{
    if (!sConnected) return;
    struct TradeSelectPacket pkt = {
        .header = { .type = MSG_C_TRADE_SELECT, .size = sizeof(struct TradeSelectPacket) },
        .fromPlayerId = sMyPlayerId,
        .selectedSlot = slot
    };
    send(sSocket, (const char *)&pkt, sizeof(pkt), 0);
    sMyTradeSlot = slot;
    if (sPartnerTradeSlot >= 0)
        sInteractState = NET_INTERACT_TRADE_READY;
    else
        sInteractState = NET_INTERACT_TRADE_WAIT;
}

int NetClient_GetInteractState(void) { return sInteractState; }
int NetClient_GetInteractPartnerId(void) { return sInteractPartnerId; }
int NetClient_GetInteractType(void) { return sInteractType; }

const char *NetClient_GetInteractPartnerName(void)
{
    if (sInteractPartnerId >= 0 && sInteractPartnerId < MAX_PLAYERS
        && sRemotePlayers[sInteractPartnerId].active)
        return sRemotePlayers[sInteractPartnerId].username;
    return "???";
}

void *NetClient_GetReceivedParty(void) { return sReceivedPartyData; }
int NetClient_GetReceivedPartyCount(void) { return sReceivedPartyCount; }
int NetClient_GetPartnerTradeSlot(void) { return sPartnerTradeSlot; }

int NetClient_SendRaw(const void *data, int size)
{
    if (!sConnected) return 0;
    return send(sSocket, (const char *)data, size, 0);
}

void NetClient_ClearInteraction(void)
{
    sInteractState = NET_INTERACT_NONE;
    sInteractPartnerId = -1;
    sInteractType = 0;
    sReceivedChunks = 0;
    sReceivedPartyCount = 0;
    sMyTradeSlot = -1;
    sPartnerTradeSlot = -1;
    sPartySent = 0;
}
