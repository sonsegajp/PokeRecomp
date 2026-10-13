"""master.py — FireRed PC master server: accounts, server list, friends.

    python master.py [--port 27100] [--db master.db] [--key-file server_key.txt]

Accounts have no typed password: on registering, the game generates a random
32-byte secret and keeps it next to the save; we store only its scrypt hash.
Each account gets a 4-digit tag so names can repeat (RED#4821).

Game servers connect here too (G_* messages) with the shared server key: they
register their public address, heartbeat their player count, verify the
login tokens their players present, and report presence for friends lists.
"""
import argparse
import asyncio
import hashlib
import os
import random
import secrets
import sqlite3
import struct
import time

from protocol import *  # noqa: F401,F403
from trades import Trades
from social import Guilds
from market import Market

TOKEN_TTL = 24 * 3600
TICKET_TTL = 60            # a join ticket: one world, once, within a minute
SERVER_TIMEOUT = 30
MAX_SERVERS = 64           # listed worlds
MAX_PER_HOST = 4           # community worlds from one address


def hash_secret(secret, salt):
    return hashlib.scrypt(secret, salt=salt, n=2 ** 14, r=8, p=1, dklen=32)


class Master:
    def __init__(self, db_path, server_key):
        self.db = sqlite3.connect(db_path)
        self.db.execute("PRAGMA journal_mode=WAL")
        self.db.executescript("""
            CREATE TABLE IF NOT EXISTS accounts (
                id INTEGER PRIMARY KEY, name BLOB NOT NULL, tag INTEGER NOT NULL,
                salt BLOB NOT NULL, hash BLOB NOT NULL, created REAL NOT NULL,
                UNIQUE(name, tag));
            CREATE TABLE IF NOT EXISTS friends (
                a INTEGER NOT NULL, b INTEGER NOT NULL, PRIMARY KEY(a, b));
        """)
        self.server_key = server_key
        self.tokens = {}       # token -> (account_id, expiry)
        self.tickets = {}      # join ticket -> (account_id, server id, expiry)
        self.servers = {}      # writer -> dict(id, name, host, port, max, players, seen, region, official, verified)
        self.next_server_id = 1
        self.online = {}       # account_id -> server name
        self.trades = Trades(self.db)
        self.guilds = Guilds(self.db)
        self.market = Market(self.db)

    # ---- accounts ----------------------------------------------------------

    def register(self, name, secret):
        taken = {t for (t,) in self.db.execute("SELECT tag FROM accounts WHERE name=?", (name,))}
        free = [t for t in range(1000, 10000) if t not in taken]
        if not free:
            return None
        tag = random.choice(free)
        salt = os.urandom(16)
        cur = self.db.execute(
            "INSERT INTO accounts(name, tag, salt, hash, created) VALUES (?,?,?,?,?)",
            (name, tag, salt, hash_secret(secret, salt), time.time()))
        self.db.commit()
        return cur.lastrowid, tag

    def free_tag(self, name):
        taken = {t for (t,) in self.db.execute("SELECT tag FROM accounts WHERE name=?", (name,))}
        free = [t for t in range(1000, 10000) if t not in taken]
        return random.choice(free) if free else None

    def rename(self, account_id, name):
        tag = self.free_tag(name)
        if tag is None:
            return None
        self.db.execute("UPDATE accounts SET name=?, tag=? WHERE id=?", (name, tag, account_id))
        self.db.commit()
        return tag

    def login(self, account_id, secret):
        row = self.db.execute("SELECT name, tag, salt, hash FROM accounts WHERE id=?",
                              (account_id,)).fetchone()
        if not row or not secrets.compare_digest(hash_secret(secret, row[2]), row[3]):
            return None
        token = os.urandom(TOKEN_LEN)
        self.tokens[token] = (account_id, time.time() + TOKEN_TTL)
        return token, row[0], row[1]

    def account(self, account_id):
        return self.db.execute("SELECT name, tag FROM accounts WHERE id=?", (account_id,)).fetchone()

    # ---- friends -------------------------------------------------------------

    def add_friend(self, me, name, tag):
        row = self.db.execute("SELECT id FROM accounts WHERE name=? AND tag=?", (name, tag)).fetchone()
        if not row or row[0] == me:
            return False
        self.db.execute("INSERT OR IGNORE INTO friends(a, b) VALUES (?,?)", (me, row[0]))
        self.db.commit()
        return True

    def remove_friend(self, me, other):
        self.db.execute("DELETE FROM friends WHERE a=? AND b=?", (me, other))
        self.db.commit()

    def to_servers(self, data):
        for w in list(self.servers):
            try:
                w.write(data)
            except Exception:
                pass

    def friend_list(self, me):
        out = []
        for (fid,) in self.db.execute("SELECT b FROM friends WHERE a=?", (me,)):
            name, tag = self.account(fid)
            server = self.online.get(fid)
            out.append(struct.pack("<I", fid) + fixed(name, NAME_LEN) + struct.pack("<HB", tag, server is not None)
                       + text(server or "", 24))
        return out

    # ---- connections -----------------------------------------------------------

    def server_list(self):
        now = time.time()
        live = [s for s in self.servers.values() if now - s["seen"] < SERVER_TIMEOUT]
        live.sort(key=lambda s: (not s["official"], s["name"].lower()))   # ours first
        live = live[:MAX_SERVERS]
        body = struct.pack("<B", len(live))
        for s in live:
            body += text(s["name"], 24) + text(s["host"], 64) + struct.pack("<HHH", s["port"], s["players"], s["max"])
        # then (older clients stop reading before each of these) regions, ids, flags
        body += bytes(s.get("region", REGION_KANTO) for s in live)
        body += b"".join(struct.pack("<I", s["id"]) for s in live)
        return body + bytes(SERVER_OFFICIAL if s["official"] else 0 for s in live)

    def ticket(self, account_id, server_id):
        """A one-use pass onto one world: what a player hands that world's server.
        A community server never sees anything it could log in elsewhere with."""
        now = time.time()
        self.tickets = {k: v for k, v in self.tickets.items() if v[2] > now}
        if not any(s["id"] == server_id for s in self.servers.values()):
            return None
        t = os.urandom(TOKEN_LEN)
        self.tickets[t] = (account_id, server_id, now + TICKET_TTL)
        return t

    async def handle(self, reader, writer):
        peer = writer.get_extra_info("peername")
        account_id = None
        try:
            while True:
                msg = await read_frame(reader)
                if msg is None:
                    break
                t, p = msg
                reply = self.dispatch(writer, t, p, account_id)
                if isinstance(reply, tuple):          # (new account id, frame)
                    account_id, reply = reply
                if reply:
                    writer.write(reply)
                    await writer.drain()
        except Exception as e:  # one bad client never takes the server down
            print("master: client", peer, "error:", e)
        finally:
            if writer in self.servers:
                gone = self.servers.pop(writer)
                print("master: game server left:", gone["name"])
                self.online = {a: s for a, s in self.online.items() if s != gone["name"]}
            writer.close()

    def dispatch(self, writer, t, p, account_id):
        if t == M_HELLO:
            (version,) = struct.unpack_from("<H", p)
            return None if version == VERSION else frame(M_ERROR, bytes([ERR_BAD_VERSION]))
        if t == M_REGISTER and len(p) >= NAME_LEN + SECRET_LEN:
            result = self.register(bytes(p[:NAME_LEN]), bytes(p[NAME_LEN:NAME_LEN + SECRET_LEN]))
            if not result:
                return frame(M_ERROR, bytes([ERR_FULL]))
            print("master: registered account", result[0], "tag", result[1])
            return frame(M_REGISTERED, struct.pack("<IH", *result))
        if t == M_LOGIN and len(p) >= 4 + SECRET_LEN:
            (aid,) = struct.unpack_from("<I", p)
            result = self.login(aid, bytes(p[4:4 + SECRET_LEN]))
            if not result:
                return frame(M_ERROR, bytes([ERR_BAD_LOGIN]))
            token, name, tag = result
            return aid, frame(M_LOGIN_OK, token + fixed(name, NAME_LEN) + struct.pack("<H", tag))
        if t == M_RENAME and account_id and len(p) >= NAME_LEN:
            name = bytes(p[:NAME_LEN])
            tag = self.rename(account_id, name)
            if tag is None:
                return frame(M_ERROR, bytes([ERR_FULL]))
            print("master: account", account_id, "renamed, tag", tag)
            return frame(M_RENAMED, name + struct.pack("<H", tag))
        if account_id and M_WONDER <= t <= M_GTS_WITHDRAW:
            names = lambda acc: (self.account(acc) or (b"", 0))[0]
            reply = self.trades.dispatch(account_id, t, p, names)
            if reply:
                return reply
        if t == M_LIST_SERVERS:
            return frame(M_SERVER_LIST, self.server_list())
        if t == M_JOIN_TICKET and account_id and len(p) >= 4:
            ticket = self.ticket(account_id, struct.unpack_from("<I", p)[0])
            return frame(M_JOIN_TICKET_OK, ticket) if ticket else frame(M_ERROR, bytes([ERR_NOT_FOUND]))
        if t == M_FRIEND_ADD and account_id and len(p) >= NAME_LEN + 2:
            (tag,) = struct.unpack_from("<H", p, NAME_LEN)
            ok = self.add_friend(account_id, bytes(p[:NAME_LEN]), tag)
            return frame(M_FRIENDS, self.friends_frame(account_id)) if ok else frame(M_ERROR, bytes([ERR_NOT_FOUND]))
        if t == M_FRIEND_LIST and account_id:
            return frame(M_FRIENDS, self.friends_frame(account_id))
        if t == M_FRIEND_REMOVE and account_id and len(p) >= 4:
            self.remove_friend(account_id, struct.unpack_from("<I", p)[0])
            return frame(M_FRIENDS, self.friends_frame(account_id))
        if account_id and M_MARKET_SELL <= t <= M_MARKET_COLLECT:
            reply = self.market.dispatch(account_id, t, p, lambda a: (self.account(a) or (b"", 0))[0])
            if reply:
                return reply
        if account_id and M_GUILD_INFO <= t <= M_GUILD_LEAVE:
            result = self.guilds.dispatch(account_id, t, p, lambda a: self.account(a) or (b"", 0), self.online)
            if result:
                reply, changed = result
                if changed:
                    self.to_servers(frame(G_GUILD_SET, struct.pack("<II", account_id, self.guilds.guild_of(account_id))))
                return reply

        # game servers
        if t == G_REGISTER and len(p) >= 32 + 24 + 64 + 4:
            key = bytes(p[:32])
            official = secrets.compare_digest(key, fixed(self.server_key, 32))
            if not official and any(key):
                return frame(M_ERROR, bytes([ERR_BAD_LOGIN]))   # a wrong key: not a community world either
            name, host = untext(p[32:56]).strip(), untext(p[56:120]).strip()
            port, mx = struct.unpack_from("<HH", p, 120)
            region = p[124] if len(p) > 124 else REGION_KANTO
            peer = (writer.get_extra_info("peername") or ("",))[0]
            if not official:
                # anyone may host: players reach it where it connects from
                host = peer
                if sum(1 for s in self.servers.values() if not s["official"] and s["host"] == host) >= MAX_PER_HOST:
                    return frame(M_ERROR, bytes([ERR_FULL]))
                name = name or "Community World"
            elif not host:
                host = peer
            self.servers[writer] = dict(id=self.next_server_id, name=name, host=host, port=port, max=min(mx, 200),
                                        players=0, seen=time.time(), region=region, official=official, verified=set())
            self.next_server_id += 1
            print("master: game server up:", name, "(official)" if official else "(community)", f"{host}:{port}")
            return None
        if writer in self.servers:
            s = self.servers[writer]
            if t == G_HEARTBEAT:
                (s["players"],) = struct.unpack_from("<H", p)
                s["seen"] = time.time()
            elif t == G_VERIFY:
                req, token = struct.unpack_from("<I", p)[0], bytes(p[4:4 + TOKEN_LEN])
                ticket = self.tickets.pop(token, None)
                aid = None
                if ticket and ticket[1] == s["id"] and ticket[2] > time.time():
                    aid = ticket[0]
                elif s["official"]:                   # older clients: their login token, on our worlds only
                    entry = self.tokens.get(token)
                    if entry and entry[1] > time.time():
                        aid = entry[0]
                if aid:
                    s["verified"].add(aid)
                    name, tag = self.account(aid)
                    entry = (aid,)
                    return frame(G_VERIFIED, struct.pack("<IBI", req, 1, entry[0]) + fixed(name, NAME_LEN)
                                 + struct.pack("<HI", tag, self.guilds.guild_of(entry[0])))
                return frame(G_VERIFIED, struct.pack("<IBI", req, 0, 0) + bytes(NAME_LEN) + bytes(6))
            elif t == G_GUILD_CHAT and len(p) >= 4:
                # only for a player this world verified; their guild and name come from here
                (aid,) = struct.unpack_from("<I", p)
                guild = self.guilds.guild_of(aid)
                if aid in s["verified"] and guild:
                    name, tag = self.account(aid)
                    self.to_servers(frame(G_GUILD_CHAT, struct.pack("<I", guild) + fixed(name, NAME_LEN)
                                          + struct.pack("<H", tag) + bytes(p[4:4 + 64])))
            elif t == G_PRESENCE:
                aid, on = struct.unpack_from("<IB", p)
                if on and aid in s["verified"]:
                    self.online[aid] = s["name"]
                elif not on and self.online.get(aid) == s["name"]:
                    self.online.pop(aid, None)
            return None
        return frame(M_ERROR, bytes([ERR_BAD_REQUEST]))

    def friends_frame(self, me):
        entries = self.friend_list(me)
        return struct.pack("<B", len(entries)) + b"".join(entries[:32])


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=27100)
    ap.add_argument("--db", default="master.db")
    ap.add_argument("--key-file", default="server_key.txt", help="shared secret game servers must present")
    args = ap.parse_args()
    if not os.path.exists(args.key_file):
        with open(args.key_file, "w") as f:
            f.write(secrets.token_hex(16))
        print("master: wrote a new server key to", args.key_file)
    key = open(args.key_file).read().strip().encode()
    master = Master(args.db, key)
    srv = await asyncio.start_server(master.handle, "0.0.0.0", args.port)
    print("master: listening on", args.port)
    async with srv:
        await srv.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
