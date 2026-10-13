/* net.c — online play client: master login, game servers, virtual link cable.
 *
 * Two TCP connections, both non-blocking and pumped once per frame from the
 * host loop (never from the game fiber, so game code only sees state):
 *   master — register / log in / server list (closed again after joining)
 *   game   — the joined world: presence, requests, link frames.
 * Wire format: see server/protocol.py (u16 length, u8 type, payload). */
#define _CRT_RAND_S
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "pc_net.h"
#include "pc_chat.h"

#define VERSION 1
#define SECRET_LEN 32
#define TOKEN_LEN 16
#define CONNECT_TIMEOUT_MS 6000
#define CREDENTIALS_PATH "online.dat"
#define CONFIG_PATH "online.ini"
#define DEFAULT_MASTER "127.0.0.1:27100"

enum {
    M_HELLO = 0x01, M_REGISTER = 0x02, M_LOGIN = 0x03, M_LIST_SERVERS = 0x04, M_RENAME = 0x07,
    M_REGISTERED = 0x81, M_LOGIN_OK = 0x82, M_SERVER_LIST = 0x83, M_RENAMED = 0x85, M_ERROR = 0x8F,
    S_JOIN = 0x40, S_POS = 0x41, S_CHAT = 0x42, S_REQUEST = 0x43, S_RESPOND = 0x44, S_LINK_CMD = 0x45, S_LINK_END = 0x46,
    S_WELCOME = 0xC0, S_PLAYER = 0xC1, S_PLAYER_GONE = 0xC2, S_CHAT_FROM = 0xC3, S_REQUESTED = 0xC4, S_DECLINED = 0xC5,
    S_SESSION = 0xC6, S_LINK_FRAME = 0xC7, S_SESSION_END = 0xC8, S_LINK_STATE = 0xC9, S_WORLD = 0xCA, S_GUILD_CHAT_FROM = 0xCB, S_ERROR = 0xCF,
    S_LINK_OPEN = 0x47,
};

struct Conn {
    SOCKET s;
    int connecting;
    ULONGLONG started;
    uint8_t in[65536];
    int in_len;
    uint8_t out[65536];
    int out_len;
};

static int wsa_ready;
static struct Conn master = {INVALID_SOCKET}, game = {INVALID_SOCKET};

/* account */
static int have_account;
static uint32_t account_id;
static uint8_t secret[SECRET_LEN];
static uint8_t my_name[PC_NET_NAME_LEN];
static uint16_t my_tag;
static uint8_t token[TOKEN_LEN];

/* operation in progress */
enum { OP_NONE, OP_REGISTER, OP_LOGIN, OP_JOIN, OP_RENAME, OP_REQUEST };
static int op, op_status = PC_NET_OP_IDLE;
static int master_logged_in;
static uint8_t rename_to[PC_NET_NAME_LEN];
static uint8_t request_type, request_data[256];
static int request_len;
static uint8_t reply_type, reply_data[4096];
static int reply_len, master_error;
static uint8_t join_gender;

/* servers */
static struct { char name[25], host[65]; uint16_t port, players, max; } servers[PC_NET_MAX_SERVERS];
static int server_count;
static int connected;
static uint16_t my_player_id;
static char connected_name[25];

/* world */
static struct PcNetPlayer players[PC_NET_MAX_PLAYERS];

/* requests */
static int request_result;
static int incoming;
static uint16_t incoming_from;
static uint8_t incoming_kind;

/* link */
static int link_active, link_id, link_players, link_kind, link_broken;
static int link_open, link_connected;
#define LINK_RING 128
static uint16_t link_ring[LINK_RING][4][8];
static int link_head, link_count;

/* ---- small helpers ----------------------------------------------------------- */

static void put16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; }
static void put32(uint8_t *p, uint32_t v) { put16(p, v); put16(p + 2, v >> 16); }
static uint16_t get16(const uint8_t *p) { return p[0] | p[1] << 8; }
static uint32_t get32(const uint8_t *p) { return get16(p) | (uint32_t)get16(p + 2) << 16; }

