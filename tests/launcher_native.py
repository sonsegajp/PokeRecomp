"""Native launcher acceptance using an isolated copy of the existing save fixture.

Run: python tests/launcher_native.py
Requires build/deposit.sav (the existing capture/deposit test save).
Captures remain under build/launcher-acceptance-* for visual inspection.
"""
from pathlib import Path
import hashlib
import shutil
import struct
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "dist/launcher-native/FireRedPC.exe"
FIXTURE = ROOT / "build/deposit.sav"
WORK = Path(tempfile.mkdtemp(prefix="launcher-acceptance-", dir=ROOT / "build"))
PROFILE = WORK / "profile"
PROFILE.mkdir()
shutil.copyfile(FIXTURE, PROFILE / "firered.sav")


def run(*args, profile=PROFILE, expected=0):
    result = subprocess.run(
        [str(EXE), "--headless", "--data-dir", str(profile), *map(str, args)],
        capture_output=True, text=True, timeout=30,
    )
    assert result.returncode == expected, (result.returncode, result.stderr)
    return result


def latest_sectors(path):
    raw = path.read_bytes()
    assert len(raw) == 131072
    sectors = {}
    for offset in range(0, 28 * 4096, 4096):
        sector = raw[offset:offset + 4096]
        ident, checksum, signature, counter = struct.unpack_from("<HHII", sector, 4084)
        if signature == 0x08012025 and ident < 14:
            if ident not in sectors or counter > sectors[ident][0]:
                sectors[ident] = (counter, sector[:3968])
    assert len(sectors) == 14
    return sectors


def script(name, contents):
    path = WORK / name
    path.write_text(contents, encoding="ascii")
    return path


run("--pc-boxes", "view", "--frames", 240, "--dump", WORK / "boxes-before.bmp")
run("--pc-boxes", "view", "--frames", 240, profile=WORK / "empty", expected=2)
assert not (WORK / "empty/firered.sav").exists()

before = latest_sectors(FIXTURE)
before_storage = b"".join(before[i][1] for i in range(5, 14))
assert before_storage[4:84] != bytes(80), "Fixture must have a Pokemon in box 1 slot 1"
assert before_storage[84:164] == bytes(80), "Fixture box 1 slot 2 must be empty"
inputs = script("move.txt", "120 A\n180 A\n240 RIGHT\n300 A\n360 A\n440 B\n520 B\n")
result = run("--pc-boxes", "move", "--frames", 800, "--input", inputs)
assert "PC Box changes saved." in result.stderr
after = latest_sectors(PROFILE / "firered.sav")
after_storage = b"".join(after[i][1] for i in range(5, 14))
assert after[0][0] == before[0][0] + 1
assert after_storage[4:84] == bytes(80)
assert after_storage[84:164] == before_storage[4:84]
assert all(before[i][1] == after[i][1] for i in range(5)), "Party/map/trainer changed"
assert (PROFILE / "firered.sav.launcher.bak").read_bytes() == FIXTURE.read_bytes()

summary = script("summary.txt", "120 RIGHT\n200 A\n260 DOWN\n320 A\n")
run("--pc-boxes", "view", "--frames", 600, "--input", summary,
    "--dump", WORK / "summary-after-reload.bmp")
run("--mystery-gift", "--frames", 240, "--dump", WORK / "mystery-gift.bmp")
gift_exit = script("gift-exit.txt", "120 B\n")
result = run("--mystery-gift", "--frames", 500, "--input", gift_exit)
assert "Returned from Mystery Gift." in result.stderr

# Keep one finite, isolated headless process alive while checking the mutex.
holder = subprocess.Popen([str(EXE), "--headless", "--paced", "--data-dir", str(PROFILE),
                           "--pc-boxes", "view", "--frames", "180"],
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
time.sleep(0.35)
run("--pc-boxes", "view", "--frames", 30, expected=4)
assert holder.wait(timeout=10) == 0
print("PASS: native box move/save/reload; party/map preserved; backup; missing-save refusal; Mystery Gift return; save mutex")
print("Moved Pokemon SHA-256:", hashlib.sha256(after_storage[84:164]).hexdigest())
print("Visual captures:", WORK)
