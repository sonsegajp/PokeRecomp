#ifndef POKERECOMP_PROTOCOL_H
#define POKERECOMP_PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_VERSION 1
#define MAX_USERNAME_LEN 16
#define MAX_CHAT_LEN 128
#define MAX_PLAYERS 64
#define DEFAULT_PORT 27015

// Message types (client → server)
#define MSG_C_CONNECT       0x01
#define MSG_C_DISCONNECT    0x02
#define MSG_C_POSITION      0x03
#define MSG_C_CHAT          0x04
#define MSG_C_INTERACT      0x05
#define MSG_C_TRADE_REQUEST 0x06
#define MSG_C_BATTLE_REQUEST 0x07
#define MSG_C_TRADE_ACCEPT  0x08
#define MSG_C_BATTLE_ACCEPT 0x09
#define MSG_C_TRADE_CANCEL  0x0A
#define MSG_C_BATTLE_CANCEL 0x0B
#define MSG_C_PARTY_DATA    0x0C
#define MSG_C_TRADE_SELECT  0x0D
#define MSG_C_LINK_DATA     0x0E
#define MSG_C_SAVE_DATA     0x0F

// Message types (server → client)
#define MSG_S_WELCOME       0x81
#define MSG_S_PLAYER_JOIN   0x82
#define MSG_S_PLAYER_LEAVE  0x83
#define MSG_S_POSITION      0x84
#define MSG_S_CHAT          0x85
#define MSG_S_PLAYER_LIST   0x86
#define MSG_S_TRADE_REQUEST 0x87
#define MSG_S_BATTLE_REQUEST 0x88
#define MSG_S_TRADE_START   0x89
#define MSG_S_BATTLE_START  0x8A
#define MSG_S_INTERACT_CANCEL 0x8B
#define MSG_S_SERVER_MSG    0x8C
#define MSG_S_PARTY_DATA    0x8D
#define MSG_S_TRADE_SELECT  0x8E
#define MSG_S_LINK_DATA     0x8F
#define MSG_S_SERVER_CONFIG 0x90
#define MSG_S_SAVE_DATA     0x91
#define MSG_S_LOGIN_RESULT  0x92

#pragma pack(push, 1)

struct PacketHeader {
    uint8_t type;
    uint16_t size;
};

#define PASSWORD_HASH_LEN 32

struct ConnectPacket {
    struct PacketHeader header;
    uint8_t protocolVersion;
    char username[MAX_USERNAME_LEN];
    uint8_t trainerSprite;
    uint16_t mapGroup;
    uint16_t mapNum;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint8_t passwordHash[PASSWORD_HASH_LEN];
};

struct WelcomePacket {
    struct PacketHeader header;
    uint8_t playerId;
    uint8_t playerCount;
};

struct PositionPacket {
    struct PacketHeader header;
    uint8_t playerId;
    uint16_t mapGroup;
    uint16_t mapNum;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint8_t movementAction;
};

struct ChatPacket {
    struct PacketHeader header;
    uint8_t playerId;
    char username[MAX_USERNAME_LEN];
    char message[MAX_CHAT_LEN];
};

struct PlayerJoinPacket {
    struct PacketHeader header;
    uint8_t playerId;
    char username[MAX_USERNAME_LEN];
    uint8_t trainerSprite;
    uint16_t mapGroup;
    uint16_t mapNum;
    int16_t x;
    int16_t y;
    uint8_t direction;
};

struct PlayerLeavePacket {
    struct PacketHeader header;
    uint8_t playerId;
};

struct InteractPacket {
    struct PacketHeader header;
    uint8_t fromPlayerId;
    uint8_t toPlayerId;
    uint8_t interactType; // MSG_C_TRADE_REQUEST or MSG_C_BATTLE_REQUEST
};

struct InteractResponsePacket {
    struct PacketHeader header;
    uint8_t fromPlayerId;
    uint8_t toPlayerId;
    uint8_t accepted;
};

struct ServerMsgPacket {
    struct PacketHeader header;
    char message[MAX_CHAT_LEN];
};

struct PartyDataPacket {
    struct PacketHeader header;
    uint8_t fromPlayerId;
    uint8_t chunkIndex;
    uint8_t totalPartyCount;
    uint8_t monData[200];
};

struct TradeSelectPacket {
    struct PacketHeader header;
    uint8_t fromPlayerId;
    uint8_t selectedSlot;
};

struct ServerConfigDataPacket {
    struct PacketHeader header;
    uint16_t shinyRate;
};

#define LINK_BLOCK_MAX 256
struct LinkDataPacket {
    struct PacketHeader header;
    uint8_t fromPlayerId;
    uint16_t dataSize;
    uint8_t data[LINK_BLOCK_MAX];
};

#define LOGIN_OK           0
#define LOGIN_BAD_PASSWORD 1
#define LOGIN_NEW_ACCOUNT  2

struct LoginResultPacket {
    struct PacketHeader header;
    uint8_t result;
    uint8_t playerId;
    uint8_t playerCount;
};

#define SAVE_CHUNK_SIZE 4096
#define SAVE_TOTAL_SIZE 0x20000  // 128KB flash

struct SaveDataPacket {
    struct PacketHeader header;
    uint16_t chunkIndex;
    uint16_t totalChunks;
    uint16_t dataSize;
    uint8_t data[SAVE_CHUNK_SIZE];
};

#pragma pack(pop)

#endif // POKERECOMP_PROTOCOL_H
