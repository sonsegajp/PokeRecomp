"""make_release.py VERSION — the player download: build/release/PokeRecomp-VERSION-windows.zip.

The same layout as the repository root, holding only what runs:

    Launch PokeRecomp.cmd / .ps1      start the launcher
    dist/launcher/                    the launcher window (WebView2 host)
    launcher/                         the launcher app and its local service
    PokeRecompGame.exe, SDL2.dll      the runner that plays a game DLL
    games/*.dll                       FireRed, LeafGreen, Emerald (no ROM content:
                                      importing your ROM makes each game's .pkmn)
    game/...                          the source tables PC Boxes reads
    server/*.py                       for hosting a world
    Host * World (*).bat              host a Kanto or Hoenn world

Build the three games first (./build.sh VERSION=...), with the retail ROMs
present so their content is taken out of the DLLs.
"""
import glob
import os
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SDL2 = "C:/msys64/mingw32/bin/SDL2.dll"   # the 32-bit SDL2 the games link against

FILES = [
    "Launch PokeRecomp.cmd", "Launch PokeRecomp.ps1",
    "Host Kanto World (FireRed).bat", "Host Kanto World (LeafGreen).bat", "Host Hoenn World (Emerald).bat",
    "PokeRecompGame.exe", "games/firered.dll", "games/leafgreen.dll", "games/emerald.dll",
    "dist/launcher/PokeRecomp.exe", "dist/launcher/PokeRecomp.exe.config",
    "dist/launcher/Microsoft.Web.WebView2.Core.dll", "dist/launcher/Microsoft.Web.WebView2.WinForms.dll",
    "dist/launcher/WebView2Loader.dll",
    # what launcher/gen3.py reads (PC Boxes)
    "game/charmap.txt", "game/src/pokemon.c", "game/src/data/items.json",
    "game/include/constants/species.h", "game/include/constants/pokemon.h", "game/include/constants/items.h",
    "game/include/constants/moves.h", "game/include/constants/pokedex.h",
    "game/src/data/text/species_names.h", "game/src/data/text/move_names.h",
    "game/src/data/pokemon/species_info.h",
    "docs/LAUNCHER.md", "docs/GAME_DLL.md",
]
TREES = [  # (folder, files to leave out)
    ("launcher", ("launcher/state/", "launcher/native/", "/__pycache__/", "launcher/test_")),
    ("server", ("/__pycache__/", "server/deploy/", "server/master", "server/smoke_test", "positions_",
                "legality_data", "server_key", ".bin")),
]

README = """Pokemon Recompiled (PokeRecomp) {version}

Native PC versions of Pokemon FireRed, LeafGreen and Emerald, built from the
pret decompilations. They run off your own cartridge: the first time you pick
a game, import your ROM and it's unpacked into that game's .pkmn file.

Start: run "Launch PokeRecomp.cmd".
Needs: Windows 10/11, the Microsoft Edge WebView2 Runtime, .NET Framework
4.7.2 and Python 3.10 or newer (the launcher's local service).

ROMs: FireRed (USA 1.0 / 1.1), LeafGreen (USA 1.0 / 1.1), Emerald (USA/Europe).

Online: talk to the desk on the left of any Pokemon Center 2F (the ONLINE
CLUB). Host your own world with the "Host ... World" .bat files (Python 3);
its settings are in "Kanto World Settings.ini" / "Hoenn World Settings.ini".

Unofficial fan project, not affiliated with Nintendo, Creatures Inc.,
GAME FREAK inc. or The Pokemon Company. No ROMs or game downloads included.
https://github.com/sonsegajp/PokeRecomp
"""


def skip(rel, rules):
    rel = "/" + rel.replace("\\", "/")
    return any(r in rel for r in rules)


def main():
    version = sys.argv[1] if len(sys.argv) > 1 else "dev"
    out_dir = os.path.join(ROOT, "build", "release")
    os.makedirs(out_dir, exist_ok=True)
    out = os.path.join(out_dir, "PokeRecomp-%s-windows.zip" % version)
    top = "PokeRecomp-%s" % version
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for rel in FILES:
            z.write(os.path.join(ROOT, rel), "%s/%s" % (top, rel))
        z.write(SDL2, "%s/SDL2.dll" % top)
        for folder, rules in TREES:
            for path in sorted(glob.glob(os.path.join(ROOT, folder, "**", "*"), recursive=True)):
                rel = os.path.relpath(path, ROOT).replace("\\", "/")
                if os.path.isfile(path) and not skip(rel, rules):
                    z.write(path, "%s/%s" % (top, rel))
        z.writestr("%s/README.txt" % top, README.format(version=version))
    print("%s (%.1f MB)" % (out, os.path.getsize(out) / 2 ** 20))


if __name__ == "__main__":
    main()
