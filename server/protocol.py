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
CMD_LENGTH = 8        # u16 words per link command (link.c CMD_LENGTH)

# client <-> master
M_HELLO          = 0x01  # u16 version
M_REGISTER       = 0x02  # name[8] secret[32]
M_LOGIN          = 0x03  # u32 account_id, secret[32]
M_LIST_SERVERS   = 0x04
M_FRIEND_ADD     = 0x05  # name[8] u16 tag
M_FRIEND_LIST    = 0x06
M_RENAME         = 0x07  # name[8] (logged in) -> M_RENAMED
M_REGISTERED     = 0x81  # u32 account_id, u16 tag
M_LOGIN_OK       = 0x82  # token[16], name[8], u16 tag
M_SERVER_LIST    = 0x83  # u8 n, n * (name[24], host[64], u16 port, u16 players, u16 max)
M_FRIENDS        = 0x84  # u8 n, n * (u32 id, name[8], u16 tag, u8 online, name[24] server)
M_RENAMED        = 0x85  # name[8], u16 tag
M_ERROR          = 0x8F  # u8 code
ERR_BAD_VERSION, ERR_BAD_LOGIN, ERR_BAD_REQUEST, ERR_NOT_FOUND, ERR_FULL = 1, 2, 3, 4, 5

# game server <-> master
G_REGISTER       = 0x20  # key[32] name[24] host[64] u16 port u16 max
G_HEARTBEAT      = 0x21  # u16 players
G_VERIFY         = 0x22  # u32 req, token[16]
G_PRESENCE       = 0x23  # u32 account_id, u8 online
G_VERIFIED       = 0xA0  # u32 req, u8 ok, u32 account_id, name[8], u16 tag

# client <-> game server
S_JOIN           = 0x40  # token[16], u8 gender, u16 version
S_POS            = 0x41  # u8 group, u8 num, s16 x, s16 y, u8 facing, u8 flags
S_CHAT           = 0x42  # text (game charset, 0xFF-terminated, <= 64)
S_REQUEST        = 0x43  # u16 target, u8 kind
S_RESPOND        = 0x44  # u16 from, u8 accept
S_LINK_CMD       = 0x45  # u16[8]
S_LINK_END       = 0x46
S_LINK_OPEN      = 0x47  # u8 open (this player's end of the cable)
S_WELCOME        = 0xC0  # u16 my_id, name[24] server_name
S_PLAYER         = 0xC1  # u16 id, name[8], u16 tag, u8 gender, u8 group, u8 num, s16 x, s16 y, u8 facing, u8 flags
S_PLAYER_GONE    = 0xC2  # u16 id
S_CHAT_FROM      = 0xC3  # u16 id, text
S_REQUESTED      = 0xC4  # u16 from, u8 kind
S_DECLINED       = 0xC5  # u16 target, u8 reason
S_SESSION        = 0xC6  # u8 link_id, u8 players, u8 kind, u16 partner
S_LINK_FRAME     = 0xC7  # u8 players, players * u16[8]
S_SESSION_END    = 0xC8  # u8 reason
S_LINK_STATE     = 0xC9  # u8 all_open (every member has its end open = "connected")
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
