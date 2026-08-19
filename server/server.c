#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
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
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

struct Player {
    uint8_t id;
    SOCKET sock;
    char username[MAX_USERNAME_LEN];
    uint8_t trainerSprite;
    uint16_t mapGroup;
    uint16_t mapNum;
    int16_t x;
    int16_t y;
    uint8_t direction;
    int active;
    int interactPartner;
};

static struct Player sPlayers[MAX_PLAYERS];
static int sPlayerCount = 0;

struct ServerConfig {
    char localip[64];
    int hostport;
    char motd[256];
    char server_name[64];
    int max_players;
    int shiny_rate;
};

static struct ServerConfig sConfig = {
    .localip = "0.0.0.0",
    .hostport = DEFAULT_PORT,
    .motd = "",
    .server_name = "PokeRecomp Server",
    .max_players = MAX_PLAYERS,
    .shiny_rate = 4096
};

static void TrimWhitespace(char *str)
{
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        *end-- = '\0';
    char *start = str;
    while (*start == ' ' || *start == '\t')
        start++;
    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

static void LoadConfig(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("[CONFIG] No config file found at '%s', using defaults\n", path);
        return;
    }

    printf("[CONFIG] Loading %s\n", path);
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        TrimWhitespace(key);
        TrimWhitespace(val);

        if (strcmp(key, "localip") == 0)
            strncpy(sConfig.localip, val, sizeof(sConfig.localip) - 1);
        else if (strcmp(key, "hostport") == 0)
            sConfig.hostport = atoi(val);
        else if (strcmp(key, "motd") == 0)
            strncpy(sConfig.motd, val, sizeof(sConfig.motd) - 1);
        else if (strcmp(key, "server-name") == 0)
            strncpy(sConfig.server_name, val, sizeof(sConfig.server_name) - 1);
        else if (strcmp(key, "max-players") == 0)
        {
            int n = atoi(val);
            if (n > 0)
                sConfig.max_players = n;
        }
        else if (strcmp(key, "shiny-rate") == 0)
            sConfig.shiny_rate = atoi(val);
        else
            printf("[CONFIG] Unknown key: '%s'\n", key);
    }
    fclose(f);
}

static void SendTo(int playerId, const void *data, int size);

static void EnsureDir(const char *dir)
{
#ifdef _WIN32
    CreateDirectoryA(dir, NULL);
#else
    mkdir(dir, 0755);
#endif
}

// Account file format: [32 bytes password hash][save data...]
// Returns: 0 = existing account + password match, 1 = bad password, 2 = new account
static int AuthenticateUser(const char *username, const uint8_t *hash)
{
    EnsureDir("accounts");
    char path[256];
    snprintf(path, sizeof(path), "accounts/%s.dat", username);

    FILE *f = fopen(path, "rb");
    if (!f)
    {
        f = fopen(path, "wb");
        if (f) { fwrite(hash, 1, PASSWORD_HASH_LEN, f); fclose(f); }
        printf("[AUTH] New account created: %s\n", username);
        return LOGIN_NEW_ACCOUNT;
    }

    uint8_t stored[PASSWORD_HASH_LEN];
    size_t bytesRead = fread(stored, 1, PASSWORD_HASH_LEN, f);
    fclose(f);
    if (bytesRead != PASSWORD_HASH_LEN || memcmp(stored, hash, PASSWORD_HASH_LEN) != 0)
    {
        printf("[AUTH] Bad password for: %s\n", username);
        return LOGIN_BAD_PASSWORD;
    }

    printf("[AUTH] Login OK: %s\n", username);
    return LOGIN_OK;
}

static int LoadUserSave(const char *username, uint8_t *buf, int bufSize)
{
    char path[256];
    snprintf(path, sizeof(path), "accounts/%s.dat", username);

    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    fseek(f, PASSWORD_HASH_LEN, SEEK_SET);
    int bytesRead = fread(buf, 1, bufSize, f);
    fclose(f);
    return bytesRead;
}

