/* pc_net.h — online play: master server login, game servers, the virtual
 * link cable. Plain C (no SDL/Winsock types) so game code can include it.
 *
 * All operations are asynchronous: begin one, then poll PcNetOpStatus()
 * each frame. PcNetPump() runs once per frame on the host side. */
#ifndef PC_NET_H
#define PC_NET_H

#include <stdint.h>

enum { PC_NET_OP_IDLE, PC_NET_OP_PENDING, PC_NET_OP_OK, PC_NET_OP_FAILED };
enum { PC_NET_KIND_SINGLE = 1, PC_NET_KIND_DOUBLE = 2, PC_NET_KIND_TRADE = 3 };
/* which game a trainer plays (sent on join; decides how others see them) */
enum { PC_NET_GAME_FIRERED, PC_NET_GAME_LEAFGREEN, PC_NET_GAME_EMERALD, PC_NET_GAME_RUBY, PC_NET_GAME_SAPPHIRE };
enum { PC_NET_REGION_KANTO, PC_NET_REGION_HOENN };
#if defined(EMERALD)
#define PC_NET_REGION PC_NET_REGION_HOENN
#else
#define PC_NET_REGION PC_NET_REGION_KANTO
#endif
#if defined(EMERALD)
#define PC_NET_GAME PC_NET_GAME_EMERALD
#elif defined(LEAFGREEN)
#define PC_NET_GAME PC_NET_GAME_LEAFGREEN
#else
#define PC_NET_GAME PC_NET_GAME_FIRERED
#endif

#define PC_NET_NAME_LEN    8     /* game name buffer, 0xFF-terminated */
#define PC_NET_MAX_SERVERS 16
#define PC_NET_MAX_PLAYERS 64

struct PcNetPlayer {
    uint8_t active;
    uint16_t id;
    uint8_t name[PC_NET_NAME_LEN];
    uint16_t tag;
    uint8_t gender;
    uint8_t game;               /* PC_NET_GAME_* */
    uint8_t mapGroup, mapNum;
    int16_t x, y;
    uint8_t facing, flags;
};

/* the server's world state (S_WORLD): the day's shiny-boost type and a
 * timed event such as a mass outbreak */
enum { PC_NET_EVENT_NONE, PC_NET_EVENT_OUTBREAK };
struct PcNetWorld {
    uint8_t boostType;          /* TYPE_*, 0xFF = none */
    uint16_t shinyOdds;         /* 1 in N for the boosted type */
    uint8_t eventKind;
    uint8_t mapGroup, mapNum;
    uint16_t species;
    uint8_t minLevel, maxLevel;
    uint8_t percent;            /* share of wild encounters there */
};

void PcNetPump(void);

/* account (credentials live in online.dat next to the save) */
int PcNetHasAccount(void);
void PcNetBeginRegister(const uint8_t *name);   /* registers, then logs in */
void PcNetBeginLogin(void);                     /* logs in + fetches the server list */
void PcNetBeginRename(const uint8_t *name);     /* new online name (new tag), then the server list */

/* Global trade (Wonder Trade / GTS / mailbox) requests to the master: send
 * one request, then read the reply once PcNetOpStatus() is OK (or the error
 * code once it FAILED). Types/layouts: server/protocol.py M_WONDER..M_GTS_*. */
void PcNetMasterRequest(uint8_t type, const void *payload, int len);
int PcNetMasterReply(uint8_t *type, const uint8_t **payload);  /* returns payload length */
int PcNetMasterError(void);
int PcNetOpStatus(void);
const uint8_t *PcNetMyName(void);
uint16_t PcNetMyTag(void);

/* servers */
int PcNetServerCount(void);
const char *PcNetServerName(int i);
int PcNetServerPlayers(int i);
int PcNetServerRegion(int i);           /* PC_NET_REGION_* */
int PcNetOwnsRegion(int region);        /* this player can play there (owns that region's game) */
void PcNetBeginJoin(int index, uint8_t gender);
void PcNetDisconnect(void);
int PcNetConnected(void);
const char *PcNetConnectedServerName(void);

/* presence */
void PcNetSendPos(uint8_t group, uint8_t num, int16_t x, int16_t y, uint8_t facing, uint8_t flags);
void PcNetSendChat(const uint8_t *text, int len);   /* game charset, 0xFF-terminated */
void PcNetSendGuildChat(const uint8_t *text, int len);   /* to the guild, on every server */
const struct PcNetPlayer *PcNetGetPlayer(int slot);   /* slot 0..PC_NET_MAX_PLAYERS-1 */
const struct PcNetWorld *PcNetWorldState(void);       /* NULL when not on a server */

/* battle / trade requests */
void PcNetRequest(uint16_t target, uint8_t kind);
int PcNetRequestResult(void);          /* 0 waiting, 1 session started, -1 declined/failed */
int PcNetPollIncoming(uint16_t *from, uint8_t *kind);
void PcNetRespond(uint16_t from, uint8_t accept);

/* the virtual link cable */
int PcNetLinkActive(void);
int PcNetLinkId(void);
int PcNetLinkPlayers(void);
int PcNetLinkKind(void);
int PcNetLinkBroken(void);             /* partner dropped mid-session */
void PcNetLinkSend(const uint16_t *cmd);
int PcNetLinkRecv(uint16_t (*frame)[8]); /* fills [players][8]; 1 if a frame was taken */
void PcNetLinkEnd(void);
void PcNetLinkSetOpen(int open);        /* this game's end of the cable opened/closed */
int PcNetLinkConnected(void);           /* every member's end is open */
/* link.c reports link errors here (written to linkerr.log for diagnosis) */
void PcNetLogLinkError(uint32_t status, int state, int sendCount, int recvCount, int lag, int queueFull);

#endif