static void conn_close(struct Conn *c)
{
    if (c->s != INVALID_SOCKET) closesocket(c->s);
    c->s = INVALID_SOCKET;
    c->connecting = 0;
    c->in_len = c->out_len = 0;
}

static int conn_open(struct Conn *c, const char *host, int port)
{
    struct addrinfo hints = {0}, *res = NULL;
    char port_str[16];
    u_long nonblocking = 1;
    int nodelay = 1;

    if (!wsa_ready) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 0;
        wsa_ready = 1;
    }
    conn_close(c);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_str, sizeof port_str, "%d", port);
    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) return 0;
    c->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (c->s == INVALID_SOCKET) { freeaddrinfo(res); return 0; }
    ioctlsocket(c->s, FIONBIO, &nonblocking);
    setsockopt(c->s, IPPROTO_TCP, TCP_NODELAY, (const char *)&nodelay, sizeof nodelay);
    if (connect(c->s, res->ai_addr, (int)res->ai_addrlen) != 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
        freeaddrinfo(res);
        conn_close(c);
        return 0;
    }
    freeaddrinfo(res);
    c->connecting = 1;
    c->started = GetTickCount64();
    return 1;
}

static void conn_send(struct Conn *c, uint8_t type, const void *payload, int len)
{
    if (c->s == INVALID_SOCKET || c->out_len + len + 3 > (int)sizeof c->out) return;
    put16(c->out + c->out_len, (uint16_t)(len + 1));
    c->out[c->out_len + 2] = type;
    if (len) memcpy(c->out + c->out_len + 3, payload, len);
    c->out_len += len + 3;
}

/* Move bytes both ways. Returns 0 if the connection failed or closed. */
static int conn_io(struct Conn *c)
{
    if (c->s == INVALID_SOCKET) return 0;
    if (c->connecting) {
        fd_set w, e;
        struct timeval tv = {0, 0};
        FD_ZERO(&w); FD_ZERO(&e);
        FD_SET(c->s, &w); FD_SET(c->s, &e);
        if (select(0, NULL, &w, &e, &tv) > 0) {
            if (FD_ISSET(c->s, &e)) return 0;
            if (FD_ISSET(c->s, &w)) c->connecting = 0;
        }
        if (c->connecting)
            return GetTickCount64() - c->started < CONNECT_TIMEOUT_MS;
    }
    while (c->out_len > 0) {
        int n = send(c->s, (const char *)c->out, c->out_len, 0);
        if (n <= 0) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) break;
            return 0;
        }
        memmove(c->out, c->out + n, c->out_len - n);
        c->out_len -= n;
    }
    for (;;) {
        int room = (int)sizeof c->in - c->in_len;
        int n;
        if (room <= 0) return 0;
        n = recv(c->s, (char *)c->in + c->in_len, room, 0);
        if (n == 0) return 0;
        if (n < 0) return WSAGetLastError() == WSAEWOULDBLOCK;
        c->in_len += n;
    }
}

/* Next complete frame; *payload points into the buffer until conn_consume. */
static int conn_next(struct Conn *c, uint8_t *type, const uint8_t **payload, int *len)
{
    int size;
    if (c->in_len < 3) return 0;
    size = get16(c->in);
    if (size == 0) { c->in_len = 0; return 0; }
    if (c->in_len < size + 2) return 0;
    *type = c->in[2];
    *payload = c->in + 3;
    *len = size - 1;
    return size + 2;
}

static void conn_consume(struct Conn *c, int n)
{
    memmove(c->in, c->in + n, c->in_len - n);
    c->in_len -= n;
}

static void finish_op(int status)
{
    op = OP_NONE;
    op_status = status;
}

/* ---- credentials + config ------------------------------------------------------- */