static void StoreUserSave(const char *username, const uint8_t *buf, int size)
{
    char path[256];
    snprintf(path, sizeof(path), "accounts/%s.dat", username);

    // Read existing hash
    uint8_t hash[PASSWORD_HASH_LEN] = {0};
    FILE *f = fopen(path, "rb");
    if (f) { fread(hash, 1, PASSWORD_HASH_LEN, f); fclose(f); }

    f = fopen(path, "wb");
    if (f) { fwrite(hash, 1, PASSWORD_HASH_LEN, f); fwrite(buf, 1, size, f); fclose(f); }
}

static void SendSaveToClient(int playerId, const uint8_t *saveData, int saveSize)
{
    int totalChunks = (SAVE_TOTAL_SIZE + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
    for (int i = 0; i < totalChunks; i++)
    {
        struct SaveDataPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.header.type = MSG_S_SAVE_DATA;
        pkt.chunkIndex = i;
        pkt.totalChunks = totalChunks;

        int offset = i * SAVE_CHUNK_SIZE;
        int remaining = saveSize - offset;
        int chunkSize = remaining < SAVE_CHUNK_SIZE ? remaining : SAVE_CHUNK_SIZE;
        if (chunkSize < 0) chunkSize = 0;

        pkt.dataSize = chunkSize;
        if (chunkSize > 0)
            memcpy(pkt.data, &saveData[offset], chunkSize);
        pkt.header.size = sizeof(struct PacketHeader) + 2 + 2 + 2 + chunkSize;
        SendTo(playerId, &pkt, pkt.header.size);
    }
}

static void SetNonBlocking(SOCKET sock)
{
#ifdef _WIN32
    unsigned long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

static int FindFreeSlot(void)
{
    int limit = sConfig.max_players < MAX_PLAYERS ? sConfig.max_players : MAX_PLAYERS;
    for (int i = 0; i < limit; i++)
        if (!sPlayers[i].active)
            return i;
    return -1;
}

static void BroadcastExcept(int exceptId, const void *data, int size)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (sPlayers[i].active && sPlayers[i].id != exceptId)
            send(sPlayers[i].sock, (const char *)data, size, 0);
    }
}

static void BroadcastAll(const void *data, int size)
{
    BroadcastExcept(-1, data, size);
}

static void SendTo(int playerId, const void *data, int size)
{
    if (sPlayers[playerId].active)
        send(sPlayers[playerId].sock, (const char *)data, size, 0);
}

