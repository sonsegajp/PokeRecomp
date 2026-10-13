"""Save codec/transaction tests. Synthetic RSE layouts are NOT cartridge testing.

Uses the existing captured FireRed Rattata record, then puts that record into
generated checksum-valid source-layout fixtures inside temporary directories.
No test Pokemon is installed in a user's save or in the launcher product.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest.mock import patch

import gen3

ROOT = Path(__file__).resolve().parents[1]
REAL_SAVE = ROOT / "build/deposit.sav"


def encode_text(text, length):
    chars = {char: byte for byte, char in gen3.metadata()["international"].items()}
    return bytes(chars[char] for char in text).ljust(length, b"\xff")[:length]


def record_with(record, *, personality=None, item=None, egg=None):
    """Test-only variation of an existing real record, never a product fixture."""
    raw = bytearray(record)
    pid, otid = struct.unpack_from("<II", raw)
    decoded = struct.pack("<12I", *(word ^ pid ^ otid for word in struct.unpack_from("<12I", raw, 32)))
    logical = [bytearray(decoded[index * 12:index * 12 + 12]) for index in gen3.metadata()["orders"][pid % 24]]
    if item is not None:
        struct.pack_into("<H", logical[0], 2, item)
    if egg is not None:
        word = struct.unpack_from("<I", logical[3], 4)[0]
        struct.pack_into("<I", logical[3], 4, word | 1 << 30 if egg else word & ~(1 << 30))
        raw[19] = raw[19] | 4 if egg else raw[19] & ~4
    pid = pid if personality is None else personality
    struct.pack_into("<I", raw, 0, pid)
    shuffled = bytearray(48)
    for logical_id, physical in enumerate(gen3.metadata()["orders"][pid % 24]):
        shuffled[physical * 12:physical * 12 + 12] = logical[logical_id]
    struct.pack_into("<H", raw, 28, sum(struct.unpack("<24H", shuffled)) & 65535)
    struct.pack_into("<12I", raw, 32, *(word ^ pid ^ otid for word in struct.unpack("<12I", shuffled)))
    return bytes(raw)


def fixture(game, record=None, counter=2):
    """Synthetic save scaffolding from source struct lengths and sector code."""
    sizes = gen3.section_sizes(game)
    sections = {i: bytearray(size) for i, size in enumerate(sizes)}
    # Distinguishable nonzero non-storage payload, with stable trainer header.
    for ident in range(5):
        sections[ident][:] = bytes(((offset * 17 + ident * 31 + 3) % 256 for offset in range(sizes[ident])))
    sections[0][:8] = encode_text("TEST", 8)
    sections[0][8] = 0
    storage = bytearray(gen3.STORAGE_SIZE)
    for box in range(14):
        storage[0x8344 + box * 9:0x8344 + (box + 1) * 9] = encode_text("BOX%d" % (box + 1), 9)
        storage[0x83C2 + box] = box % 4
    if record is not None:
        storage[4:84] = record
    for i in range(9):
        sections[i + 5][:] = storage[i * gen3.DATA_SIZE:i * gen3.DATA_SIZE + sizes[i + 5]]
    raw = bytearray(b"\xff" * 131072)
    for count in ((counter - 1) & 0xFFFFFFFF, counter):
        slot, rotation = count % 2, count % 14
        for ident in range(14):
            sector = bytearray(4096)
            sector[:sizes[ident]] = sections[ident]
            struct.pack_into("<HHII", sector, 0xFF4, ident, gen3.section_checksum(sections[ident]), gen3.SIGNATURE, count)
            offset = (slot * 14 + (rotation + ident) % 14) * 4096
            raw[offset:offset + 4096] = sector
    return bytes(raw)


class SaveTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.real = gen3.SaveImage(REAL_SAVE.read_bytes(), "firered")
        cls.record = cls.real.mon(0, 0)

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="gen3-test-")
        self.root = Path(self.temp.name)
        self.journals = self.root / "journals"

    def tearDown(self):
        self.temp.cleanup()

    def save(self, name, game, record=None):
        path = self.root / (name + ".sav")
        path.write_bytes(fixture(game, record))
        return path

    def move(self, source, source_game, target, target_game, **kwargs):
        return gen3.transfer_pokemon(source, source_game, 0, 0, target, target_game, 1, 5,
                                     journal_dir=self.journals, **kwargs)

    def test_real_firered_capture_decodes_without_game_process(self):
        parsed = gen3.inspect_save(REAL_SAVE, "firered", journal_dir=self.journals)
        mon = parsed["boxes"][0]["mons"][0]
        self.assertEqual((mon["species"], mon["nickname"], mon["level"], mon["nature"], mon["gender"]),
                         (19, "RATTATA", 5, "Quirky", "Male"))
        self.assertEqual([m["id"] for m in mon["moves"]], [33, 39])
        self.assertEqual(len(parsed["boxes"]), 14)
        self.assertEqual(sum(len(b["mons"]) for b in parsed["boxes"]), 420)
        self.assertEqual(parsed["validSlots"], 2)

    def test_all_24_original_encryption_orders(self):
        for pid in range(24):
            with self.subTest(permutation=pid):
                mon = gen3.decode_mon(record_with(self.record, personality=pid))
                self.assertTrue(mon["valid"])
                self.assertEqual((mon["species"], mon["experience"]), (19, 125))
                self.assertEqual([m["id"] for m in mon["moves"]], [33, 39])

    def test_all_five_layouts_and_twenty_cross_game_routes(self):
        for source_game in gen3.GAME_LAYOUTS:
            for target_game in gen3.GAME_LAYOUTS:
                if source_game == target_game:
                    continue
                with self.subTest(source=source_game, target=target_game):
                    source = self.save("source", source_game, self.record)
                    target = self.save("target", target_game)
                    before_source, before_target = source.read_bytes(), target.read_bytes()
                    result = self.move(source, source_game, target, target_game,
                                       source_revision=hashlib.sha256(before_source).hexdigest(),
                                       target_revision=hashlib.sha256(before_target).hexdigest())
                    a, b = gen3.SaveImage(source.read_bytes(), source_game), gen3.SaveImage(target.read_bytes(), target_game)
                    self.assertEqual(a.mon(0, 0), bytes(80))
                    self.assertEqual(b.mon(1, 5), self.record)
                    self.assertEqual((len(a.valid_slots), len(b.valid_slots)), (2, 2))
                    self.assertEqual((a.active["counter"], b.active["counter"]), (3, 3))
                    original = gen3.SaveImage(before_source, source_game)
                    self.assertTrue(all(a.sections[i] == original.sections[i] for i in range(5)))
                    self.assertEqual(source.read_bytes()[28 * 4096:], before_source[28 * 4096:])
                    self.assertEqual(target.read_bytes()[28 * 4096:], before_target[28 * 4096:])
                    self.assertEqual(Path(result["backups"][0]).read_bytes(), before_target)
                    self.assertEqual(Path(result["backups"][1]).read_bytes(), before_source)
                    # Arrival registers caught/seen, including both save-block mirrors.
                    index, bit = 18 // 8, 1 << (18 % 8)
                    self.assertTrue(b.sections[0][0x28 + index] & bit)
                    self.assertTrue(b.sections[0][0x5C + index] & bit)
                    block1 = b"".join(b.sections[i] for i in range(1, 5))
                    self.assertTrue(all(block1[offset + index] & bit for offset in gen3.DEX_SEEN_OFFSETS[target_game]))

    def test_same_save_move_is_one_atomic_generation(self):
        source = self.save("same", "firered", self.record)
        before = source.read_bytes()
        result = self.move(source, "firered", source, "firered")
        image = gen3.SaveImage(source.read_bytes(), "firered")
        self.assertEqual(image.mon(1, 5), self.record)
        self.assertEqual(image.mon(0, 0), bytes(80))
        self.assertEqual(image.active["counter"], 3)
        self.assertEqual(len(result["backups"]), 1)
        original = gen3.SaveImage(before, "firered")
        self.assertTrue(all(image.sections[i] == original.sections[i] for i in range(5)))

    def test_failure_after_destination_commit_restores_both_files(self):
        source = self.save("source", "firered", self.record)
        target = self.save("target", "emerald")
        original = (source.read_bytes(), target.read_bytes())
        def fail(stage):
            if stage == "after_destination": raise OSError("simulated source disk failure")
        with self.assertRaisesRegex(gen3.Gen3Error, "both original saves were restored"):
            self.move(source, "firered", target, "emerald", _fault=fail)
        self.assertEqual((source.read_bytes(), target.read_bytes()), original)

    def test_error_after_atomic_replace_still_recovers(self):
        source = self.save("source", "firered", self.record)
        target = self.save("target", "emerald")
        original = (source.read_bytes(), target.read_bytes())
        real_replace = gen3._replace_file
        failed = False
        def replace_then_fail(path, data):
            nonlocal failed
            real_replace(path, data)
            if Path(path) == target and not failed:
                failed = True
                raise OSError("simulated directory sync failure after replacement")
        with patch.object(gen3, "_replace_file", replace_then_fail):
            with self.assertRaisesRegex(gen3.Gen3Error, "both original saves were restored"):
                self.move(source, "firered", target, "emerald")
        self.assertEqual((source.read_bytes(), target.read_bytes()), original)

    def test_external_preflight_change_abandons_without_blocking_recovery(self):
        source = self.save("source", "firered", self.record)
        target = self.save("target", "emerald")
        original_target = target.read_bytes()
        external = source.read_bytes()[:-1] + b"Q"
        real_journal = gen3._write_journal
        def journal_then_external_edit(path, data):
            real_journal(path, data)
            if data["state"] == "prepared":
                source.write_bytes(external)
        with patch.object(gen3, "_write_journal", journal_then_external_edit):
            with self.assertRaisesRegex(gen3.Gen3Error, "changed during transfer preparation"):
                self.move(source, "firered", target, "emerald")
        self.assertEqual((source.read_bytes(), target.read_bytes()), (external, original_target))
        self.assertEqual(gen3.recover_pending_transactions(self.journals), [])
        journal = json.loads(next(self.journals.glob("*/journal.json")).read_text())
        self.assertEqual(journal["state"], "aborted")

    def test_hard_linked_save_alias_is_rejected(self):
        source = self.save("source", "firered", self.record)
        alias = self.root / "alias.sav"
        os.link(source, alias)
        before = source.read_bytes()
        with self.assertRaisesRegex(gen3.Gen3Error, "same file"):
            self.move(source, "firered", alias, "firered")
        self.assertEqual(source.read_bytes(), before)

    def test_interrupted_process_is_recovered_from_journal(self):
        source = self.save("source", "ruby", self.record)
        target = self.save("target", "leafgreen")
        original = (source.read_bytes(), target.read_bytes())
        def crash(stage):
            if stage == "after_destination": raise SystemExit("simulated power loss")
        with self.assertRaises(SystemExit):
            self.move(source, "ruby", target, "leafgreen", _fault=crash)
        self.assertEqual(source.read_bytes(), original[0])
        self.assertNotEqual(target.read_bytes(), original[1])
        # The next storage inspection rolls back the incomplete cross-save pair.
        gen3.inspect_save(source, "ruby", journal_dir=self.journals)
        self.assertEqual((source.read_bytes(), target.read_bytes()), original)

    def test_recovery_does_not_overwrite_new_external_edits(self):
        source = self.save("source", "firered", self.record)
        target = self.save("target", "emerald")
        def crash(stage): raise SystemExit()
        with self.assertRaises(SystemExit): self.move(source, "firered", target, "emerald", _fault=crash)
        external = source.read_bytes()[:-1] + b"Q"
        source.write_bytes(external)
        with self.assertRaisesRegex(gen3.Gen3Error, "changed externally"):
            gen3.recover_pending_transactions(self.journals)
        self.assertEqual(source.read_bytes(), external)

    def test_corruption_incomplete_generations_and_counter_wrap(self):
        raw = bytearray(fixture("emerald", self.record))
        latest = gen3.SaveImage(bytes(raw), "emerald")
        offset = latest.active["physical"][7] * 4096
        raw[offset] ^= 1
        fallback = gen3.SaveImage(bytes(raw), "emerald")
        self.assertEqual((fallback.active["counter"], len(fallback.valid_slots)), (1, 1))
        # Mixed counters remain invalid even when section payload checksums pass.
        raw = bytearray(fixture("emerald", self.record))
        struct.pack_into("<I", raw, 0xFFC, 4)
        self.assertEqual(gen3.SaveImage(bytes(raw), "emerald").active["counter"], 1)
        wrapped = gen3.SaveImage(fixture("ruby", self.record, 0xFFFFFFFF), "ruby")
        after = gen3.SaveImage(wrapped.updated_bytes(), "ruby")
        self.assertEqual(after.active["counter"], 0)

    def test_stale_revision_and_occupied_destination_never_write(self):
        source = self.save("source", "firered", self.record)
        target = self.save("target", "emerald")
        old = (source.read_bytes(), target.read_bytes())
        with self.assertRaisesRegex(gen3.Gen3Error, "changed since"):
            self.move(source, "firered", target, "emerald", source_revision="stale")
        occupied = gen3.SaveImage(target.read_bytes(), "emerald")
        occupied.set_mon(1, 5, self.record)
        target.write_bytes(occupied.updated_bytes())
        with self.assertRaisesRegex(gen3.Gen3Error, "empty destination"):
            self.move(source, "firered", target, "emerald")
        self.assertEqual(source.read_bytes(), old[0])

    def test_bad_checksums_bad_eggs_and_incompatible_items_rejected(self):
        for variant in ("checksum", "bad-egg", "mail", "enigma", "key-item"):
            record = bytearray(self.record)
            if variant == "checksum": record[44] ^= 1
            elif variant == "bad-egg": record[19] |= 1
            else: record = record_with(record, item={"mail": 121, "enigma": 175, "key-item": 349}[variant])
            source = self.save("source", "firered", bytes(record))
            target = self.save("target", "ruby")
            old = (source.read_bytes(), target.read_bytes())
            with self.subTest(variant=variant), self.assertRaises(gen3.Gen3Error):
                self.move(source, "firered", target, "ruby")
            self.assertEqual((source.read_bytes(), target.read_bytes()), old)

    def test_regular_held_item_and_egg_are_preserved_byte_for_byte(self):
        for egg in (False, True):
            record = record_with(self.record, item=139, egg=egg) # Oran Berry
            source = self.save("source", "sapphire", record)
            target = self.save("target", "firered")
            self.move(source, "sapphire", target, "firered")
            moved = gen3.SaveImage(target.read_bytes(), "firered").mon(1, 5)
            self.assertEqual(moved, record)
            self.assertEqual(gen3.decode_mon(moved)["isEgg"], egg)
            self.assertEqual(gen3.decode_mon(moved)["heldItemId"], 139)

    def test_64k_can_be_viewed_but_not_written(self):
        source = self.save("short", "ruby", self.record)
        source.write_bytes(source.read_bytes()[:65536])
        target = self.save("target", "firered")
        self.assertTrue(gen3.inspect_save(source, "ruby", journal_dir=self.journals)["readOnly"])
        old = (source.read_bytes(), target.read_bytes())
        with self.assertRaisesRegex(gen3.Gen3Error, "64 KiB"):
            self.move(source, "ruby", target, "firered")
        self.assertEqual((source.read_bytes(), target.read_bytes()), old)

    def test_original_growth_tables_and_character_map(self):
        self.assertEqual([gen3.experience_at_level(rate, 100) for rate in range(6)],
                         [1000000, 600000, 1640000, 1059860, 800000, 1250000])
        self.assertEqual(gen3.decode_text(bytes([0xBB, 0x1B, 0xB5, 0xB6, 0xFF])), "Aé♂♀")
        self.assertEqual(gen3.decode_text(bytes([1, 2, 3, 0xFF]), 1), "あいう")


if __name__ == "__main__":
    unittest.main(verbosity=2)
