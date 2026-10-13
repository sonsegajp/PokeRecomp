"""protocol.py — FireRed PC online wire format (shared by master + game server).

Every message on a TCP connection is a frame:

    u16 length   (of everything after this field: type + payload)
    u8  type
    ... payload (little-endian, fixed layouts below)

Names travel as the game's own 8-byte name buffers (FireRed charset,
0xFF-terminated) so nothing is ever re-encoded; the servers treat them as
opaque bytes and only hex them for logs.
"""
import asyncio
import struct

VERSION = 1

NAME_LEN = 8          # PLAYER_NAME_LENGTH + 1 in the game
SECRET_LEN = 32
TOKEN_LEN = 16
GUILD_NAME_LEN = 10   # 8 letters + EOS, padded with 0xFF
CMD_LENGTH = 8        # u16 words per link command (link.c CMD_LENGTH)

# client <-> master
M_HELLO          = 0x01  # u16 version
M_REGISTER       = 0x02  # name[8] secret[32]
M_LOGIN          = 0x03  # u32 account_id, secret[32]
M_LIST_SERVERS   = 0x04
M_FRIEND_ADD     = 0x05  # name[8] u16 tag
M_FRIEND_LIST    = 0x06
M_RENAME         = 0x07  # name[8] (logged in) -> M_RENAMED
M_WONDER         = 0x08  # mon[100] -> M_WONDER_RESULT
M_MAIL_COLLECT   = 0x09  # -> M_MAIL
M_GTS_DEPOSIT    = 0x0A  # mon[100], u16 want_species, u8 want_min, u8 want_max, u8 want_gender -> M_GTS_DEPOSITED
M_GTS_SEARCH     = 0x0B  # u16 species, u8 min, u8 max -> M_GTS_LIST
M_GTS_OFFER      = 0x0C  # u32 id, mon[100] -> M_GTS_RESULT
M_GTS_MINE       = 0x0D  # -> M_GTS_LIST (own listings)
M_GTS_WITHDRAW   = 0x0E  # u32 id -> M_GTS_RESULT
M_FRIEND_REMOVE  = 0x0F  # u32 account -> M_FRIENDS
M_GUILD_INFO     = 0x10  # -> M_GUILD
M_GUILD_CREATE   = 0x11  # name[10] -> M_GUILD
M_GUILD_JOIN     = 0x12  # name[10] -> M_GUILD
M_GUILD_LEAVE    = 0x13  # -> M_GUILD
M_MARKET_SELL    = 0x14  # u16 item, u16 quantity, u32 price_each -> M_MARKET_OK (u32 id)
M_MARKET_BROWSE  = 0x15  # -> M_MARKET_LIST
M_MARKET_BUY     = 0x16  # u32 id, u16 quantity, u32 price_each -> M_MARKET_OK (u16 item, u16 quantity, u32 cost)
M_MARKET_MINE    = 0x17  # -> M_MARKET_MINE_LIST
M_MARKET_CANCEL  = 0x18  # u32 id -> M_MARKET_OK (u16 item, u16 quantity)
M_MARKET_COLLECT = 0x19  # u32 room (most money the player can take) -> M_MARKET_OK (u32 amount)
M_REGISTERED     = 0x81  # u32 account_id, u16 tag
M_LOGIN_OK       = 0x82  # token[16], name[8], u16 tag
M_SERVER_LIST    = 0x83  # u8 n, n * (name[24], host[64], u16 port, u16 players, u16 max)
M_FRIENDS        = 0x84  # u8 n, n * (u32 id, name[8], u16 tag, u8 online, name[24] server)
M_RENAMED        = 0x85  # name[8], u16 tag
M_WONDER_RESULT  = 0x86  # u8 status (1 traded: mon follows, 2 waiting), mon[100]
M_MAIL           = 0x87  # u8 remaining, u8 has, u8 source (1 wonder, 2 gts), mon[100]
M_GTS_DEPOSITED  = 0x88  # u32 id
M_GTS_LIST       = 0x89  # u8 n, n * (u32 id, u16 species, u8 level, u8 gender, u16 want, u8 min, u8 max, u8 want_gender, name[8])
M_GTS_RESULT     = 0x8A  # u8 status (1 ok: mon follows), mon[100]
M_MARKET_OK      = 0x8C  # payload depends on the request
M_MARKET_LIST    = 0x8D  # u8 n, n * (u32 id, u16 item, u16 quantity, u32 price_each, name[8])
M_MARKET_MINE_LIST = 0x8E  # u32 wallet, u8 n, n * (u32 id, u16 item, u16 quantity, u32 price_each)
M_GUILD          = 0x8B  # u8 in_guild, name[10], u8 n, n * (name[8], u16 tag, u8 online, u8 owner)
M_ERROR          = 0x8F  # u8 code
ERR_BAD_VERSION, ERR_BAD_LOGIN, ERR_BAD_REQUEST, ERR_NOT_FOUND, ERR_FULL = 1, 2, 3, 4, 5
ERR_ILLEGAL, ERR_NO_MATCH = 6, 7
MON_SIZE = 100
GENDER_ANY, GENDER_MALE, GENDER_FEMALE, GENDER_NONE = 0, 1, 2, 3
MAIL_WONDER, MAIL_GTS = 1, 2

