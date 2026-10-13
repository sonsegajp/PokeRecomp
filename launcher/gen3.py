"""Direct Gen III cartridge storage, derived from the local pret game sources.

Authority: game/{include/pokemon.h,include/global.h,src/save.c,src/pokemon.c},
and commit-pinned Ruby/Emerald references in launcher/gen3-reference/sources.json.
No game process, synthetic Pokemon, or emulated storage UI is used here.
"""
from __future__ import annotations

import ast
from contextlib import contextmanager
from dataclasses import dataclass
from functools import lru_cache
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import threading
import time
import uuid

ROOT = Path(__file__).resolve().parents[1]
JOURNAL_DIR = ROOT / "launcher/state/gen3-transfers"
SECTOR_SIZE = 0x1000
DATA_SIZE = 0xF80
SIGNATURE = 0x08012025
STORAGE_SIZE = 0x83D0
MON_SIZE = 80
BOX_COUNT = 14
BOX_SLOTS = 30
GAME_LAYOUTS = {
    "ruby": (0x890, 0x3AC0), "sapphire": (0x890, 0x3AC0),
    "firered": (0xF24, 0x3D68), "leafgreen": (0xF24, 0x3D68),
    "emerald": (0xF2C, 0x3D88),
}
DEX_SEEN_OFFSETS = {"ruby": (0x938, 0x3A8C), "sapphire": (0x938, 0x3A8C),
                    "firered": (0x5F8, 0x3A18), "leafgreen": (0x5F8, 0x3A18),
                    "emerald": (0x988, 0x3B24)}
_THREAD_LOCK = threading.RLock()


class Gen3Error(ValueError):
    """A save or requested transaction could not be validated safely."""


def _sha(data):
    return hashlib.sha256(data).hexdigest()


def section_sizes(game):
    if game not in GAME_LAYOUTS:
        raise Gen3Error("Choose Ruby, Sapphire, FireRed, LeafGreen, or Emerald.")
    block2, block1 = GAME_LAYOUTS[game]
    return [block2] + [min(DATA_SIZE, block1 - i * DATA_SIZE) for i in range(4)] + [
        min(DATA_SIZE, STORAGE_SIZE - i * DATA_SIZE) for i in range(9)]


