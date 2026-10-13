"""world.py — a game server's world state: the day's shiny-boost type and
timed world events (mass outbreaks), announced to everyone in chat.

Outbreaks come from the games' own wild tables (legality_data*.json, dumped
by the game): a place with wild Pokémon, one of its rarer species from either
version, at that slot's levels. While it runs, about half the wild encounters
there are that species.

The boost type changes every day (UTC); wild Pokémon of that type are shiny
far more often while online.
"""
import glob
import json
import os
import random
import struct
import time

from protocol import *  # noqa: F401,F403

TYPE_MYSTERY = 9          # "???" -- never boosted
SPECIES_UNOWN = 201
LAND_RATES = (20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1)   # % per land slot (wild_encounters.json)
RARE_SHARE = 10            # a species is "rare" at a place if it is <= 10% of encounters there

# ASCII/UTF-8 -> the game's charset (chat text)
_CHARS = {" ": 0x00, "é": 0x1B, "&": 0x2D, "+": 0x2E, "(": 0x5C, ")": 0x5D, "!": 0xAB, "?": 0xAC, ".": 0xAD,
          "-": 0xAE, "'": 0xB4, "♂": 0xB5, "♀": 0xB6, ",": 0xB8, "/": 0xBA, ":": 0xF0}


def to_game(s, size=64):
    out = bytearray()
    for c in s:
        if "A" <= c <= "Z":
            out.append(0xBB + ord(c) - ord("A"))
        elif "a" <= c <= "z":
            out.append(0xD5 + ord(c) - ord("a"))
        elif "0" <= c <= "9":
            out.append(0xA1 + ord(c) - ord("0"))
        elif c in _CHARS:
            out.append(_CHARS[c])
    return bytes(out[:size - 1]) + b"\xff"


class World:
    def __init__(self, args):
        self.args = args
        here = os.path.dirname(os.path.abspath(__file__))
        dumps = [json.load(open(p, encoding="utf-8")) for p in sorted(glob.glob(os.path.join(here, "legality_data*.json")))]
        self.names = {int(k): v for k, v in dumps[0]["names"].items()}
        self.type_names = dumps[0]["type_names"]
        places = {}
        self.all_places = {}       # (group, num) -> (name, lowest level, highest level)
        for d in dumps:
            for w in d["wild"]:
                if "CHAMBER" in w["name"] or not w["land"]:
                    continue
                self.all_places[tuple(w["map"])] = (w["name"], min(x[1] for x in w["land"]), max(x[2] for x in w["land"]))
                place = places.setdefault(tuple(w["map"]), {"name": w["name"], "mons": {}})
                share, levels = {}, {}
                for (species, lo, hi), rate in zip(w["land"], LAND_RATES):
                    share[species] = share.get(species, 0) + rate
                    old = levels.get(species, (lo, hi))
                    levels[species] = (min(old[0], lo), max(old[1], hi))
                for species, pct in share.items():
                    if species and species != SPECIES_UNOWN and pct <= RARE_SHARE:
                        place["mons"].setdefault(species, levels[species])
        self.places = [(m, p["name"], sorted(p["mons"].items())) for m, p in sorted(places.items()) if p["mons"]]
        self.event = None          # (group, num, name, species, lo, hi, ends_at)
        self.next_event = time.time() + args.first_event * 60
        self.day = None

    # ---- the day's boost ----------------------------------------------------

    def boost_type(self):
        types = [t for t in range(len(self.type_names)) if t != TYPE_MYSTERY]
        day = int(time.time() // 86400)
        return random.Random(day).choice(types)

    # ---- state + announcements ----------------------------------------------------

    def state_frame(self):
        e = self.event
        left = max(0, int(e[6] - time.time())) if e else 0
        return frame(S_WORLD, struct.pack("<BHBBBHBBBI", self.boost_type(), self.args.shiny_odds,
                                          EVENT_OUTBREAK if e else EVENT_NONE,
                                          e[0] if e else 0, e[1] if e else 0, e[3] if e else 0,
                                          e[4] if e else 0, e[5] if e else 0, self.args.outbreak_percent, left))

    def boost_text(self):
        return "Today, %s-type POKéMON shine more often!" % self.type_names[self.boost_type()]

    def event_text(self):
        e = self.event
        mins = max(1, int((e[6] - time.time() + 59) // 60))
        return "Outbreak! %s on %s for %d min!" % (self.names[e[3]], e[2], mins)

    def announcements(self):
        """Chat lines for a player who just arrived."""
        lines = [self.boost_text()]
        if self.event:
            lines.append(self.event_text())
        return [system_chat(t) for t in lines]

    # ---- scheduling (call every few seconds) -------------------------------------------

    def update(self):
        """Returns frames to broadcast to everyone (state + chat), or []."""
        out = []
        now = time.time()
        day = int(now // 86400)
        if self.day is not None and day != self.day:
            out += [system_chat(self.boost_text())]
        changed = self.day != day
        self.day = day
        if self.event and now >= self.event[6]:
            out.append(system_chat("The outbreak of %s on %s is over." % (self.names[self.event[3]], self.event[2])))
            self.event = None
            changed = True
        if not self.event and now >= self.next_event and self.places:
            (group, num), name, mons = random.choice(self.places)
            species, (lo, hi) = random.choice(mons)
            if self.args.outbreak:            # an admin's pick: "GROUP.NUM:SPECIES"
                where, species = self.args.outbreak.split(":")
                group, num = (int(x) for x in where.split("."))
                species = int(species)
                name, lo, hi = self.all_places[(group, num)]
                self.args.outbreak = None
            self.event = (group, num, name, species, lo, hi, now + self.args.event_length * 60)
            self.next_event = now + self.args.event_every * 60
            out.append(system_chat(self.event_text()))
            print(f"world: outbreak of {self.names[species]} on {name} ({group}.{num}) Lv{lo}-{hi}")
            changed = True
        if changed:
            out.insert(0, self.state_frame())
        return out


def system_chat(s):
    """A chat line from the server itself (player id 0)."""
    return frame(S_CHAT_FROM, struct.pack("<H", 0) + to_game(s))
