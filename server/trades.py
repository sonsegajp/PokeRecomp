"""trades.py — Wonder Trade, the GTS and the mailbox (master server).

Every Pokémon is checked by legality.py before the server accepts it, and
only legal ones are ever handed out.

Wonder Trade is a queue of waiting deposits. A new deposit is paired with the
oldest one from another account: the depositor gets that Pokémon right away,
and the waiting trainer gets the new one in their mailbox. With nobody
waiting, the deposit itself waits.

GTS: a listing says which species / level range / gender the depositor wants.
Others search and offer a matching Pokémon; the offerer gets the listed one
immediately, the depositor gets the offer in their mailbox.
"""
import struct
import time

import legality
from protocol import *  # noqa: F401,F403

MAX_LISTINGS_PER_ACCOUNT = 3
SEARCH_LIMIT = 16


def mon_info(raw):
    mon = legality.decode(raw)
    species = mon["species"]
    ratio = legality.SPECIES[species]["gender"]
    if ratio == 255:
        gender = GENDER_NONE
    elif ratio == 254:
        gender = GENDER_FEMALE
    elif ratio == 0:
        gender = GENDER_MALE
    else:
        gender = GENDER_FEMALE if (mon["personality"] & 0xFF) < ratio else GENDER_MALE
    return species, raw[84], gender, mon["is_egg"]