def section_checksum(data):
    """Original CalculateChecksum: wrapping u32 sum, then fold to u16 once."""
    if len(data) % 4:
        raise Gen3Error("A save section has an invalid byte count.")
    total = sum(struct.unpack("<%dI" % (len(data) // 4), data)) & 0xFFFFFFFF
    return ((total >> 16) + total) & 0xFFFF


def _constants(path):
    text = path.read_text(encoding="utf-8")
    result = {}
    for key, value in re.findall(r"^#define[ \t]+(\w+)[ \t]+([^\s/]+)", text, re.M):
        try:
            result[key] = int(value, 0)
        except ValueError:
            if value in result:
                result[key] = result[value]
    return result


@lru_cache(maxsize=1)
def metadata():
    """Read original source tables; no independently guessed species database."""
    game = ROOT / "game"
    species_ids = _constants(game / "include/constants/species.h")
    pokemon = _constants(game / "include/constants/pokemon.h")
    item_ids = _constants(game / "include/constants/items.h")
    move_ids = _constants(game / "include/constants/moves.h")
    species_names = {species_ids[key]: name for key, name in re.findall(
        r'\[(SPECIES_\w+)\]\s*=\s*_\("([^"\n]*)"\)',
        (game / "src/data/text/species_names.h").read_text(encoding="utf-8")) if key in species_ids}
    moves = {move_ids[key]: name.rstrip("$") for key, name in re.findall(
        r'\[(MOVE_\w+)\]\s*=\s*_\("([^"\n]*)"\)',
        (game / "src/data/text/move_names.h").read_text(encoding="utf-8")) if key in move_ids}
    dex_text = (game / "include/constants/pokedex.h").read_text(encoding="utf-8").split("};", 1)[0]
    national = {key: index for index, key in enumerate(re.findall(r"\bNATIONAL_DEX_(\w+)\s*,", dex_text))}
    species = {}
    info = (game / "src/data/pokemon/species_info.h").read_text(encoding="utf-8")
    for symbol, body in re.findall(r"\[(SPECIES_\w+)\]\s*=\s*\{\s*\n(.*?)\n\s*\},", info, re.S):
        ident = species_ids.get(symbol)
        dex = national.get(symbol.removeprefix("SPECIES_"), 0)
        growth = re.search(r"\.growthRate\s*=\s*(\w+)", body)
        gender = re.search(r"\.genderRatio\s*=\s*([^,]+)", body)
        if ident is None or not 1 <= dex <= 386 or not growth or not gender:
            continue
        value = gender[1].strip()
        if value.startswith("PERCENT_FEMALE"):
            ratio = min(254, int(float(re.search(r"[\d.]+", value)[0]) * 255 / 100))
        else:
            ratio = pokemon.get(value, int(value) if value.isdecimal() else 255)
        types = re.search(r"\.types\s*=\s*\{([^}]+)\}", body)
        species[ident] = dict(name=species_names[ident], nationalDex=dex,
                             growthRate=pokemon[growth[1]], genderRatio=ratio,
                             types=[x.strip().removeprefix("TYPE_").title() for x in types[1].split(",")])
    if len(species) != 386:
        raise Gen3Error("The original species metadata is incomplete; rebuild the launcher source assets.")

    # The CASE arguments are logical substructure -> physical index, not the
    # other way around. Read all 24 original cases to avoid permutation drift.
    pokemon_c = (game / "src/pokemon.c").read_text(encoding="utf-8")
    orders = [None] * 24
    for row in re.findall(r"SUBSTRUCT_CASE\(\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)", pokemon_c):
        number, *order = map(int, row)
        orders[number] = order
    if any(order is None for order in orders):
        raise Gen3Error("The original Pokemon encryption table is incomplete.")

    international, japanese = {}, {}
    jp = False
    for line in (game / "charmap.txt").read_text(encoding="utf-8").splitlines():
        if line.startswith("@ Hiragana"):
            jp = True
        match = re.match(r"^('(?:\\.|[^'])*')\s*=\s*([0-9A-F]{2})(?:\s*(?:@.*)?)$", line)
        if match:
            if "\\" in match[1] and match[1] not in ("'\\\''", "'\\\\'"):
                continue  # Text control escapes are not nickname glyphs.
            char = ast.literal_eval(match[1])
            target = japanese if jp and any(ord(c) > 0x3000 for c in char) else international
            target.setdefault(int(match[2], 16), char)
    japanese = dict(international, **{str(k): v for k, v in japanese.items()})
    japanese = {int(k): v for k, v in japanese.items()}
    item_data = json.loads((game / "src/data/items.json").read_text(encoding="utf-8"))["items"]
    items = {}
    for entry in item_data:
        ident = item_ids.get(entry["itemId"])
        if ident is not None and entry["itemId"] != "ITEM_NONE":
            items[ident] = dict(name=entry["english"], symbol=entry["itemId"],
                                holdable=entry.get("importance", 0) == 0)
    items[0] = dict(name="None", symbol="ITEM_NONE", holdable=True)
    game_items = {"firered": item_ids, "leafgreen": item_ids}
    for title, repo in (("ruby", "pokeruby"), ("sapphire", "pokeruby"), ("emerald", "pokeemerald")):
        game_items[title] = _constants(ROOT / f"launcher/gen3-reference/{repo}/include/constants/items.h")
    nature_names = {value: key.removeprefix("NATURE_").title() for key, value in pokemon.items()
                    if key.startswith("NATURE_")}
    return dict(species=species, moves=moves, orders=orders, international=international,
                japanese=japanese, items=items, gameItems=game_items, natures=nature_names)


def decode_text(data, language=2):
    tables = metadata()
    table = tables["japanese"] if language == 1 else tables["international"]
    output, index = [], 0
    while index < len(data):
        value = data[index]
        if value == 0xFF:
            break
        if value == 0xFC and index + 1 < len(data) and data[index + 1] in (0x15, 0x16):
            table = tables["japanese"] if data[index + 1] == 0x15 else tables["international"]
            index += 2
            continue
        output.append(table.get(value, "\ufffd"))
        index += 1
    return "".join(output).rstrip()


def experience_at_level(growth, n):
    """Integer evaluation order from original experience_tables.h macros."""
    cube = n ** 3
    if growth == 0:
        return cube
    if growth == 1:
        if n <= 50: return (100 - n) * cube // 50
        if n <= 68: return (150 - n) * cube // 100
        if n <= 98: return ((1911 - 10 * n) // 3) * cube // 500
        return (160 - n) * cube // 100
    if growth == 2:
        if n <= 15: return ((n + 1) // 3 + 24) * cube // 50
        if n <= 36: return (n + 14) * cube // 50
        return (n // 2 + 32) * cube // 50
    if growth == 3: return max(0, 6 * cube // 5 - 15 * n * n + 100 * n - 140)
    if growth == 4: return 4 * cube // 5
    if growth == 5: return 5 * cube // 4
    raise Gen3Error("Unsupported experience growth table.")


def decode_mon(raw):
    if len(raw) != MON_SIZE:
        raise Gen3Error("A boxed Pokemon record must contain exactly 80 bytes.")
    if raw == bytes(MON_SIZE):
        return None
    tables = metadata()
    personality, trainer_id = struct.unpack_from("<II", raw)
    key = personality ^ trainer_id
    secure = struct.pack("<12I", *(word ^ key for word in struct.unpack_from("<12I", raw, 32)))
    valid_checksum = sum(struct.unpack("<24H", secure)) & 0xFFFF == struct.unpack_from("<H", raw, 28)[0]
    logical = [secure[index * 12:index * 12 + 12] for index in tables["orders"][personality % 24]]
    species_id, held_item, exp = struct.unpack_from("<HHI", logical[0])
    header_flags = raw[19]
    language = raw[18]
    basic = dict(species=species_id, nickname=decode_text(raw[8:18], language),
                 trainerName=decode_text(raw[20:27], language), language=language,
                 personality=personality, originalTrainerId=trainer_id,
                 trainerId=trainer_id & 0xFFFF, secretId=trainer_id >> 16,
                 recordHash=_sha(raw), valid=False, isEgg=bool(header_flags & 4))
    if not valid_checksum:
        return dict(basic, name="Invalid Pokemon", error="Pokemon checksum is invalid. Transfer is disabled.")
    if header_flags & 1:
        return dict(basic, name="Bad Egg", error="Bad Eggs cannot be transferred.")
    if species_id == 0 and not header_flags & 6:
        return None
    species = tables["species"].get(species_id)
    if not species or not header_flags & 2:
        return dict(basic, name="Unsupported Pokemon", error="This record is not a supported Gen III Pokemon.")
    iv_word = struct.unpack_from("<I", logical[3], 4)[0]
    egg = bool(iv_word & (1 << 30))
    if egg != bool(header_flags & 4):
        return dict(basic, name="Invalid Egg", error="Pokemon egg flags disagree. Transfer is disabled.")
    ratio = species["genderRatio"]
    gender = "Genderless" if ratio == 255 else "Female" if ratio == 254 or (personality & 255) < ratio else "Male"
    level = 1
    while level < 100 and exp >= experience_at_level(species["growthRate"], level + 1):
        level += 1
    move_ids = struct.unpack_from("<4H", logical[1])
    if any(move not in tables["moves"] for move in move_ids):
        return dict(basic, name=species["name"], error="This Pokemon has a move outside the original Gen III move table.")
    origin = struct.unpack_from("<H", logical[3], 2)[0]
    stats = ("hp", "attack", "defense", "speed", "specialAttack", "specialDefense")
    return dict(basic, valid=True, name=species["name"], speciesName=species["name"],
                nationalDex=species["nationalDex"], level=level, experience=exp,
                types=species["types"], gender=gender, isEgg=egg,
                shiny=((trainer_id >> 16) ^ (trainer_id & 0xFFFF) ^ (personality >> 16) ^ (personality & 0xFFFF)) < 8,
                nature=tables["natures"][personality % 25], heldItemId=held_item,
                heldItem=tables["items"].get(held_item, {}).get("name", "Unknown item %d" % held_item),
                moves=[dict(id=move, name=tables["moves"][move], pp=logical[1][8 + i]) for i, move in enumerate(move_ids) if move],
                ivs={stat: (iv_word >> (index * 5)) & 31 for index, stat in enumerate(stats)},
                evs={stat: logical[2][index] for index, stat in enumerate(stats)},
                friendship=logical[0][9], markings=raw[27], abilitySlot=iv_word >> 31,
                metLevel=origin & 127, metGame=(origin >> 7) & 15,
                metLocation=logical[3][1], pokeball=(origin >> 11) & 15,
                ribbons=struct.unpack_from("<I", logical[3], 8)[0])


@dataclass
class SaveImage:
    raw: bytes
    game: str
    path: Path | None = None

    def __post_init__(self):
        if len(self.raw) not in (65536, 131072):
            raise Gen3Error("Choose a 128 KiB Gen III cartridge save (.sav), not an emulator save state.")
        self.sizes = section_sizes(self.game)
        self.valid_slots = []
        self.slot_errors = []
        for slot in range(2 if len(self.raw) == 131072 else 1):
            try:
                self.valid_slots.append(self._read_slot(slot))
            except Gen3Error as error:
                self.slot_errors.append(str(error))
        if not self.valid_slots:
            raise Gen3Error("No complete, checksum-valid %s save slot was found. Check the selected game and save file." % self.game)
        self.active = self.valid_slots[0]
        for candidate in self.valid_slots[1:]:
            a, b = self.active["counter"], candidate["counter"]
            if a == 0xFFFFFFFF and b == 0 or (b > a and not (b == 0xFFFFFFFF and a == 0)):
                self.active = candidate
        self.sections = {i: bytearray(data) for i, data in self.active["sections"].items()}
        self.revision = _sha(self.raw)
        if self.storage()[0] >= BOX_COUNT:
            raise Gen3Error("The saved current box index is invalid.")

    def _read_slot(self, slot):
        sections, physical, counters = {}, {}, set()
        for physical_index in range(14):
            offset = (slot * 14 + physical_index) * SECTOR_SIZE
            sector = self.raw[offset:offset + SECTOR_SIZE]
            ident, check, signature, counter = struct.unpack_from("<HHII", sector, 0xFF4)
            if signature != SIGNATURE or ident >= 14 or ident in sections:
                raise Gen3Error("Save slot %d has a missing or duplicate section." % slot)
            if check != section_checksum(sector[:self.sizes[ident]]):
                raise Gen3Error("Save slot %d section %d failed its checksum." % (slot, ident))
            sections[ident] = sector[:self.sizes[ident]]
            physical[ident] = physical_index
            counters.add(counter)
        if len(counters) != 1:
            raise Gen3Error("Save slot %d mixes incomplete save generations." % slot)
        counter = counters.pop()
        if counter % 2 != slot:
            raise Gen3Error("Save slot and save counter disagree.")
        return dict(slot=slot, counter=counter, sections=sections, physical=physical)

    def storage(self):
        return bytearray(b"".join(self.sections[i] for i in range(5, 14)))

    def set_storage(self, storage):
        if len(storage) != STORAGE_SIZE:
            raise Gen3Error("Invalid Pokemon storage length.")
        for i in range(9):
            self.sections[5 + i] = bytearray(storage[i * DATA_SIZE:i * DATA_SIZE + self.sizes[5 + i]])

    def mon(self, box, slot):
        offset = _mon_offset(box, slot)
        return bytes(self.storage()[offset:offset + MON_SIZE])

    def set_mon(self, box, slot, data):
        if len(data) != MON_SIZE:
            raise Gen3Error("Invalid Pokemon record length.")
        storage = self.storage()
        offset = _mon_offset(box, slot)
        storage[offset:offset + MON_SIZE] = data
        self.set_storage(storage)

    def register_arrival(self, mon):
        if mon["isEgg"]:
            return
        dex = mon["nationalDex"] - 1
        index, bit = dex // 8, 1 << (dex % 8)
        block1 = bytearray(b"".join(self.sections[i] for i in range(1, 5)))
        first_caught = not (self.sections[0][0x28 + index] & bit and
                            self.sections[0][0x5C + index] & bit and
                            all(block1[offset + index] & bit for offset in DEX_SEEN_OFFSETS[self.game]))
        self.sections[0][0x28 + index] |= bit
        self.sections[0][0x5C + index] |= bit
        if first_caught and mon["nationalDex"] in (201, 327):
            struct.pack_into("<I", self.sections[0], 0x1C if mon["nationalDex"] == 201 else 0x20, mon["personality"])
        for offset in DEX_SEEN_OFFSETS[self.game]:
            block1[offset + index] |= bit
        for i in range(4):
            self.sections[1 + i] = block1[i * DATA_SIZE:i * DATA_SIZE + self.sizes[1 + i]]

    def updated_bytes(self):
        if len(self.raw) != 131072:
            raise Gen3Error("64 KiB saves can be viewed. Export a full 128 KiB cartridge save before moving Pokemon.")
        result = bytearray(self.raw)
        counter = (self.active["counter"] + 1) & 0xFFFFFFFF
        slot = counter % 2
        rotation = (self.active["physical"][0] + 1) % 14
        for ident in range(14):
            previous_offset = (self.active["slot"] * 14 + self.active["physical"][ident]) * SECTOR_SIZE
            sector = bytearray(self.raw[previous_offset:previous_offset + SECTOR_SIZE])
            sector[:self.sizes[ident]] = self.sections[ident]
            struct.pack_into("<HHII", sector, 0xFF4, ident,
                             section_checksum(self.sections[ident]), SIGNATURE, counter)
            offset = (slot * 14 + (rotation + ident) % 14) * SECTOR_SIZE
            result[offset:offset + SECTOR_SIZE] = sector
        # Do not touch Hall of Fame, Trainer Tower/Hill, or recorded battle sectors.
        validated = SaveImage(bytes(result), self.game)
        if validated.active["counter"] != counter:
            raise Gen3Error("Updated save generation could not be verified.")
        return bytes(result)


def _mon_offset(box, slot):
    if type(box) is not int or type(slot) is not int or not 0 <= box < 14 or not 0 <= slot < 30:
        raise Gen3Error("Choose a box from 1-14 and a slot from 1-30.")
    return 4 + (box * 30 + slot) * MON_SIZE


def _read_save(path, game):
    path = Path(path).expanduser().resolve()
    if path.suffix.lower() != ".sav" or not path.is_file():
        raise Gen3Error("Register an existing .sav cartridge file for this game.")
    return SaveImage(path.read_bytes(), game, path)


def _summary(save):
    storage = save.storage()
    boxes, invalid = [], 0
    for box in range(14):
        mons = []
        for slot in range(30):
            mon = decode_mon(save.mon(box, slot))
            if mon:
                mon["box"], mon["slot"] = box, slot
                invalid += not mon["valid"]
            mons.append(mon)
        name = decode_text(storage[0x8344 + box * 9:0x8344 + (box + 1) * 9])
        boxes.append(dict(index=box, name=name or "BOX %d" % (box + 1),
                          wallpaper=storage[0x83C2 + box], mons=mons,
                          count=sum(mon is not None for mon in mons)))
    block2 = save.sections[0]
    trainer_id = struct.unpack_from("<I", block2, 10)[0]
    return dict(game=save.game, revision=save.revision, currentBox=storage[0], boxes=boxes,
                trainer=dict(name=decode_text(block2[:8]), id=trainer_id & 65535,
                             fullId=trainer_id, gender="Female" if block2[8] else "Male"),
                saveCounter=save.active["counter"], activeSlot=save.active["slot"],
                validSlots=len(save.valid_slots), readOnly=len(save.raw) != 131072,
                invalidMons=invalid, count=sum(box["count"] for box in boxes),
                warnings=(["Only one coherent save generation is available; the other slot will be rebuilt on the next move."]
                          if len(save.valid_slots) == 1 and len(save.raw) == 131072 else []) +
                         (["64 KiB export: view only. Register a full 128 KiB save to move Pokemon."] if len(save.raw) != 131072 else []))


@contextmanager
def operation_lock(journal_dir=None):
    """One durable interprocess transaction lock shared by all launcher hosts."""
    directory = Path(journal_dir) if journal_dir else JOURNAL_DIR
    directory.mkdir(parents=True, exist_ok=True)
    with _THREAD_LOCK:
        with (directory / "operations.lock").open("a+b") as lock:
            lock.seek(0)
            if not lock.read(1):
                lock.write(b"0"); lock.flush()
            lock.seek(0)
            try:
                if os.name == "nt":
                    import msvcrt
                    msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
                else:
                    import fcntl
                    fcntl.flock(lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            except OSError as error:
                raise Gen3Error("Another storage operation is in progress. Try again when it finishes.") from error
            try:
                yield directory
            finally:
                lock.seek(0)
                if os.name == "nt":
                    msvcrt.locking(lock.fileno(), msvcrt.LK_UNLCK, 1)
                else:
                    fcntl.flock(lock.fileno(), fcntl.LOCK_UN)


@contextmanager
def _native_save_locks(paths):
    """Use the same save-path mutex as the rebuilt native game on Windows."""
    handles = []
    if os.name == "nt":
        import ctypes
        from ctypes import wintypes
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel.CreateMutexW.argtypes = [ctypes.c_void_p, wintypes.BOOL, wintypes.LPCWSTR]
        kernel.CreateMutexW.restype = wintypes.HANDLE
        kernel.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
        kernel.ReleaseMutex.argtypes = [wintypes.HANDLE]
        kernel.CloseHandle.argtypes = [wintypes.HANDLE]
    try:
        for path in sorted(set(map(str, paths))):
            if os.name != "nt":
                continue
            value = 2166136261
            for byte in path.lower().encode("mbcs"):
                value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
            handle = kernel.CreateMutexW(None, False, "Local\\FireRedPC.Save.%08x" % value)
            if not handle:
                raise Gen3Error("Could not lock the selected save.")
            if kernel.WaitForSingleObject(handle, 0) not in (0, 0x80):
                kernel.CloseHandle(handle)
                raise Gen3Error("Close the game using this save before moving Pokemon.")
            handles.append(handle)
        yield
    finally:
        for handle in reversed(handles):
            kernel.ReleaseMutex(handle); kernel.CloseHandle(handle)


def _write_durable(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def _replace_file(path, data):
    path = Path(path)
    temp = path.with_name(path.name + ".gen3-" + uuid.uuid4().hex + ".tmp")
    _write_durable(temp, data)
    if os.name == "nt":
        import ctypes
        from ctypes import wintypes
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel.MoveFileExW.argtypes = [wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.DWORD]
        if not kernel.MoveFileExW(str(temp), str(path), 0x1 | 0x8):
            raise ctypes.WinError(ctypes.get_last_error())
    else:
        os.replace(temp, path)
        descriptor = os.open(path.parent, os.O_RDONLY)
        try:
            os.fsync(descriptor)
        finally:
            os.close(descriptor)


def _write_journal(path, journal):
    _replace_file(path, (json.dumps(journal, indent=2) + "\n").encode("utf-8"))


def _rollback(journal, journal_path):
    # Check EVERY file before overwriting ANY file. Never overwrite a newer
    # externally edited save while recovering an interrupted transaction.
    for entry in journal["files"]:
        path = Path(entry["path"])
        if not path.is_file() or _sha(path.read_bytes()) not in (entry["before"], entry["after"]):
            raise Gen3Error("Transfer recovery needs review because a save changed externally. Backups: " + str(journal_path.parent))
        backup = Path(entry["backup"]).read_bytes()
        if _sha(backup) != entry["before"]:
            raise Gen3Error("A transfer backup failed validation. Preserve this journal: " + str(journal_path))
    # Restore the source before the destination: no intermediate deletion.
    for entry in reversed(journal["files"]):
        path = Path(entry["path"])
        if _sha(path.read_bytes()) != entry["before"]:
            _replace_file(path, Path(entry["backup"]).read_bytes())
        if _sha(path.read_bytes()) != entry["before"]:
            raise Gen3Error("Transfer rollback could not be verified: " + str(journal_path))
    journal["state"] = "rolled_back"
    _write_journal(journal_path, journal)


def _recover(directory):
    recovered = []
    for path in sorted(directory.glob("*/journal.json")):
        try:
            journal = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError) as error:
            raise Gen3Error("A storage transaction journal could not be read: " + str(path)) from error
        if journal.get("state") in ("committed", "rolled_back", "aborted"):
            continue
        with _native_save_locks([Path(entry["path"]) for entry in journal["files"]]):
            _rollback(journal, path)
        recovered.append(journal["id"])
    return recovered


def recover_pending_transactions(journal_dir=None):
    with operation_lock(journal_dir) as directory:
        return _recover(directory)


def inspect_save(path, game="firered", *, journal_dir=None):
    with operation_lock(journal_dir) as directory:
        _recover(directory)
        return _summary(_read_save(path, game))


def _compatible_item(mon, source_game, target_game, same_save):
    item_id = mon["heldItemId"]
    if not item_id:
        return
    tables = metadata()
    item = tables["items"].get(item_id)
    if not item or not item["holdable"]:
        raise Gen3Error("This Pokemon holds an invalid or key item. Remove it in the original game first.")
    for game in (source_game, target_game):
        if tables["gameItems"][game].get(item["symbol"]) != item_id:
            raise Gen3Error("The held item is not compatible with %s. Remove it in the original game first." % game)
    if not same_save and (121 <= item_id <= 132 or item_id == 175):
        raise Gen3Error("Remove Mail or the Enigma Berry before moving between saves; its associated data belongs to that save.")


def transfer_pokemon(source_path, source_game, source_box, source_slot,
                     target_path, target_game, target_box, target_slot,
                     source_revision=None, target_revision=None, *, journal_dir=None, _fault=None):
    source_path, target_path = Path(source_path).resolve(), Path(target_path).resolve()
    same_save = source_path == target_path
    if not same_save and source_path.exists() and target_path.exists() and os.path.samefile(source_path, target_path):
        raise Gen3Error("Both registered save paths refer to the same file. Use a single save registration.")
    if same_save and source_game != target_game:
        raise Gen3Error("One save file cannot be registered as two different games for a transfer.")
    if same_save and (source_box, source_slot) == (target_box, target_slot):
        raise Gen3Error("Choose a different empty destination slot.")
    with operation_lock(journal_dir) as directory:
        _recover(directory)
        with _native_save_locks([source_path, target_path]):
            source = _read_save(source_path, source_game)
            target = source if same_save else _read_save(target_path, target_game)
            for save, expected in ((source, source_revision), (target, target_revision)):
                if expected is not None and expected != save.revision:
                    raise Gen3Error("The save changed since these boxes were opened. Refresh both saves and choose the Pokemon again.")
            record = source.mon(source_box, source_slot)
            mon = decode_mon(record)
            if mon is None:
                raise Gen3Error("The source slot is empty.")
            if not mon["valid"]:
                raise Gen3Error(mon["error"])
            destination = target.mon(target_box, target_slot)
            if decode_mon(destination) is not None:
                raise Gen3Error("Choose an empty destination slot. Existing Pokemon will never be overwritten.")
            _compatible_item(mon, source_game, target_game, same_save)
            target.set_mon(target_box, target_slot, record)
            source.set_mon(source_box, source_slot, bytes(MON_SIZE))
            if not same_save:
                target.register_arrival(mon)
            target_after = target.updated_bytes()
            source_after = target_after if same_save else source.updated_bytes()
            # Verify exact destination record and empty source in regenerated,
            # checksum-validated files before preparing any filesystem mutation.
            if (SaveImage(target_after, target_game).mon(target_box, target_slot) != record or
                    SaveImage(source_after, source_game).mon(source_box, source_slot) != bytes(MON_SIZE)):
                raise Gen3Error("The prepared transfer failed its Pokemon integrity check.")
            txn_id = "%d-%s" % (time.time_ns(), uuid.uuid4().hex)
            txn_dir = directory / txn_id
            txn_dir.mkdir()
            pairs = [(target, target_after)] if same_save else [(target, target_after), (source, source_after)]
            journal = dict(id=txn_id, state="prepared", files=[], recordHash=_sha(record),
                           source=dict(game=source_game, box=source_box, slot=source_slot),
                           target=dict(game=target_game, box=target_box, slot=target_slot))
            for index, (save, after) in enumerate(pairs):
                backup = txn_dir / ("%d-%s.sav.bak" % (index, save.game))
                _write_durable(backup, save.raw)
                journal["files"].append(dict(path=str(save.path), game=save.game, backup=str(backup.resolve()),
                                              before=save.revision, after=_sha(after)))
            journal_path = txn_dir / "journal.json"
            _write_journal(journal_path, journal)
            # No target has been written yet, so an external preflight change
            # can be abandoned without a rollback or blocking later recovery.
            if any(save.path.read_bytes() != save.raw for save, _ in pairs):
                journal["state"] = "aborted"
                _write_journal(journal_path, journal)
                raise Gen3Error("A save changed during transfer preparation. Refresh and try again.")
            written = 0
            try:
                for index, (save, after) in enumerate(pairs):
                    if save.path.read_bytes() != save.raw:
                        raise Gen3Error("A save changed before its transfer commit.")
                    _replace_file(save.path, after)
                    written += 1
                    if save.path.read_bytes() != after:
                        raise Gen3Error("The written save failed byte-for-byte verification.")
                    if _fault:
                        _fault("after_destination" if index == 0 else "after_source")
                journal["state"] = "committed"
                _write_journal(journal_path, journal)
            except Exception as error:
                # A replacement helper can fail after the rename (for example
                # during directory fsync). Check disk state before declaring
                # that nothing was written; retain recovery for uncertainty.
                untouched = not written and all(save.path.is_file() and save.path.read_bytes() == save.raw
                                                for save, _ in pairs)
                if untouched:
                    journal["state"] = "aborted"
                    _write_journal(journal_path, journal)
                    raise Gen3Error("Transfer was not started; no save was changed. " + str(error)) from error
                try:
                    _rollback(journal, journal_path)
                except Exception as rollback_error:
                    raise Gen3Error("Transfer interrupted; automatic recovery is pending. Preserve backups at %s. %s" % (txn_dir, rollback_error)) from error
                raise Gen3Error("Transfer was not completed; both original saves were restored. " + str(error)) from error
            return dict(ok=True, transaction=txn_id, sourceRevision=_sha(source_after),
                        targetRevision=_sha(target_after), pokemon=mon,
                        backups=[entry["backup"] for entry in journal["files"]],
                        message="%s moved successfully." % (mon["nickname"] or mon["name"]))
