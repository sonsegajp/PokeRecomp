"""market.py — the POKé MARKET: trainers sell items to each other (master server).

A listing is some of one item at a price each. Buyers take any amount of it;
the seller's money (less the market's fee, which keeps money flowing out of
the economy) waits in their market wallet until they collect it at the desk.

Only items a trainer could trade on a cartridge are allowed: real items, not
key items or HMs (importance), not Mail. Item rules come from the game's own
tables (legality_data.json "items": [pocket, importance, price, mail, name]).

The game moves items and money only after the master has done its part (and
removes items it lists before asking), so a crash can lose a sale, never
duplicate one. Money itself lives in the player's save and is trusted.
"""
import json
import os
import struct
import time

from protocol import *  # noqa: F401,F403

POCKET_KEY_ITEMS = 2
MAX_LISTINGS_PER_ACCOUNT = 10
MAX_QUANTITY = 999
MAX_PRICE = 999999
MAX_MONEY = 999999
FEE_PERCENT = 5
BROWSE_LIMIT = 48

_here = os.path.dirname(os.path.abspath(__file__))
ITEMS = json.load(open(os.path.join(_here, "legality_data.json"), encoding="utf-8"))["items"]


def tradable(item):
    if not (0 < item < len(ITEMS)):
        return False
    pocket, importance, price, mail, name = ITEMS[item]
    return pocket != POCKET_KEY_ITEMS and importance == 0 and not mail and name != "????????"


class Market:
    def __init__(self, db):
        self.db = db
        db.executescript("""
            CREATE TABLE IF NOT EXISTS market (
                id INTEGER PRIMARY KEY, account INTEGER NOT NULL, item INTEGER NOT NULL,
                quantity INTEGER NOT NULL, price INTEGER NOT NULL, created REAL NOT NULL);
            CREATE TABLE IF NOT EXISTS wallets (
                account INTEGER PRIMARY KEY, money INTEGER NOT NULL);
        """)

    def ok(self, payload=b""):
        return frame(M_MARKET_OK, payload)

    def error(self, code):
        return frame(M_ERROR, bytes([code]))

    def credit(self, account, amount):
        self.db.execute("INSERT INTO wallets(account, money) VALUES (?, ?) "
                        "ON CONFLICT(account) DO UPDATE SET money = money + excluded.money", (account, amount))

    def sell(self, account, item, quantity, price):
        if not tradable(item) or not (1 <= quantity <= MAX_QUANTITY) or not (1 <= price <= MAX_PRICE):
            return self.error(ERR_BAD_REQUEST)
        (count,) = self.db.execute("SELECT COUNT(*) FROM market WHERE account=?", (account,)).fetchone()
        if count >= MAX_LISTINGS_PER_ACCOUNT:
            return self.error(ERR_FULL)
        cur = self.db.execute("INSERT INTO market(account, item, quantity, price, created) VALUES (?,?,?,?,?)",
                              (account, item, quantity, price, time.time()))
        self.db.commit()
        return self.ok(struct.pack("<I", cur.lastrowid))

    def browse(self, account, names):
        rows = self.db.execute(
            "SELECT id, account, item, quantity, price FROM market WHERE account != ? "
            "ORDER BY item, price, id LIMIT ?", (account, BROWSE_LIMIT)).fetchall()
        body = bytes([len(rows)])
        for lid, acc, item, qty, price in rows:
            body += struct.pack("<IHHI", lid, item, qty, price) + fixed(names(acc), NAME_LEN)
        return frame(M_MARKET_LIST, body)

    def buy(self, account, lid, quantity, price):
        row = self.db.execute("SELECT account, item, quantity, price FROM market WHERE id=?", (lid,)).fetchone()
        if not row or row[0] == account:
            return self.error(ERR_NOT_FOUND)
        seller, item, left, each = row
        if each != price or not (1 <= quantity <= left):
            return self.error(ERR_NO_MATCH)          # it changed: look again
        cost = each * quantity
        if cost > MAX_MONEY:
            return self.error(ERR_BAD_REQUEST)
        if quantity == left:
            self.db.execute("DELETE FROM market WHERE id=?", (lid,))
        else:
            self.db.execute("UPDATE market SET quantity = quantity - ? WHERE id=?", (quantity, lid))
        self.credit(seller, cost * (100 - FEE_PERCENT) // 100)
        self.db.commit()
        return self.ok(struct.pack("<HHI", item, quantity, cost))

    def mine(self, account):
        row = self.db.execute("SELECT money FROM wallets WHERE account=?", (account,)).fetchone()
        rows = self.db.execute("SELECT id, item, quantity, price FROM market WHERE account=? ORDER BY id",
                               (account,)).fetchall()
        body = struct.pack("<IB", row[0] if row else 0, len(rows))
        for lid, item, qty, price in rows:
            body += struct.pack("<IHHI", lid, item, qty, price)
        return frame(M_MARKET_MINE_LIST, body)

    def cancel(self, account, lid):
        row = self.db.execute("SELECT item, quantity FROM market WHERE id=? AND account=?", (lid, account)).fetchone()
        if not row:
            return self.error(ERR_NOT_FOUND)
        self.db.execute("DELETE FROM market WHERE id=?", (lid,))
        self.db.commit()
        return self.ok(struct.pack("<HH", *row))

    def collect(self, account, room):
        row = self.db.execute("SELECT money FROM wallets WHERE account=?", (account,)).fetchone()
        amount = min(row[0] if row else 0, max(0, room))
        if amount:
            self.credit(account, -amount)
            self.db.commit()
        return self.ok(struct.pack("<I", amount))

    def dispatch(self, account, t, p, names):
        if t == M_MARKET_SELL and len(p) >= 8:
            return self.sell(account, *struct.unpack_from("<HHI", p))
        if t == M_MARKET_BROWSE:
            return self.browse(account, names)
        if t == M_MARKET_BUY and len(p) >= 10:
            return self.buy(account, *struct.unpack_from("<IHI", p))
        if t == M_MARKET_MINE:
            return self.mine(account)
        if t == M_MARKET_CANCEL and len(p) >= 4:
            return self.cancel(account, struct.unpack_from("<I", p)[0])
        if t == M_MARKET_COLLECT and len(p) >= 4:
            return self.collect(account, struct.unpack_from("<I", p)[0])
        return None