static void load_credentials(void)
{
    FILE *f = fopen(CREDENTIALS_PATH, "r");
    char line[256];
    unsigned v;
    if (!f) return;
    while (fgets(line, sizeof line, f)) {
        if (sscanf(line, "id = %u", &v) == 1) account_id = v;
        else if (sscanf(line, "tag = %u", &v) == 1) my_tag = (uint16_t)v;
        else if (!strncmp(line, "secret = ", 9))
            for (int i = 0; i < SECRET_LEN && sscanf(line + 9 + i * 2, "%2x", &v) == 1; i++) secret[i] = (uint8_t)v;
        else if (!strncmp(line, "name = ", 7))
            for (int i = 0; i < PC_NET_NAME_LEN && sscanf(line + 7 + i * 2, "%2x", &v) == 1; i++) my_name[i] = (uint8_t)v;
    }
    fclose(f);
    have_account = account_id != 0;
}

static void save_credentials(void)
{
    FILE *f = fopen(CREDENTIALS_PATH, "w");
    if (!f) return;
    fprintf(f, "# FireRed PC online account. Keep this file: it is your login.\n");
    fprintf(f, "id = %u\ntag = %u\nsecret = ", account_id, my_tag);
    for (int i = 0; i < SECRET_LEN; i++) fprintf(f, "%02x", secret[i]);
    fprintf(f, "\nname = ");
    for (int i = 0; i < PC_NET_NAME_LEN; i++) fprintf(f, "%02x", my_name[i]);
    fprintf(f, "\n");
    fclose(f);
}

static void master_address(char *host, int size, int *port)
{
    FILE *f = fopen(CONFIG_PATH, "r");
    char line[256], addr[200] = DEFAULT_MASTER;
    if (f) {
        while (fgets(line, sizeof line, f))
            sscanf(line, " master = %199s", addr);
        fclose(f);
    }
    char *colon = strrchr(addr, ':');
    *port = colon ? atoi(colon + 1) : 27100;
    if (colon) *colon = 0;
    snprintf(host, size, "%s", addr);
}

static int open_master(void)
{
    char host[200];
    int port;
    uint8_t hello[2];
    master_address(host, sizeof host, &port);
    if (!conn_open(&master, host, port)) return 0;
    put16(hello, VERSION);
    conn_send(&master, M_HELLO, hello, 2);
    return 1;
}

static void send_login(void)
{
    uint8_t p[4 + SECRET_LEN];
    put32(p, account_id);
    memcpy(p + 4, secret, SECRET_LEN);
    conn_send(&master, M_LOGIN, p, sizeof p);
}

/* ---- API: account + servers ------------------------------------------------------------ */

int PcNetHasAccount(void)
{
    static int loaded;
    if (!loaded) { load_credentials(); loaded = 1; }
    return have_account;
}

void PcNetBeginRegister(const uint8_t *name)
{
    uint8_t p[PC_NET_NAME_LEN + SECRET_LEN];
    for (int i = 0; i < SECRET_LEN; i++) {
        unsigned r;
        rand_s(&r);
        secret[i] = (uint8_t)r;
    }
    memcpy(my_name, name, PC_NET_NAME_LEN);
    op = OP_REGISTER;
    op_status = PC_NET_OP_PENDING;
    if (!open_master()) { finish_op(PC_NET_OP_FAILED); return; }
    memcpy(p, my_name, PC_NET_NAME_LEN);
    memcpy(p + PC_NET_NAME_LEN, secret, SECRET_LEN);
    conn_send(&master, M_REGISTER, p, sizeof p);
}

void PcNetBeginLogin(void)
{
    op = OP_LOGIN;
    op_status = PC_NET_OP_PENDING;
    if (!PcNetHasAccount() || !open_master()) { finish_op(PC_NET_OP_FAILED); return; }
    send_login();
}

