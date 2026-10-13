"""Native explicit-save-file regression, using only isolated fixture copies."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "launcher"))
import gen3

EXE = ROOT / "dist/launcher-native/FireRedPC.exe"
REAL = ROOT / "build/deposit.sav"
WORK = Path(tempfile.mkdtemp(prefix="launcher-save-path-", dir=ROOT / "build"))
PROFILE = WORK / "settings profile"
PROFILE.mkdir()
SAVES = WORK / "linked cartridges"
SAVES.mkdir()
SELECTED = SAVES / "FireRed current cartridge.sav"
DECOY = PROFILE / "firered.sav"
shutil.copyfile(REAL, SELECTED)
shutil.copyfile(REAL, DECOY)
original = REAL.read_bytes()
record = gen3.SaveImage(original, "firered").mon(0, 0)
move_input = WORK / "move.txt"
move_input.write_text("120 A\n180 A\n240 RIGHT\n300 A\n360 A\n440 B\n520 B\n", encoding="ascii")


def command(*args, profile=PROFILE):
    return [str(EXE), "--headless", "--data-dir", str(profile), "--save-file", str(SELECTED), *map(str, args)]


def run(*args, expected=0, profile=PROFILE):
    result = subprocess.run(command(*args, profile=profile), capture_output=True, text=True, timeout=30)
    assert result.returncode == expected, (result.returncode, result.stderr)
    return result


result = run("--pc-boxes", "move", "--frames", 800, "--input", move_input)
assert "PC Box changes saved." in result.stderr
changed = gen3.SaveImage(SELECTED.read_bytes(), "firered")
assert changed.mon(0, 0) == bytes(80)
assert changed.mon(0, 1) == record
assert len(changed.valid_slots) == 2
assert DECOY.read_bytes() == original, "Default save changed instead of selected cartridge"
assert SELECTED.with_name(SELECTED.name + ".launcher.bak").read_bytes() == original
assert not SELECTED.with_name(SELECTED.name + ".tmp").exists()
assert not (PROFILE / "firered.sav.launcher.bak").exists()
direct = gen3.inspect_save(SELECTED, "firered", journal_dir=WORK / "journals")
assert direct["boxes"][0]["mons"][1]["recordHash"] == gen3.decode_mon(record)["recordHash"]

summary_input = WORK / "summary.txt"
summary_input.write_text("120 RIGHT\n200 A\n260 DOWN\n320 A\n", encoding="ascii")
committed = SELECTED.read_bytes()
run("--pc-boxes", "view", "--frames", 600, "--input", summary_input, "--dump", WORK / "linked-summary.bmp")
assert SELECTED.read_bytes() == committed

# A different settings directory must not bypass the actual cartridge mutex.
holder = subprocess.Popen(command("--paced", "--pc-boxes", "view", "--frames", 180),
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
try:
    time.sleep(0.4)
    run("--pc-boxes", "view", "--frames", 30, expected=4, profile=WORK / "other settings")
    try:
        gen3.transfer_pokemon(SELECTED, "firered", 0, 1, SELECTED, "firered", 0, 2,
                             journal_dir=WORK / "journals")
        raise AssertionError("Direct transfer bypassed the native save mutex")
    except gen3.Gen3Error as error:
        assert "Close the game" in str(error), str(error)
finally:
    assert holder.wait(timeout=10) == 0
assert SELECTED.read_bytes() == committed

missing = subprocess.run([str(EXE), "--headless", "--save-file"], capture_output=True, text=True, timeout=10)
assert missing.returncode == 2
print("PASS: linked alternate-filename save loaded, saved, backed up, reopened; default save untouched; shared native/direct mutex")
print("Visual capture:", WORK / "linked-summary.bmp")