static void HandleConnect(SOCKET clientSock, struct ConnectPacket *pkt)
{
    // Authenticate first
    char username[MAX_USERNAME_LEN];
    strncpy(username, pkt->username, MAX_USERNAME_LEN - 1);
    username[MAX_USERNAME_LEN - 1] = '\0';

    int authResult = AuthenticateUser(username, pkt->passwordHash);
    if (authResult == LOGIN_BAD_PASSWORD)
    {
        struct LoginResultPacket lr = {
            .header = { .type = MSG_S_LOGIN_RESULT, .size = sizeof(struct LoginResultPacket) },
            .result = LOGIN_BAD_PASSWORD, .playerId = 0, .playerCount = 0
        };
        send(clientSock, (const char *)&lr, sizeof(lr), 0);
        closesocket(clientSock);
        return;
    }

    int slot = FindFreeSlot();
    if (slot < 0)
    {
        closesocket(clientSock);
        printf("[SERVER] Rejected connection: server full\n");
        return;
    }

    sPlayers[slot].id = slot;
    sPlayers[slot].sock = clientSock;
    sPlayers[slot].active = 1;
    sPlayers[slot].trainerSprite = pkt->trainerSprite;
    sPlayers[slot].mapGroup = pkt->mapGroup;
    sPlayers[slot].mapNum = pkt->mapNum;
    sPlayers[slot].x = pkt->x;
    sPlayers[slot].y = pkt->y;
    sPlayers[slot].direction = pkt->direction;
    sPlayers[slot].interactPartner = -1;
    strncpy(sPlayers[slot].username, username, MAX_USERNAME_LEN - 1);
    sPlayers[slot].username[MAX_USERNAME_LEN - 1] = '\0';
    sPlayerCount++;

    printf("[SERVER] %s connected (id=%d, %d/%d players)\n",
        username, slot, sPlayerCount, sConfig.max_players);

    // Send login result
    struct LoginResultPacket lr = {
        .header = { .type = MSG_S_LOGIN_RESULT, .size = sizeof(struct LoginResultPacket) },
        .result = authResult, .playerId = slot, .playerCount = sPlayerCount
    };
    SendTo(slot, &lr, sizeof(lr));

    // Send save data if existing account
    if (authResult == LOGIN_OK)
    {
        uint8_t *saveBuffer = (uint8_t *)calloc(1, SAVE_TOTAL_SIZE);
        int saveSize = LoadUserSave(username, saveBuffer, SAVE_TOTAL_SIZE);
        printf("[SERVER] Save load for %s: %d bytes, first4=[%02x %02x %02x %02x]\n",
            username, saveSize, saveBuffer[0], saveBuffer[1], saveBuffer[2], saveBuffer[3]);
        if (saveSize > 0)
            SendSaveToClient(slot, saveBuffer, saveSize);
        else
            SendSaveToClient(slot, saveBuffer, SAVE_TOTAL_SIZE);
        free(saveBuffer);
    }
    else
    {
        printf("[SERVER] New account, no save to send\n");
    }

    // Send existing players to new player
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (sPlayers[i].active && i != slot)
        {
            struct PlayerJoinPacket join = {
                .header = { .type = MSG_S_PLAYER_JOIN, .size = sizeof(struct PlayerJoinPacket) },
                .playerId = i,
                .trainerSprite = sPlayers[i].trainerSprite,
                .mapGroup = sPlayers[i].mapGroup,
                .mapNum = sPlayers[i].mapNum,
                .x = sPlayers[i].x,
                .y = sPlayers[i].y,
                .direction = sPlayers[i].direction
            };
            strncpy(join.username, sPlayers[i].username, MAX_USERNAME_LEN);
            SendTo(slot, &join, sizeof(join));
        }
    }

    // Broadcast new player to everyone else
    struct PlayerJoinPacket join = {
        .header = { .type = MSG_S_PLAYER_JOIN, .size = sizeof(struct PlayerJoinPacket) },
        .playerId = slot,
        .trainerSprite = sPlayers[slot].trainerSprite,
        .mapGroup = sPlayers[slot].mapGroup,
        .mapNum = sPlayers[slot].mapNum,
        .x = sPlayers[slot].x,
        .y = sPlayers[slot].y,
        .direction = sPlayers[slot].direction
    };
    strncpy(join.username, sPlayers[slot].username, MAX_USERNAME_LEN);
    BroadcastExcept(slot, &join, sizeof(join));

    // Send server config
    struct ServerConfigDataPacket cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.header.type = MSG_S_SERVER_CONFIG;
    cfg.header.size = sizeof(cfg);
    cfg.shinyRate = sConfig.shiny_rate;
    SendTo(slot, &cfg, sizeof(cfg));

    if (sConfig.motd[0])
    {
        struct ServerMsgPacket msg;
        memset(&msg, 0, sizeof(msg));
        msg.header.type = MSG_S_SERVER_MSG;
        msg.header.size = sizeof(msg);
        strncpy(msg.message, sConfig.motd, MAX_CHAT_LEN - 1);
        SendTo(slot, &msg, sizeof(msg));
    }
}

static void HandleDisconnect(int playerId)
{
    if (!sPlayers[playerId].active) return;

    printf("[SERVER] %s disconnected (id=%d)\n", sPlayers[playerId].username, playerId);

    int partner = sPlayers[playerId].interactPartner;
    if (partner >= 0 && partner < MAX_PLAYERS && sPlayers[partner].active)
    {
        sPlayers[partner].interactPartner = -1;
        struct InteractPacket cancel;
        cancel.header.type = MSG_S_INTERACT_CANCEL;
        cancel.header.size = sizeof(struct InteractPacket);
        cancel.fromPlayerId = playerId;
        cancel.toPlayerId = partner;
        cancel.interactType = 0;
        SendTo(partner, &cancel, sizeof(cancel));
    }
    closesocket(sPlayers[playerId].sock);
    sPlayers[playerId].active = 0;
    sPlayers[playerId].interactPartner = -1;
    sPlayerCount--;

    struct PlayerLeavePacket leave = {
        .header = { .type = MSG_S_PLAYER_LEAVE, .size = sizeof(struct PlayerLeavePacket) },
        .playerId = playerId
    };
    BroadcastAll(&leave, sizeof(leave));
}