# game server <-> master
G_REGISTER       = 0x20  # key[32] name[24] host[64] u16 port u16 max, u8 region (REGION_*)
G_HEARTBEAT      = 0x21  # u16 players
G_VERIFY         = 0x22  # u32 req, token[16]
G_PRESENCE       = 0x23  # u32 account_id, u8 online
G_GUILD_SET      = 0x24  # master -> servers: u32 account_id, u32 guild (0 = none)
G_GUILD_CHAT     = 0x25  # both ways: u32 guild, name[8], u16 tag, text
G_VERIFIED       = 0xA0  # u32 req, u8 ok, u32 account_id, name[8], u16 tag, u32 guild

# client <-> game server
S_JOIN           = 0x40  # token[16], u8 gender, u16 version, u8 game (GAME_*)
S_POS            = 0x41  # u8 group, u8 num, s16 x, s16 y, u8 facing, u8 flags (POS_*)
S_CHAT           = 0x42  # text (game charset, 0xFF-terminated, <= 64)
S_REQUEST        = 0x43  # u16 target, u8 kind
S_RESPOND        = 0x44  # u16 from, u8 accept
S_LINK_CMD       = 0x45  # u16[8]
S_LINK_END       = 0x46
S_LINK_OPEN      = 0x47  # u8 open (this player's end of the cable)
S_GUILD_CHAT     = 0x48  # text (game charset) -> the sender's guild, on every server
S_WELCOME        = 0xC0  # u16 my_id, name[24] server_name, then u8 has_pos + u8 group, u8 num,
                         #   s16 x, s16 y, u8 facing: where this account last was on this server
S_PLAYER         = 0xC1  # u16 id, name[8], u16 tag, u8 gender, u8 group, u8 num, s16 x, s16 y, u8 facing, u8 flags, u8 game
S_PLAYER_GONE    = 0xC2  # u16 id
S_CHAT_FROM      = 0xC3  # u16 id, text
S_REQUESTED      = 0xC4  # u16 from, u8 kind
S_DECLINED       = 0xC5  # u16 target, u8 reason
S_SESSION        = 0xC6  # u8 link_id, u8 players, u8 kind, u16 partner
S_LINK_FRAME     = 0xC7  # u8 players, players * u16[8]
S_SESSION_END    = 0xC8  # u8 reason
S_LINK_STATE     = 0xC9  # u8 all_open (every member has its end open = "connected")
S_GUILD_CHAT_FROM = 0xCB  # name[8], u16 tag, text
S_WORLD          = 0xCA  # u8 boost_type, u16 shiny_odds, u8 event_kind, u8 group, u8 num, u16 species,
                         #   u8 min, u8 max, u8 percent, u32 seconds_left (S_CHAT_FROM id 0 = the server)
S_ERROR          = 0xCF  # u8 code

KIND_SINGLE, KIND_DOUBLE, KIND_TRADE = 1, 2, 3
DECLINE_NO, DECLINE_BUSY, DECLINE_GONE = 1, 2, 3


def frame(msg_type, payload=b""):
    return struct.pack("<HB", len(payload) + 1, msg_type) + payload


def fixed(data, size):
    """Pad/truncate bytes to a fixed field."""
    return bytes(data[:size]).ljust(size, b"\0")


def text(s, size):
    return fixed(s.encode("utf-8", "replace"), size)


def untext(b):
    return b.split(b"\0", 1)[0].decode("utf-8", "replace")


async def read_frame(reader):
    """Return (type, payload) or None at EOF."""
    try:
        head = await reader.readexactly(2)
        (length,) = struct.unpack("<H", head)
        if length == 0:
            return None
        body = await reader.readexactly(length)
    except (asyncio.IncompleteReadError, ConnectionError):
        return None
    return body[0], body[1:]

EVENT_NONE, EVENT_OUTBREAK = 0, 1

# which game a trainer plays; a server is Kanto (FireRed/LeafGreen) or Hoenn (Ruby/Sapphire/Emerald)
GAME_FIRERED, GAME_LEAFGREEN, GAME_EMERALD, GAME_RUBY, GAME_SAPPHIRE = 0, 1, 2, 3, 4
REGION_KANTO, REGION_HOENN = 0, 1
REGION_NAMES = {"kanto": REGION_KANTO, "hoenn": REGION_HOENN}
GAME_REGION = {GAME_FIRERED: "kanto", GAME_LEAFGREEN: "kanto", GAME_EMERALD: "hoenn", GAME_RUBY: "hoenn", GAME_SAPPHIRE: "hoenn"}

# S_POS flags
POS_DASH, POS_BUSY, POS_SAFE = 1, 2, 4   # POS_SAFE: on foot on an ordinary map (a spot to come back to)