class Trades:
    def __init__(self, db):
        self.db = db
        db.executescript("""
            CREATE TABLE IF NOT EXISTS wonder_queue (
                id INTEGER PRIMARY KEY, account INTEGER NOT NULL, mon BLOB NOT NULL, created REAL NOT NULL);
            CREATE TABLE IF NOT EXISTS mailbox (
                id INTEGER PRIMARY KEY, account INTEGER NOT NULL, mon BLOB NOT NULL,
                source INTEGER NOT NULL, created REAL NOT NULL);
            CREATE TABLE IF NOT EXISTS gts (
                id INTEGER PRIMARY KEY, account INTEGER NOT NULL, mon BLOB NOT NULL,
                species INTEGER, level INTEGER, gender INTEGER,
                want_species INTEGER, want_min INTEGER, want_max INTEGER, want_gender INTEGER,
                created REAL NOT NULL);
        """)

    @staticmethod
    def acceptable(raw):
        """Legal, a real Pokémon, and not an egg (eggs don't travel here)."""
        if len(raw) != MON_SIZE or legality.decode(raw) is None:
            return False
        if legality.check(raw) is not None:
            return False
        species, level, gender, egg = mon_info(raw)
        return species != 0 and not egg

    def mail(self, account, raw, source):
        self.db.execute("INSERT INTO mailbox(account, mon, source, created) VALUES (?,?,?,?)",
                        (account, raw, source, time.time()))

    # ---- Wonder Trade --------------------------------------------------------

    def wonder(self, account, raw):
        if not self.acceptable(raw):
            return frame(M_ERROR, bytes([ERR_ILLEGAL]))
        row = self.db.execute("SELECT id, account, mon FROM wonder_queue WHERE account != ? ORDER BY id LIMIT 1",
                              (account,)).fetchone()
        if row:
            qid, other, other_mon = row
            self.db.execute("DELETE FROM wonder_queue WHERE id=?", (qid,))
            self.mail(other, raw, MAIL_WONDER)
            self.db.commit()
            return frame(M_WONDER_RESULT, bytes([1]) + bytes(other_mon))
        self.db.execute("INSERT INTO wonder_queue(account, mon, created) VALUES (?,?,?)", (account, raw, time.time()))
        self.db.commit()
        return frame(M_WONDER_RESULT, bytes([2]) + bytes(MON_SIZE))

    # ---- mailbox ---------------------------------------------------------------

    def collect(self, account):
        rows = self.db.execute("SELECT id, mon, source FROM mailbox WHERE account=? ORDER BY id", (account,)).fetchall()
        if not rows:
            return frame(M_MAIL, bytes([0, 0, 0]) + bytes(MON_SIZE))
        mid, raw, source = rows[0]
        self.db.execute("DELETE FROM mailbox WHERE id=?", (mid,))
        self.db.commit()
        return frame(M_MAIL, bytes([len(rows) - 1, 1, source]) + bytes(raw))

    # ---- GTS --------------------------------------------------------------------------

    def deposit(self, account, raw, want_species, want_min, want_max, want_gender):
        if not self.acceptable(raw):
            return frame(M_ERROR, bytes([ERR_ILLEGAL]))
        if not (0 < want_species < legality.DATA["num_species"]) or not (1 <= want_min <= want_max <= 100):
            return frame(M_ERROR, bytes([ERR_BAD_REQUEST]))
        (count,) = self.db.execute("SELECT COUNT(*) FROM gts WHERE account=?", (account,)).fetchone()
        if count >= MAX_LISTINGS_PER_ACCOUNT:
            return frame(M_ERROR, bytes([ERR_FULL]))
        species, level, gender, _ = mon_info(raw)
        cur = self.db.execute(
            "INSERT INTO gts(account, mon, species, level, gender, want_species, want_min, want_max, want_gender, created)"
            " VALUES (?,?,?,?,?,?,?,?,?,?)",
            (account, raw, species, level, gender, want_species, want_min, want_max, want_gender, time.time()))
        self.db.commit()
        return frame(M_GTS_DEPOSITED, struct.pack("<I", cur.lastrowid))

    def _list(self, rows, names):
        body = struct.pack("<B", len(rows))
        for (gid, acc, species, level, gender, ws, wmin, wmax, wg) in rows:
            body += struct.pack("<IHBBHBBB", gid, species, level, gender, ws, wmin, wmax, wg) + fixed(names(acc), NAME_LEN)
        return frame(M_GTS_LIST, body)

    def search(self, account, species, lo, hi, names):
        rows = self.db.execute(
            "SELECT id, account, species, level, gender, want_species, want_min, want_max, want_gender FROM gts"
            " WHERE species=? AND level BETWEEN ? AND ? AND account != ? ORDER BY id DESC LIMIT ?",
            (species, lo, hi, account, SEARCH_LIMIT)).fetchall()
        return self._list(rows, names)

    def mine(self, account, names):
        rows = self.db.execute(
            "SELECT id, account, species, level, gender, want_species, want_min, want_max, want_gender FROM gts"
            " WHERE account=? ORDER BY id", (account,)).fetchall()
        return self._list(rows, names)

    def offer(self, account, gid, raw):
        row = self.db.execute("SELECT account, mon, want_species, want_min, want_max, want_gender FROM gts WHERE id=?",
                              (gid,)).fetchone()
        if not row or row[0] == account:
            return frame(M_ERROR, bytes([ERR_NOT_FOUND]))
        owner, listed, ws, wmin, wmax, wg = row
        if not self.acceptable(raw):
            return frame(M_ERROR, bytes([ERR_ILLEGAL]))
        species, level, gender, _ = mon_info(raw)
        if species != ws or not (wmin <= level <= wmax) or (wg != GENDER_ANY and gender != wg):
            return frame(M_ERROR, bytes([ERR_NO_MATCH]))
        self.db.execute("DELETE FROM gts WHERE id=?", (gid,))
        self.mail(owner, raw, MAIL_GTS)
        self.db.commit()
        return frame(M_GTS_RESULT, bytes([1]) + bytes(listed))

    def withdraw(self, account, gid):
        row = self.db.execute("SELECT mon FROM gts WHERE id=? AND account=?", (gid, account)).fetchone()
        if not row:
            return frame(M_ERROR, bytes([ERR_NOT_FOUND]))
        self.db.execute("DELETE FROM gts WHERE id=?", (gid,))
        self.db.commit()
        return frame(M_GTS_RESULT, bytes([1]) + bytes(row[0]))

    def dispatch(self, account, t, p, names):
        if t == M_WONDER and len(p) >= MON_SIZE:
            return self.wonder(account, bytes(p[:MON_SIZE]))
        if t == M_MAIL_COLLECT:
            return self.collect(account)
        if t == M_GTS_DEPOSIT and len(p) >= MON_SIZE + 5:
            ws, wmin, wmax, wg = struct.unpack_from("<HBBB", p, MON_SIZE)
            return self.deposit(account, bytes(p[:MON_SIZE]), ws, wmin, wmax, wg)
        if t == M_GTS_SEARCH and len(p) >= 4:
            species, lo, hi = struct.unpack_from("<HBB", p)
            return self.search(account, species, lo, hi, names)
        if t == M_GTS_MINE:
            return self.mine(account, names)
        if t == M_GTS_OFFER and len(p) >= 4 + MON_SIZE:
            (gid,) = struct.unpack_from("<I", p)
            return self.offer(account, gid, bytes(p[4:4 + MON_SIZE]))
        if t == M_GTS_WITHDRAW and len(p) >= 4:
            (gid,) = struct.unpack_from("<I", p)
            return self.withdraw(account, gid)
        return None
