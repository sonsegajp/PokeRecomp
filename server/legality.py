"""legality.py — is a Gen 3 Pokémon one the game could have made?

Rules come from legality_data.json, dumped from the game's own tables
(FireRedPC.exe --dump-legality), so server and game always agree. Other
versions' dumps (legality_data_lg.json from LeafGreenPC.exe) add the moves
only they teach: a Pokémon is legal if any version could have made it.

    check(raw100) -> None if legal, else a short reason string

Checks: checksum, species, level vs experience, EV totals, every move
learnable by the species line (level-up / TM-HM / tutor / egg), ability slot,
held item, and (party Pokémon) the stats recomputed from base stats, IVs,
EVs, nature and level.
"""
import json
import os
import struct

_HERE = os.path.dirname(os.path.abspath(__file__))
DATA = json.load(open(os.path.join(_HERE, "legality_data.json")))
SPECIES = {int(k): v for k, v in DATA["species"].items()}
for _name in sorted(os.listdir(_HERE)):
    if _name.startswith("legality_data_") and _name.endswith(".json"):
        for _k, _v in json.load(open(os.path.join(_HERE, _name)))["species"].items():
            _mine = SPECIES[int(_k)]["levelup"]
            _mine.extend(m for m in _v["levelup"] if m not in _mine)
SMEARGLE, SHEDINJA = 235, 303

# Substructure orders by personality % 24 (G = growth, A = attacks, E = EVs/condition, M = misc)
ORDERS = ["GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA", "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
          "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG", "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"]

# Nature stat modifiers: (atk, def, spe, spa, spd), as in pokemon.c sNatureStatTable
NATURES = [(0, 0, 0, 0, 0), (1, -1, 0, 0, 0), (1, 0, -1, 0, 0), (1, 0, 0, -1, 0), (1, 0, 0, 0, -1),
           (-1, 1, 0, 0, 0), (0, 0, 0, 0, 0), (0, 1, -1, 0, 0), (0, 1, 0, -1, 0), (0, 1, 0, 0, -1),
           (-1, 0, 1, 0, 0), (0, -1, 1, 0, 0), (0, 0, 0, 0, 0), (0, 0, 1, -1, 0), (0, 0, 1, 0, -1),
           (-1, 0, 0, 1, 0), (0, -1, 0, 1, 0), (0, 0, -1, 1, 0), (0, 0, 0, 0, 0), (0, 0, 0, 1, -1),
           (-1, 0, 0, 0, 1), (0, -1, 0, 0, 1), (0, 0, -1, 0, 1), (0, 0, 0, -1, 1), (0, 0, 0, 0, 0)]

# pre-evolutions, so a Pokémon may keep moves it learned before evolving
_PREVO = {}
for _s, _info in SPECIES.items():
    for _t in _info["evolves_to"]:
        _PREVO.setdefault(_t, set()).add(_s)


def _line(species):
    seen, todo = set(), [species]
    while todo:
        s = todo.pop()
        if s not in seen:
            seen.add(s)
            todo.extend(_PREVO.get(s, ()))
    return seen


def _learnable(species, level):
    moves = set()
    for s in _line(species):
        info = SPECIES[s]
        moves.update(m for m, lv in info["levelup"] if lv <= level or s != species)
        moves.update(info["tmhm"])
        moves.update(info["tutor"])
        moves.update(info["egg"])
    return moves


def _level_for_exp(growth, exp):
    table = DATA["exp"][growth]
    level = 1
    while level < DATA["max_level"] and exp >= table[level + 1]:
        level += 1
    return level