static void HandlePosition(int playerId, struct PositionPacket *pkt)
{
    sPlayers[playerId].mapGroup = pkt->mapGroup;
    sPlayers[playerId].mapNum = pkt->mapNum;
    sPlayers[playerId].x = pkt->x;
    sPlayers[playerId].y = pkt->y;
    sPlayers[playerId].direction = pkt->direction;

    pkt->header.type = MSG_S_POSITION;
    pkt->playerId = playerId;
    BroadcastExcept(playerId, pkt, sizeof(struct PositionPacket));
}

static void HandleChat(int playerId, struct ChatPacket *pkt)
{
    pkt->header.type = MSG_S_CHAT;
    pkt->playerId = playerId;
    strncpy(pkt->username, sPlayers[playerId].username, MAX_USERNAME_LEN);
    printf("[CHAT] %s: %s\n", pkt->username, pkt->message);
    BroadcastAll(pkt, sizeof(struct ChatPacket));
}

static void SendServerMsg(int playerId, const char *msg)
{
    struct ServerMsgPacket pkt;
    pkt.header.type = MSG_S_SERVER_MSG;
    pkt.header.size = sizeof(struct ServerMsgPacket);
    strncpy(pkt.message, msg, MAX_CHAT_LEN - 1);
    pkt.message[MAX_CHAT_LEN - 1] = '\0';
    SendTo(playerId, &pkt, sizeof(pkt));
}

static void HandleTradeRequest(int fromId, struct InteractPacket *pkt)
{
    int toId = pkt->toPlayerId;
    if (toId >= MAX_PLAYERS || !sPlayers[toId].active)
    {
        SendServerMsg(fromId, "That player is not online.");
        return;
    }
    printf("[SERVER] %s wants to trade with %s\n", sPlayers[fromId].username, sPlayers[toId].username);

    struct InteractPacket fwd;
    fwd.header.type = MSG_S_TRADE_REQUEST;
    fwd.header.size = sizeof(struct InteractPacket);
    fwd.fromPlayerId = fromId;
    fwd.toPlayerId = toId;
    fwd.interactType = MSG_C_TRADE_REQUEST;
    SendTo(toId, &fwd, sizeof(fwd));
}

static void HandleBattleRequest(int fromId, struct InteractPacket *pkt)
{
    int toId = pkt->toPlayerId;
    if (toId >= MAX_PLAYERS || !sPlayers[toId].active)
    {
        SendServerMsg(fromId, "That player is not online.");
        return;
    }
    printf("[SERVER] %s wants to battle %s\n", sPlayers[fromId].username, sPlayers[toId].username);

    struct InteractPacket fwd;
    fwd.header.type = MSG_S_BATTLE_REQUEST;
    fwd.header.size = sizeof(struct InteractPacket);
    fwd.fromPlayerId = fromId;
    fwd.toPlayerId = toId;
    fwd.interactType = MSG_C_BATTLE_REQUEST;
    SendTo(toId, &fwd, sizeof(fwd));
}

static void HandleTradeAccept(int fromId, struct InteractResponsePacket *pkt)
{
    int toId = pkt->toPlayerId;
    if (toId >= MAX_PLAYERS || !sPlayers[toId].active) return;

    if (pkt->accepted)
    {
        if (sPlayers[fromId].interactPartner >= 0) return;
        printf("[SERVER] Trade accepted between %s and %s\n", sPlayers[fromId].username, sPlayers[toId].username);
        sPlayers[fromId].interactPartner = toId;
        sPlayers[toId].interactPartner = fromId;
        struct InteractResponsePacket resp;
        resp.header.type = MSG_S_TRADE_START;
        resp.header.size = sizeof(struct InteractResponsePacket);
        resp.fromPlayerId = fromId;
        resp.toPlayerId = toId;
        resp.accepted = 1;
        SendTo(fromId, &resp, sizeof(resp));
        SendTo(toId, &resp, sizeof(resp));
    }
    else
    {
        struct InteractPacket cancel;
        cancel.header.type = MSG_S_INTERACT_CANCEL;
        cancel.header.size = sizeof(struct InteractPacket);
        cancel.fromPlayerId = fromId;
        cancel.toPlayerId = toId;
        cancel.interactType = MSG_C_TRADE_REQUEST;
        SendTo(toId, &cancel, sizeof(cancel));
    }
}