void PcNetBeginRename(const uint8_t *name)
{
    memcpy(rename_to, name, PC_NET_NAME_LEN);
    op = OP_RENAME;
    op_status = PC_NET_OP_PENDING;
    if (master.s != INVALID_SOCKET && master_logged_in) {
        conn_send(&master, M_RENAME, rename_to, PC_NET_NAME_LEN);
        return;
    }
    if (!PcNetHasAccount() || !open_master()) { finish_op(PC_NET_OP_FAILED); return; }
    send_login();
}

void PcNetMasterRequest(uint8_t type, const void *payload, int len)
{
    if (len > (int)sizeof request_data) len = sizeof request_data;
    request_type = type;
    memcpy(request_data, payload, len);
    request_len = len;
    master_error = 0;
    op = OP_REQUEST;
    op_status = PC_NET_OP_PENDING;
    if (master.s != INVALID_SOCKET && master_logged_in) {
        conn_send(&master, request_type, request_data, request_len);
        return;
    }
    if (!PcNetHasAccount() || !open_master()) { finish_op(PC_NET_OP_FAILED); return; }
    send_login();
}

int PcNetMasterReply(uint8_t *type, const uint8_t **payload)
{
    *type = reply_type;
    *payload = reply_data;
    return reply_len;
}

int PcNetMasterError(void) { return master_error; }

int PcNetOpStatus(void) { return op_status; }
const uint8_t *PcNetMyName(void) { return my_name; }
uint16_t PcNetMyTag(void) { return my_tag; }
int PcNetServerCount(void) { return server_count; }
const char *PcNetServerName(int i) { return i >= 0 && i < server_count ? servers[i].name : ""; }
int PcNetServerPlayers(int i) { return i >= 0 && i < server_count ? servers[i].players : 0; }
int PcNetConnected(void) { return connected; }
const char *PcNetConnectedServerName(void) { return connected_name; }

void PcNetBeginJoin(int index, uint8_t gender)
{
    if (index < 0 || index >= server_count) { op_status = PC_NET_OP_FAILED; return; }
    PcNetDisconnect();
    op = OP_JOIN;
    op_status = PC_NET_OP_PENDING;
    join_gender = gender;
    snprintf(connected_name, sizeof connected_name, "%s", servers[index].name);
    if (!conn_open(&game, servers[index].host, servers[index].port)) { finish_op(PC_NET_OP_FAILED); return; }
    uint8_t p[TOKEN_LEN + 3];
    memcpy(p, token, TOKEN_LEN);
    p[TOKEN_LEN] = gender;
    put16(p + TOKEN_LEN + 1, VERSION);
    conn_send(&game, S_JOIN, p, sizeof p);
}

static void reset_world(void)
{
    memset(players, 0, sizeof players);
    incoming = 0;
    request_result = 0;
    if (link_active) link_broken = 1;
    link_active = 0;
    link_count = 0;
}

void PcNetDisconnect(void)
{
    conn_close(&game);
    connected = 0;
    reset_world();
}

/* ---- API: presence + requests ------------------------------------------------------------ */

void PcNetSendPos(uint8_t group, uint8_t num, int16_t x, int16_t y, uint8_t facing, uint8_t flags)
{
    uint8_t p[8] = {group, num};
    if (!connected) return;
    put16(p + 2, (uint16_t)x);
    put16(p + 4, (uint16_t)y);
    p[6] = facing;
    p[7] = flags;
    conn_send(&game, S_POS, p, sizeof p);
}

void PcNetSendChat(const uint8_t *text, int len)
{
    if (!connected || len <= 0) return;
    if (len > 64) len = 64;
    conn_send(&game, S_CHAT, text, len);
}

void PcNetSendGuildChat(const uint8_t *text, int len)
{
    if (!connected || len <= 0) return;
    if (len > 64) len = 64;
    conn_send(&game, 0x48 /* S_GUILD_CHAT */, text, len);
}

