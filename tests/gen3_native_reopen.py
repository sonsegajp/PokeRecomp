"""Reopen isolated direct-save transfers with the original FireRed storage code.

The Emerald source is synthetic save scaffolding containing the existing real
FireRed fixture's record. This is not an Emerald cartridge compatibility claim.
No user save or launcher registry is read or modified.
"""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "launcher"))
import gen3
from test_gen3 import fixture

WORK = Path(tempfile.mkdtemp(prefix="gen3-native-reopen-", dir=ROOT / "build"))
EXE = ROOT / "dist/launcher-native/FireRedPC.exe"
REAL = ROOT / "build/deposit.sav"
original = gen3.SaveImage(REAL.read_bytes(), "firered")
record = original.mon(0, 0)
expected = gen3.decode_mon(record)
summary_input = WORK / "summary.txt"
summary_input.write_text("120 RIGHT\n200 A\n260 DOWN\n320 A\n", encoding="ascii")
box_input = WORK / "box.txt"
box_input.write_text("120 RIGHT\n", encoding="ascii")
results = []

for route in ("within-firered", "emerald-layout-to-firered"):
    profile = WORK / route
    profile.mkdir()
    target = profile / "firered.sav"
    shutil.copyfile(REAL, target)
    source = target if route == "within-firered" else WORK / "emerald-layout.sav"
    source_game = "firered" if source == target else "emerald"
    if source != target:
        source.write_bytes(fixture(source_game, record))
    result = gen3.transfer_pokemon(source, source_game, 0, 0, target, "firered", 0, 1,
                                   journal_dir=WORK / "transactions")
    revised = gen3.SaveImage(target.read_bytes(), "firered")
    assert revised.mon(0, 1) == record
    assert gen3.decode_mon(revised.mon(0, 1)) == expected
    assert gen3.SaveImage(source.read_bytes(), source_game).mon(0, 0) == bytes(80)
    assert len(revised.valid_slots) == 2
    committed = target.read_bytes()
    for screen, inputs, frames in (("box", box_input, 240), ("summary", summary_input, 600)):
        capture = profile / (screen + ".bmp")
        native = subprocess.run([str(EXE), "--headless", "--data-dir", str(profile),
                                 "--pc-boxes", "view", "--frames", str(frames),
                                 "--input", str(inputs), "--dump", str(capture)],
                                capture_output=True, text=True, timeout=30)
        (profile / (screen + ".log")).write_text(native.stderr, encoding="utf-8")
        assert native.returncode == 0, native.stderr
        assert capture.is_file(), native.stderr
        assert target.read_bytes() == committed, "Read-only native inspection changed the save"
    results.append(dict(route=route, counter=revised.active["counter"], validSlots=2,
                        recordHash=hashlib.sha256(record).hexdigest(),
                        nativeBox=str(profile / "box.bmp"), nativeSummary=str(profile / "summary.bmp"),
                        decoded=expected, transaction=result["transaction"]))

(WORK / "evidence.json").write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
print("PASS: original native storage reopened both direct-transfer saves; record and summary fields unchanged")
print("Caveat: Emerald source layout is generated test scaffolding, not a real Emerald cartridge save.")
print("Visual captures:", WORK)
