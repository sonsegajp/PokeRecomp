# Game DLLs, `.pkmn` files and the runner

Written for whoever wires the launcher to the games (the launcher side is
Codex's). This is the contract between the launcher and the games.

## What gets built

| File | What it is |
|---|---|
| `games/firered.dll` | Pokémon FireRed (`./build.sh -j8`) |
| `games/leafgreen.dll` | Pokémon LeafGreen (`./build.sh VERSION=LEAFGREEN -j8`) |
| `games/emerald.dll` | Pokémon Emerald (`./build.sh VERSION=EMERALD -j8`) |
| `PokeRecompGame.exe` | the runner: loads a game DLL at runtime and runs it in its own process |
| `SDL2.dll` | needed next to the runner (from `C:/msys64/mingw32/bin`) |

There are no game `.exe` files any more, and nothing has to be "linked" or
registered: a game is installed when its DLL is in `games/`. The launcher
finds games by looking there (or asks the runner, below).

The DLLs contain the game's code only. Everything that came from the
cartridge (graphics, maps, music, text, scripts) is taken out at build time and
comes from the player's own ROM, unpacked into a `.pkmn` file (like Ship of
Harkinian's `.otr`).

## The `.pkmn` files

- One per game: `firered.pkmn`, `leafgreen.pkmn`, `emerald.pkmn`.
- Made from the player's ROM **once**; after that the ROM isn't needed.
- Each DLL only accepts the right game's ROM, recognised by SHA-1
  (FireRed 1.0/1.1, LeafGreen 1.0/1.1, Emerald USA/Europe). A wrong ROM is
  refused, nothing is written.
- Checked by size and checksum every launch; a damaged file is rebuilt from a
  ROM (or the player is asked for one).
- Default location: next to `PokeRecompGame.exe`. Any other location: pass
  `--pkmn PATH` when running.

## The runner — `PokeRecompGame.exe`

```
PokeRecompGame.exe --list
```
Prints the installed games as JSON, one per `games/*.dll`:
```json
[{"id":"emerald","title":"Pokémon Emerald","region":"hoenn","pkmn":"emerald.pkmn"},
 {"id":"firered","title":"Pokémon FireRed","region":"kanto","pkmn":"firered.pkmn"},
 {"id":"leafgreen","title":"Pokémon LeafGreen","region":"kanto","pkmn":"leafgreen.pkmn"}]
```

```
PokeRecompGame.exe --extract ROM [--out FILE.pkmn] [--game ID]
```
ROM import. Finds the installed game the ROM belongs to and unpacks it into its
`.pkmn` (default next to the runner, or `--out`). Prints
`{"id":...,"title":...,"region":...,"pkmn":"<full path written>"}` and exits 0,
or prints `{"error":N}` and exits N: 1 = no installed game knows this ROM,
2 = can't read the ROM, 3 = can't write the `.pkmn`.

```
PokeRecompGame.exe --game ID [game arguments...]
```
Runs a game. Arguments are the same as the old `FireRedPC.exe` took:

| Argument | |
|---|---|
| `--pkmn PATH` | the game's `.pkmn` (default: next to the runner) |
| `--data-dir DIR` | settings, `online.dat`, default save |
| `--save-file PATH` | the cartridge save to use |
| `--launcher-parent HWND` | draw into this window (decimal HWND) |
| `--pc-boxes MODE`, `--mystery-gift` | the launcher's special modes |
| `--rom PATH` | make the `.pkmn` from this ROM if it's missing |
| `--headless --frames N ...` | tests |

## ROM import in the launcher

Two moments, both through the runner:

1. **Import ahead of time** (an "Import ROM" button): ask the player for a
   `.gba`, run `PokeRecompGame.exe --extract <rom>`, show the JSON result.
2. **When a game is selected** and its `.pkmn` doesn't exist: either do (1)
   first, or just start it. The game itself then asks for the ROM with a file
   dialog, imports it, and plays. (So a game always gets its `.pkmn` one way or
   the other.)

To know whether a game is ready, check that its `.pkmn` (from `--list`) exists.

## The DLL API (for programs other than the runner)

Each DLL exports, by name (C calling convention, 32-bit):

```c
struct PokeRecompGameInfo { uint32_t abi; const char *id, *title, *region; const wchar_t *pkmn; };
const struct PokeRecompGameInfo *PokeRecomp_GameInfo(void);
int PokeRecomp_Extract(const wchar_t *rom, const wchar_t *pkmn);   /* 0 ok, 1 wrong ROM, 2 can't read, 3 can't write */
int PokeRecomp_Run(int argc, char **argv);                         /* one run per process */
```
`abi` is 1. The DLLs are 32-bit, so a 64-bit process (e.g. 64-bit Python)
can't load them; use the runner.

## Flicker when embedded (for the launcher host)

The game draws with OpenGL into a child window of the launcher's `gamePanel`
(a WinForms `Panel` with `BackColor = Black`). A plain `Panel` has no
`WS_CLIPCHILDREN` and paints its background on every invalidate, so it paints
black over the game's frame between swaps, which shows as flicker every other
frame. Fix in `LauncherHost.cs`: give the game panel `WS_CLIPCHILDREN`
(override `CreateParams`, `cp.Style |= 0x02000000`) and skip its background
painting while a game is embedded (override `OnPaintBackground` to do nothing),
and avoid calling `BringToFront()`/`SizeGameWindow()` every timer tick when
nothing changed.
