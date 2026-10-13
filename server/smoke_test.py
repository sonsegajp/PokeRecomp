"""smoke_test.py — drive master + game server with two fake clients.

Run master.py and gameserver.py first (defaults), then: python smoke_test.py
"""
import asyncio
import os
import struct

from protocol import *  # noqa: F401,F403


async def call(reader, writer, t, payload=b"", want=None):
    writer.write(frame(t, payload))
    await writer.drain()
    while True:
        msg = await read_frame(reader)
        assert msg, "connection closed"
        if want is None or msg[0] in want:
            return msg


async def login(name):
    r, w = await asyncio.open_connection("127.0.0.1", 27100)
    w.write(frame(M_HELLO, struct.pack("<H", VERSION)))
    secret = os.urandom(SECRET_LEN)
    t, p = await call(r, w, M_REGISTER, fixed(name, NAME_LEN) + secret, {M_REGISTERED})
    aid, tag = struct.unpack("<IH", p)
    t, p = await call(r, w, M_LOGIN, struct.pack("<I", aid) + secret, {M_LOGIN_OK})
    token = p[:TOKEN_LEN]
    t, p = await call(r, w, M_LIST_SERVERS, b"", {M_SERVER_LIST})
    n = p[0]
    host = untext(p[1 + 24:1 + 24 + 64])
    (port,) = struct.unpack_from("<H", p, 1 + 24 + 64)
    print(f"{name}: account {aid} tag {tag}, {n} server(s), first at {host}:{port}")
    return aid, tag, token, (host, port), (r, w)


async def main():
    a = await login(b"ALICE")
    b = await login(b"BOB")
    ra, wa = await asyncio.open_connection(*a[3])
    rb, wb = await asyncio.open_connection(*b[3])
    _, p = await call(ra, wa, S_JOIN, a[2] + struct.pack("<BH", 0, VERSION), {S_WELCOME})
    (ida,) = struct.unpack_from("<H", p)
    _, p = await call(rb, wb, S_JOIN, b[2] + struct.pack("<BH", 1, VERSION), {S_WELCOME})
    (idb,) = struct.unpack_from("<H", p)
    print("joined as", ida, idb)

    wa.write(frame(S_POS, struct.pack("<BBhhBB", 3, 0, 10, 12, 1, 0)))
    _, p = await call(rb, wb, S_POS, struct.pack("<BBhhBB", 3, 0, 11, 12, 2, 0), {S_PLAYER})
    print("bob sees player", struct.unpack_from("<H", p)[0], "at", struct.unpack_from("<BBhhBB", p, 2 + NAME_LEN + 3))

    wa.write(frame(S_REQUEST, struct.pack("<HB", idb, KIND_TRADE)))
    _, p = await call(rb, wb, S_CHAT, b"\xff", {S_REQUESTED})
    print("bob got request from", struct.unpack("<HB", p))
    _, p = await call(rb, wb, S_RESPOND, struct.pack("<HB", ida, 1), {S_SESSION})
    print("bob session (link id, players, kind, partner):", struct.unpack("<BBBH", p))
    _, p = await call(ra, wa, S_LINK_CMD, struct.pack("<8H", 0x2222, 1, 2, 3, 4, 5, 6, 7), {S_SESSION})
    print("alice session:", struct.unpack("<BBBH", p))
    # frames only flow while both ends of the cable are open
    wa.write(frame(S_LINK_OPEN, b"\x01"))
    wb.write(frame(S_LINK_OPEN, b"\x01"))
    wb.write(frame(S_LINK_CMD, struct.pack("<8H", 0x3333, 9, 9, 9, 9, 9, 9, 9)))
    for who, (r, w) in (("alice", (ra, wa)), ("bob", (rb, wb))):
        while True:
            t, p = await read_frame(r)
            if t == S_LINK_FRAME:
                print(who, "link frame:", [struct.unpack_from("<8H", p, 1 + 16 * i) for i in range(p[0])])
                break
    print("OK")


asyncio.run(main())