const struct PcNetPlayer *PcNetGetPlayer(int slot)
{
    return slot >= 0 && slot < PC_NET_MAX_PLAYERS && players[slot].active ? &players[slot] : NULL;
}

void PcNetRequest(uint16_t target, uint8_t kind)
{
    uint8_t p[3];
    put16(p, target);
    p[2] = kind;
    request_result = 0;
    conn_send(&game, S_REQUEST, p, sizeof p);
}

int PcNetRequestResult(void) { return connected ? request_result : -1; }

int PcNetPollIncoming(uint16_t *from, uint8_t *kind)
{
    if (!incoming) return 0;
    incoming = 0;
    *from = incoming_from;
    *kind = incoming_kind;
    return 1;
}

void PcNetRespond(uint16_t from, uint8_t accept)
{
    uint8_t p[3];
    put16(p, from);
    p[2] = accept;
    request_result = 0;
    conn_send(&game, S_RESPOND, p, sizeof p);
}

/* ---- API: link cable ---------------------------------------------------------------- */

int PcNetLinkActive(void) { return link_active; }
int PcNetLinkId(void) { return link_id; }
int PcNetLinkPlayers(void) { return link_players; }
int PcNetLinkKind(void) { return link_kind; }
int PcNetLinkBroken(void) { return link_broken; }

void PcNetLinkSend(const uint16_t *cmd)
{
    uint8_t p[16];
    for (int i = 0; i < 8; i++) put16(p + i * 2, cmd[i]);
    conn_send(&game, S_LINK_CMD, p, sizeof p);
}

int PcNetLinkRecv(uint16_t (*frame)[8])
{
    if (!link_count) return 0;
    memcpy(frame, link_ring[link_head], sizeof link_ring[0]);
    link_head = (link_head + 1) % LINK_RING;
    link_count--;
    return 1;
}

void PcNetLinkSetOpen(int open)
{
    uint8_t p = (uint8_t)open;
    if (!link_active || link_open == open) return;
    link_open = open;
    link_count = 0;
    conn_send(&game, S_LINK_OPEN, &p, 1);
}

int PcNetLinkConnected(void) { return link_active && link_open && link_connected; }

void PcNetLinkEnd(void)
{
    if (link_active) conn_send(&game, S_LINK_END, NULL, 0);
    link_open = link_connected = 0;
    link_active = 0;
    link_broken = 0;
    link_count = 0;
}

/* ---- pump -------------------------------------------------------------------------- */

static struct PcNetPlayer *player_slot(uint16_t id, int create)
{
    struct PcNetPlayer *free_slot = NULL;
    for (int i = 0; i < PC_NET_MAX_PLAYERS; i++) {
        if (players[i].active && players[i].id == id) return &players[i];
        if (!players[i].active && !free_slot) free_slot = &players[i];
    }
    if (create && free_slot) {
        memset(free_slot, 0, sizeof *free_slot);
        free_slot->active = 1;
        free_slot->id = id;
        return free_slot;
    }
    return NULL;
}