static void HandleBattleAccept(int fromId, struct InteractResponsePacket *pkt)
{
    int toId = pkt->toPlayerId;
    if (toId >= MAX_PLAYERS || !sPlayers[toId].active) return;

    if (pkt->accepted)
    {
        if (sPlayers[fromId].interactPartner >= 0) return;
        printf("[SERVER] Battle accepted between %s and %s\n", sPlayers[fromId].username, sPlayers[toId].username);
        sPlayers[fromId].interactPartner = toId;
        sPlayers[toId].interactPartner = fromId;
        struct InteractResponsePacket resp;
        resp.header.type = MSG_S_BATTLE_START;
        resp.header.size = sizeof(struct InteractResponsePacket);
        resp.fromPlayerId = fromId;
        resp.toPlayerId = toId;
        resp.accepted = 1;
        SendTo(fromId, &resp, sizeof(resp));
        SendTo(toId, &resp, sizeof(resp));
    }
    else
    {
        struct InteractPacket cancel;
        cancel.header.type = MSG_S_INTERACT_CANCEL;
        cancel.header.size = sizeof(struct InteractPacket);
        cancel.fromPlayerId = fromId;
        cancel.toPlayerId = toId;
        cancel.interactType = MSG_C_BATTLE_REQUEST;
        SendTo(toId, &cancel, sizeof(cancel));
    }
}

static void HandlePartyData(int fromId, struct PartyDataPacket *pkt)
{
    int partner = sPlayers[fromId].interactPartner;
    if (partner < 0 || partner >= MAX_PLAYERS || !sPlayers[partner].active) return;
    pkt->header.type = MSG_S_PARTY_DATA;
    pkt->fromPlayerId = fromId;
    SendTo(partner, pkt, pkt->header.size);
}

static void HandleTradeSelect(int fromId, struct TradeSelectPacket *pkt)
{
    int partner = sPlayers[fromId].interactPartner;
    if (partner < 0 || partner >= MAX_PLAYERS || !sPlayers[partner].active) return;
    pkt->header.type = MSG_S_TRADE_SELECT;
    pkt->fromPlayerId = fromId;
    SendTo(partner, pkt, sizeof(struct TradeSelectPacket));
}

static void HandleLinkData(int fromId, struct LinkDataPacket *pkt)
{
    int partner = sPlayers[fromId].interactPartner;
    printf("[SERVER-LINK] from=%d partner=%d size=%d\n", fromId, partner, pkt->dataSize);
    fflush(stdout);
    if (partner < 0 || partner >= MAX_PLAYERS || !sPlayers[partner].active) return;
    pkt->header.type = MSG_S_LINK_DATA;
    pkt->fromPlayerId = fromId;
    SendTo(partner, pkt, sizeof(struct PacketHeader) + 1 + 2 + pkt->dataSize);
}