def decode(raw):
    """Box part of a 100- or 80-byte Pokémon -> dict, or None if not one."""
    if len(raw) < 80:
        return None
    personality, ot_id = struct.unpack_from("<II", raw, 0)
    flags = raw[19]
    checksum = struct.unpack_from("<H", raw, 28)[0]
    key = personality ^ ot_id
    secure = bytearray(raw[32:80])
    for i in range(0, 48, 4):
        struct.pack_into("<I", secure, i, struct.unpack_from("<I", secure, i)[0] ^ key)
    if sum(struct.unpack("<24H", bytes(secure))) & 0xFFFF != checksum:
        return None
    order = ORDERS[personality % 24]
    sub = {order[i]: bytes(secure[i * 12:(i + 1) * 12]) for i in range(4)}
    species, item, exp = struct.unpack_from("<HHI", sub["G"], 0)
    moves = struct.unpack_from("<4H", sub["A"], 0)
    pp = sub["A"][8:12]
    evs = sub["E"][0:6]
    ivw = struct.unpack_from("<I", sub["M"], 4)[0]
    ivs = [(ivw >> (5 * i)) & 31 for i in range(6)]   # hp atk def spe spa spd
    return dict(personality=personality, ot_id=ot_id, species=species, item=item, exp=exp,
                moves=moves, pp=pp, evs=evs, ivs=ivs, is_egg=bool(ivw >> 30 & 1) or bool(flags & 4),
                ability=ivw >> 31 & 1, bad_egg=bool(flags & 1))


def _stat(base, iv, ev, level, nature_mod):
    v = (2 * base + iv + ev // 4) * level // 100 + 5
    if nature_mod > 0:
        v = v * 110 // 100
    elif nature_mod < 0:
        v = v * 90 // 100
    return v


def check(raw):
    mon = decode(raw)
    if mon is None:
        return None                      # not a Pokémon (or a harmless Bad Egg)
    s = mon["species"]
    if s == 0:
        return None                      # an empty party slot
    if s >= DATA["num_species"] or DATA["old_unown"][0] <= s <= DATA["old_unown"][1]:
        return "species %d" % s
    info = SPECIES[s]
    if mon["item"] >= DATA["items_count"]:
        return "item %d" % mon["item"]
    if sum(mon["evs"]) > 510:
        return "EVs total %d" % sum(mon["evs"])
    if mon["exp"] > DATA["exp"][info["growth"]][DATA["max_level"]]:
        return "experience"
    level = _level_for_exp(info["growth"], mon["exp"])
    if mon["ability"] and not info["abilities"][1]:
        return "ability slot"
    moves = [m for m in mon["moves"] if m]
    if any(m >= DATA["moves_count"] for m in moves) or len(set(moves)) != len(moves):
        return "move ids"
    if mon["moves"][:len(moves)] != tuple(moves):
        return "move gaps"
    if s != SMEARGLE:
        ok = _learnable(s, level)
        bad = [m for m in moves if m not in ok]
        if bad:
            return "moves %s not learnable" % bad
    if len(raw) >= 100 and not mon["is_egg"]:
        party_level = raw[84]
        if party_level != level:
            return "level %d vs %d" % (party_level, level)
        max_hp, atk, dfn, spe, spa, spd = struct.unpack_from("<6H", raw, 88)
        b, iv, ev = info["base"], mon["ivs"], mon["evs"]
        nat = NATURES[mon["personality"] % 25]
        hp = 1 if s == SHEDINJA else (2 * b[0] + iv[0] + ev[0] // 4) * level // 100 + level + 10
        want = (hp, _stat(b[1], iv[1], ev[1], level, nat[0]), _stat(b[2], iv[2], ev[2], level, nat[1]),
                _stat(b[3], iv[3], ev[3], level, nat[2]), _stat(b[4], iv[4], ev[4], level, nat[3]),
                _stat(b[5], iv[5], ev[5], level, nat[4]))
        if (max_hp, atk, dfn, spe, spa, spd) != want:
            return "stats %s vs %s" % ((max_hp, atk, dfn, spe, spa, spd), want)
    return None


def check_block(block):
    """A link block: every Pokémon in a 200-byte party block must be legal."""
    if len(block) != 200:
        return None
    for off in (0, 100):
        why = check(bytes(block[off:off + 100]))
        if why:
            return why
    return None


if __name__ == "__main__":
    print(len(SPECIES), "species loaded")