static void handle_master(uint8_t type, const uint8_t *p, int len)
{
    switch (type) {
    case M_REGISTERED:
        if (len < 6) break;
        account_id = get32(p);
        my_tag = get16(p + 4);
        have_account = 1;
        save_credentials();
        send_login();
        break;
    case M_LOGIN_OK:
        if (len < TOKEN_LEN + PC_NET_NAME_LEN + 2) break;
        memcpy(token, p, TOKEN_LEN);
        memcpy(my_name, p + TOKEN_LEN, PC_NET_NAME_LEN);
        my_tag = get16(p + TOKEN_LEN + PC_NET_NAME_LEN);
        master_logged_in = 1;
        if (op == OP_RENAME)
            conn_send(&master, M_RENAME, rename_to, PC_NET_NAME_LEN);
        else if (op == OP_REQUEST)
            conn_send(&master, request_type, request_data, request_len);
        else
            conn_send(&master, M_LIST_SERVERS, NULL, 0);
        break;
    case M_RENAMED:
        if (len < PC_NET_NAME_LEN + 2) break;
        memcpy(my_name, p, PC_NET_NAME_LEN);
        my_tag = get16(p + PC_NET_NAME_LEN);
        save_credentials();
        conn_send(&master, M_LIST_SERVERS, NULL, 0);
        break;
    case M_SERVER_LIST: {
        int n = len ? p[0] : 0, off = 1;
        server_count = 0;
        for (int i = 0; i < n && server_count < PC_NET_MAX_SERVERS && off + 24 + 64 + 6 <= len; i++, off += 24 + 64 + 6) {
            memcpy(servers[server_count].name, p + off, 24);
            servers[server_count].name[24] = 0;
            memcpy(servers[server_count].host, p + off + 24, 64);
            servers[server_count].host[64] = 0;
            servers[server_count].port = get16(p + off + 88);
            servers[server_count].players = get16(p + off + 90);
            servers[server_count].max = get16(p + off + 92);
            server_count++;
        }
        if (op == OP_REGISTER || op == OP_LOGIN || op == OP_RENAME) finish_op(PC_NET_OP_OK);
        break;
    }
    case 0x84:                                                /* M_FRIENDS */
    case 0x86: case 0x87: case 0x88: case 0x89: case 0x8A:   /* M_WONDER_RESULT .. M_GTS_RESULT */
    case 0x8B:                                                /* M_GUILD */
        if (op == OP_REQUEST) {
            reply_type = type;
            reply_len = len > (int)sizeof reply_data ? (int)sizeof reply_data : len;
            memcpy(reply_data, p, reply_len);
            finish_op(PC_NET_OP_OK);
        }
        break;
    case M_ERROR:
        if (op == OP_REQUEST) master_error = len ? p[0] : 0;
        if (op == OP_REGISTER || op == OP_LOGIN || op == OP_RENAME || op == OP_REQUEST) finish_op(PC_NET_OP_FAILED);
        break;
    }
}

static struct PcNetWorld world;
static int have_world;

const struct PcNetWorld *PcNetWorldState(void)
{
    return connected && have_world ? &world : NULL;
}

