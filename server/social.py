"""social.py — guilds (master server).

A guild has a name (up to 8 letters, game charset, unique), an owner and up
to MAX_MEMBERS trainers; a trainer is in at most one. Anyone can join a guild
by its name. When the owner leaves, the longest-standing member takes over;
an empty guild is gone.

Guild chat runs through the master: a game server forwards a member's line
(G_GUILD_CHAT) and the master hands it to every game server, which delivers it
to that guild's members it has. The master tells game servers when someone's
guild changes (G_GUILD_SET) so they always know who is in which.
"""
import struct
import time

from protocol import *  # noqa: F401,F403

MAX_MEMBERS = 50


class Guilds:
    def __init__(self, db):
        self.db = db
        db.executescript("""
            CREATE TABLE IF NOT EXISTS guilds (
                id INTEGER PRIMARY KEY, name BLOB NOT NULL UNIQUE, owner INTEGER NOT NULL, created REAL NOT NULL);
            CREATE TABLE IF NOT EXISTS guild_members (
                account INTEGER PRIMARY KEY, guild INTEGER NOT NULL, joined REAL NOT NULL);
        """)

    def guild_of(self, account):
        row = self.db.execute("SELECT guild FROM guild_members WHERE account=?", (account,)).fetchone()
        return row[0] if row else 0

    def info(self, account, account_name, online):
        """M_GUILD for this trainer."""
        gid = self.guild_of(account)
        if not gid:
            return frame(M_GUILD, bytes([0]) + bytes(GUILD_NAME_LEN) + bytes([0]))
        name, owner = self.db.execute("SELECT name, owner FROM guilds WHERE id=?", (gid,)).fetchone()
        members = self.db.execute("SELECT account FROM guild_members WHERE guild=? ORDER BY joined", (gid,)).fetchall()
        body = bytes([1]) + fixed(name, GUILD_NAME_LEN) + bytes([min(len(members), 32)])
        for (acc,) in members[:32]:
            mname, tag = account_name(acc)
            body += fixed(mname, NAME_LEN) + struct.pack("<HBB", tag, acc in online, acc == owner)
        return frame(M_GUILD, body)

    def create(self, account, name):
        if self.guild_of(account):
            return ERR_BAD_REQUEST
        if not name or name[0] == 0xFF:
            return ERR_BAD_REQUEST
        if self.db.execute("SELECT 1 FROM guilds WHERE name=?", (name,)).fetchone():
            return ERR_FULL                      # name taken
        cur = self.db.execute("INSERT INTO guilds(name, owner, created) VALUES (?,?,?)", (name, account, time.time()))
        self.db.execute("INSERT INTO guild_members(account, guild, joined) VALUES (?,?,?)", (account, cur.lastrowid, time.time()))
        self.db.commit()
        return 0

    def join(self, account, name):
        if self.guild_of(account):
            return ERR_BAD_REQUEST
        row = self.db.execute("SELECT id FROM guilds WHERE name=?", (name,)).fetchone()
        if not row:
            return ERR_NOT_FOUND
        (count,) = self.db.execute("SELECT COUNT(*) FROM guild_members WHERE guild=?", (row[0],)).fetchone()
        if count >= MAX_MEMBERS:
            return ERR_FULL
        self.db.execute("INSERT INTO guild_members(account, guild, joined) VALUES (?,?,?)", (account, row[0], time.time()))
        self.db.commit()
        return 0

    def leave(self, account):
        gid = self.guild_of(account)
        if not gid:
            return ERR_BAD_REQUEST
        self.db.execute("DELETE FROM guild_members WHERE account=?", (account,))
        rest = self.db.execute("SELECT account FROM guild_members WHERE guild=? ORDER BY joined", (gid,)).fetchall()
        if not rest:
            self.db.execute("DELETE FROM guilds WHERE id=?", (gid,))
        else:
            self.db.execute("UPDATE guilds SET owner=? WHERE id=? AND owner=?", (rest[0][0], gid, account))
        self.db.commit()
        return 0

    def dispatch(self, account, t, p, account_name, online):
        """(reply frame, guild changed?) or None."""
        if t == M_GUILD_INFO:
            return self.info(account, account_name, online), False
        if t in (M_GUILD_CREATE, M_GUILD_JOIN) and len(p) >= GUILD_NAME_LEN:
            name = guild_name(p[:GUILD_NAME_LEN])
            err = (self.create if t == M_GUILD_CREATE else self.join)(account, name)
        elif t == M_GUILD_LEAVE:
            err = self.leave(account)
        else:
            return None
        if err:
            return frame(M_ERROR, bytes([err])), False
        return self.info(account, account_name, online), True


def guild_name(raw):
    """Canonical form: up to the first 0xFF, padded with 0xFF."""
    raw = bytes(raw[:GUILD_NAME_LEN])
    end = raw.find(b"\xff")
    raw = raw if end < 0 else raw[:end]
    return raw[:GUILD_NAME_LEN - 1].ljust(GUILD_NAME_LEN, b"\xff")