static void ProcessPlayerData(int playerId)
{
    static char buf[256 * 1024];
    int received = recv(sPlayers[playerId].sock, buf, sizeof(buf), 0);

    if (received == 0)
    {
        // Connection closed gracefully
        HandleDisconnect(playerId);
        return;
    }
    if (received < 0)
    {
        // WOULDBLOCK = no data, not an error
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return;
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
#endif
        HandleDisconnect(playerId);
        return;
    }

    int offset = 0;
    while (offset < received)
    {
        if (received - offset < (int)sizeof(struct PacketHeader))
            break;

        struct PacketHeader *header = (struct PacketHeader *)(buf + offset);

        if (offset + header->size > received)
            break;

        if (header->type != MSG_C_POSITION)
        {
            printf("[SERVER] Player %d sent type=0x%02x size=%d\n", playerId, header->type, header->size);
            fflush(stdout);
        }

        switch (header->type)
        {
        case MSG_C_POSITION:
            HandlePosition(playerId, (struct PositionPacket *)(buf + offset));
            break;
        case MSG_C_CHAT:
            HandleChat(playerId, (struct ChatPacket *)(buf + offset));
            break;
        case MSG_C_DISCONNECT:
            HandleDisconnect(playerId);
            return;
        case MSG_C_TRADE_REQUEST:
            HandleTradeRequest(playerId, (struct InteractPacket *)(buf + offset));
            break;
        case MSG_C_BATTLE_REQUEST:
            HandleBattleRequest(playerId, (struct InteractPacket *)(buf + offset));
            break;
        case MSG_C_TRADE_ACCEPT:
            HandleTradeAccept(playerId, (struct InteractResponsePacket *)(buf + offset));
            break;
        case MSG_C_BATTLE_ACCEPT:
            HandleBattleAccept(playerId, (struct InteractResponsePacket *)(buf + offset));
            break;
        case MSG_C_PARTY_DATA:
            HandlePartyData(playerId, (struct PartyDataPacket *)(buf + offset));
            break;
        case MSG_C_TRADE_SELECT:
            HandleTradeSelect(playerId, (struct TradeSelectPacket *)(buf + offset));
            break;
        case MSG_C_LINK_DATA:
            HandleLinkData(playerId, (struct LinkDataPacket *)(buf + offset));
            break;
        case MSG_C_SAVE_DATA:
        {
            struct SaveDataPacket *sp = (struct SaveDataPacket *)(buf + offset);
            int saveOffset = sp->chunkIndex * SAVE_CHUNK_SIZE;
            if (saveOffset + sp->dataSize <= SAVE_TOTAL_SIZE)
            {
                static uint8_t sSaveBuf[MAX_PLAYERS][SAVE_TOTAL_SIZE];
                memcpy(&sSaveBuf[playerId][saveOffset], sp->data, sp->dataSize);
                if (sp->chunkIndex + 1 >= sp->totalChunks)
                {
                    StoreUserSave(sPlayers[playerId].username, sSaveBuf[playerId], SAVE_TOTAL_SIZE);
                    printf("[SERVER] Save stored for %s\n", sPlayers[playerId].username);
                }
            }
            break;
        }
        }

        offset += header->size;
    }
}

int main(int argc, char **argv)
{
    const char *configPath = "server.cfg";
    if (argc > 1) configPath = argv[1];
    LoadConfig(configPath);

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    memset(sPlayers, 0, sizeof(sPlayers));

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET)
    {
        printf("[SERVER] Failed to create socket\n");
        return 1;
    }

    int opt = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(sConfig.localip);
    addr.sin_port = htons(sConfig.hostport);

    if (bind(listenSock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("[SERVER] Failed to bind to %s:%d\n", sConfig.localip, sConfig.hostport);
        closesocket(listenSock);
        return 1;
    }

    listen(listenSock, 8);
    SetNonBlocking(listenSock);

    printf("=== %s (v%d) ===\n", sConfig.server_name, PROTOCOL_VERSION);
    printf("Listening on %s:%d\n", sConfig.localip, sConfig.hostport);
    printf("Max players: %d\n", sConfig.max_players);
    printf("Shiny rate: 1/%d\n\n", sConfig.shiny_rate);

    while (1)
    {
        // Accept new connections
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(listenSock, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientSock != INVALID_SOCKET)
        {
            { int one = 1; setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one)); }
            // Keep blocking to receive connect packet and send save data
            char buf[256];
            int received = recv(clientSock, buf, sizeof(buf), 0);

            if (received >= (int)sizeof(struct ConnectPacket))
            {
                struct ConnectPacket *pkt = (struct ConnectPacket *)buf;
                if (pkt->header.type == MSG_C_CONNECT && pkt->protocolVersion == PROTOCOL_VERSION)
                {
                    HandleConnect(clientSock, pkt);
                    SetNonBlocking(clientSock);
                }
                else
                {
                    printf("[SERVER] Invalid connect packet\n");
                    closesocket(clientSock);
                }
            }
            else
            {
                closesocket(clientSock);
            }
        }

        // Process existing players
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (sPlayers[i].active)
                ProcessPlayerData(i);
        }

#ifdef _WIN32
        Sleep(16); // ~60 tick rate
#else
        usleep(16000);
#endif
    }

    closesocket(listenSock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