static void handle_game(uint8_t type, const uint8_t *p, int len)
{
    struct PcNetPlayer *pl;
    switch (type) {
    case S_WELCOME:
        connected = 1;
        have_world = 0;
        if (len >= 2) my_player_id = get16(p);
        if (op == OP_JOIN) finish_op(PC_NET_OP_OK);
        conn_close(&master);           /* the master is only needed until we're in */
        master_logged_in = 0;
        break;
    case S_PLAYER:
        if (len < 2 + PC_NET_NAME_LEN + 3 + 8 || !(pl = player_slot(get16(p), 1))) break;
        memcpy(pl->name, p + 2, PC_NET_NAME_LEN);
        pl->tag = get16(p + 2 + PC_NET_NAME_LEN);
        pl->gender = p[2 + PC_NET_NAME_LEN + 2];
        p += 2 + PC_NET_NAME_LEN + 3;
        pl->mapGroup = p[0];
        pl->mapNum = p[1];
        pl->x = (int16_t)get16(p + 2);
        pl->y = (int16_t)get16(p + 4);
        pl->facing = p[6];
        pl->flags = p[7];
        break;
    case S_CHAT_FROM:
        if (len >= 3) {
            uint8_t text[65];
            int n = len - 2 > 64 ? 64 : len - 2;
            uint16_t id = get16(p);
            memcpy(text, p + 2, n);
            text[n] = 0xFF;
            if (id == 0)                       /* the server itself: events etc. */
                PcChatReceive(NULL, 0, text);
            else if (id == my_player_id)
                PcChatReceive(my_name, my_tag, text);
            else if ((pl = player_slot(id, 0)) != NULL)
                PcChatReceive(pl->name, pl->tag, text);
        }
        break;
    case S_GUILD_CHAT_FROM:
        if (len >= PC_NET_NAME_LEN + 3) {
            uint8_t text[65];
            int n = len - PC_NET_NAME_LEN - 2 > 64 ? 64 : len - PC_NET_NAME_LEN - 2;
            memcpy(text, p + PC_NET_NAME_LEN + 2, n);
            text[n] = 0xFF;
            PcChatReceiveGuild(p, get16(p + PC_NET_NAME_LEN), text);
        }
        break;
    case S_PLAYER_GONE:
        if (len >= 2 && (pl = player_slot(get16(p), 0))) pl->active = 0;
        break;
    case S_REQUESTED:
        if (len < 3) break;
        incoming = 1;
        incoming_from = get16(p);
        incoming_kind = p[2];
        break;
    case S_DECLINED:
        request_result = -1;
        break;
    case S_SESSION:
        if (len < 5) break;
        link_id = p[0];
        link_players = p[1];
        link_kind = p[2];
        link_active = 1;
        link_broken = 0;
        link_count = 0;
        link_open = link_connected = 0;
        request_result = 1;
        break;
    case S_WORLD:
        if (len < 15) break;
        world.boostType = p[0];
        world.shinyOdds = get16(p + 1);
        world.eventKind = p[3];
        world.mapGroup = p[4];
        world.mapNum = p[5];
        world.species = get16(p + 6);
        world.minLevel = p[8];
        world.maxLevel = p[9];
        world.percent = p[10];
        have_world = 1;
        break;
    case S_LINK_STATE:
        if (len >= 1 && link_active) {
            link_connected = p[0];
            link_count = 0;
        }
        break;
    case S_LINK_FRAME:
        if (len < 1 || !link_active || !link_open || link_count >= LINK_RING) break;
        {
            int slot = (link_head + link_count) % LINK_RING;
            memset(link_ring[slot], 0, sizeof link_ring[slot]);
            for (int i = 0; i < p[0] && i < 4 && 1 + i * 16 + 16 <= len; i++)
                for (int j = 0; j < 8; j++)
                    link_ring[slot][i][j] = get16(p + 1 + i * 16 + j * 2);
            link_count++;
        }
        break;
    case S_SESSION_END:
        if (link_active) link_broken = 1;
        link_active = 0;
        break;
    case S_ERROR:
        if (op == OP_JOIN) finish_op(PC_NET_OP_FAILED);
        break;
    }
}

void PcNetPump(void)
{
    uint8_t type;
    const uint8_t *p;
    int len, n;

    if (master.s != INVALID_SOCKET) {
        if (!conn_io(&master)) {
            conn_close(&master);
            master_logged_in = 0;
            if (op == OP_REGISTER || op == OP_LOGIN || op == OP_RENAME || op == OP_REQUEST) finish_op(PC_NET_OP_FAILED);
        }
        while ((n = conn_next(&master, &type, &p, &len)) > 0) {
            handle_master(type, p, len);
            conn_consume(&master, n);
        }
    }
    if (game.s != INVALID_SOCKET) {
        if (!conn_io(&game)) {
            if (op == OP_JOIN) finish_op(PC_NET_OP_FAILED);
            PcNetDisconnect();
            return;
        }
        while ((n = conn_next(&game, &type, &p, &len)) > 0) {
            handle_game(type, p, len);
            conn_consume(&game, n);
        }
    }
}

void PcNetLogLinkError(uint32_t status, int state, int sendCount, int recvCount, int lag, int queueFull)
{
    FILE *f = fopen("linkerr.log", "a");
    if (!f) return;
    fprintf(f, "link error: status=%08x state=%d send=%d recv=%d lag=%d queueFull=%d net=%d broken=%d\n",
            (unsigned)status, state, sendCount, recvCount, lag, queueFull, link_active, link_broken);
    fclose(f);
}
