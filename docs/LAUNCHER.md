# Pokémon Recompiled launcher

The launcher implements the six reference screens: title, main menu, game selection,
PC Boxes, Mystery Gifts, and Options. Its five game entries are Ruby, Sapphire,
FireRed, LeafGreen, and Emerald. The carousel renders actual WebGL box geometry
with separate retail cover textures. Buttons, text, dialogs, focus states, character
art, and background images are independent application layers.

Each opening starts with a plain, centered eight-second notice on a dark screen,
crediting Nintendo, GAME FREAK,
Creatures Inc., and The Pokémon Company. It identifies the project as unofficial
and unaffiliated, and states that the launcher provides no game or ROM downloads.
Enter, controller A, or Continue skips the notice. The notice has no artwork panels
or animated sprites.

Screen changes fade to black over 160 ms, replace the screen while covered, wait
for its returned asset-readiness promise and two rendering frames, then reveal it
over 240 ms. The opening reveal takes 350 ms. Reduced-motion mode removes these
fades. The native window and browser title are `Pokémon Recompiled · GEN 3`.

PC Boxes is a standalone launcher screen using original FireRed/LeafGreen graphics
and direct Gen III save data. Opening boxes does not start the native game. Native
gameplay renders inside the Windows launcher window when started from Game Select.

## Start and build

Run `Launch PokeRecomp.cmd` from the repository. The native Windows host is
`dist/launcher/PokeRecomp.exe`, built from `launcher/native/LauncherHost.cs` with
`tools/build_launcher_host.ps1`. The host needs Microsoft Edge WebView2 Runtime and
.NET Framework 4.7.2; the local service needs Python 3.10 or newer. It binds only to
`127.0.0.1`. This is a repository installation: original source tables under `game`
and pinned references under `launcher/gen3-reference` are needed by the storage codec.

```powershell
# Native gameplay executable, using the existing MinGW32/MSYS2 toolchain:
& 'C:\Program Files\Git\bin\bash.exe' ./build.sh -j8 EXE=dist/launcher-native/FireRedPC.exe

# Windows launcher host:
powershell -NoProfile -ExecutionPolicy Bypass -File tools/build_launcher_host.ps1

# Regenerate the standalone PC's original game artwork:
python tools/export_storage_assets.py

# Development service without the native host:
python launcher/server.py --data-dir C:/path/to/save-directory
```

The host build accepts `-WebViewLibrary` or discovers the WebView2 SDK DLLs from an
installed `pywebview` package. It copies required DLLs beside the host executable.
The dedicated game build needs `SDL2.dll` in `dist/launcher-native`. Close that game
session before rebuilding it. This target does not replace an unrelated running
root `FireRedPC.exe`.

Host logs and session metadata live in `launcher/state/native-<host PID>`. Persistent
game/save registrations, backups, and transfer journals use
`launcher/state/library` by default, or `<explicit data directory>/.launcher-library`
when the native host receives `--data-dir`. A directly started service defaults to
`launcher/state` and accepts `--library-dir` for a different persistent library.

## Launcher audio

`launcher/assets/audio.js` plays the three user-supplied MP3 files directly:

| File | Behavior |
| --- | --- |
| `launcher/assets/sounds/title_screen.mp3` | Loops on the title screen |
| `launcher/assets/sounds/menus_music.mp3` | Loops across main menu, game selection, boxes, gifts, and options |
| `launcher/assets/sounds/select.mp3` | Plays for launcher navigation and selection |

The opening notice stays quiet. Title and menu music use a gain of 0.25 times
master volume; selection sounds use 0.55 times master volume. Music fades in over
1.2 seconds when starting or resuming. The same menu track continues uninterrupted
across all interior screens and submenus, including standalone storage: navigation
does not restart it or dip its volume. Changing between title and menu music fades
the outgoing track over 160 ms, then fades the selected track in over 1.2 seconds.

Audio pauses while a native game is starting/running or the launcher page is hidden.
The master-volume control previews changes immediately for music and selection
sounds; saving commits the setting, while leaving the audio panel restores the
saved value. Native audio quality remains a separate next-game setting.

The Windows host enables WebView2 autoplay with
`--autoplay-policy=no-user-gesture-required`. Browser development sessions also
retry playback on keyboard, pointer, and controller interaction when autoplay is
restricted. Audio files and SHA-256 hashes are recorded as user-supplied assets in
`launcher/assets/manifest.json`; no different recording or synthesized substitute
is used.

## Standalone PC Boxes

Link an existing `.sav` for each game in the PC screen. Linking records the actual
file path; it does not copy that save into another game or install a sample save.
Moves change those linked files. FireRed defaults to `firered.sav` in the active
native data directory when no separate FireRed storage save is linked. FireRed
Game Select and Mystery Gifts use that same linked cartridge via `--save-file`,
including when it has a different filename or folder. Native settings still use
`--data-dir`.

The screen reads all 14 boxes with 30 slots each, box names, wallpaper IDs, and each
saved Pokemon's details. Select a Pokemon, choose an empty destination slot, then
confirm Move. Both sides can show the same game for movement within one save, or
different linked games for transfer. Source and destination revisions are checked
again at commit, so a stale screen cannot overwrite a newer save.

The graphics come from `game/graphics/pokemon_storage`, the original icon/front
sprite tables and palettes, and original storage wallpaper tilemaps. The exported
files in `launcher/assets/storage` are addressed by the game's internal species
IDs. The launcher uses FRLG visual assets for all five games; save wallpaper IDs
and names remain intact. It does not run `pokemon_storage_system*.c` to draw this
screen. `launcher/gen3.py` implements the storage data operation from the original
C save/BoxPokemon definitions, encryption order, checksum routines, character
map, species data, and experience tables.

### Transfer semantics

- The destination must be empty. An existing Pokemon is never replaced.
- The entire 80-byte encrypted BoxPokemon record is moved byte for byte. Personality,
  original trainer, nickname, experience, moves/PP, IVs, EVs, held item, ribbons,
  origin data, markings, and egg state stay unchanged.
- This is a storage transfer, not a link trade. It does not run trade evolution or
  rewrite friendship. A non-egg arrival registers the destination's caught/seen
  Pokedex bits, including the original mirrored flags. It does not unlock the
  National Dex or progression flags.
- Compatible ordinary held items move with the Pokemon. Cross-save Mail and Enigma
  Berry transfers are refused because their associated data belongs to the save.
  Invalid items, key items, invalid Pokemon checksums, Bad Eggs, unsupported species,
  and invalid move IDs are refused. Valid eggs can move without Pokedex registration.
- Full 128 KiB cartridge saves can be changed. A checksum-valid 64 KiB save can be
  viewed; export a full 128 KiB save before transferring. Emulator save states and
  ROM hacks with changed structures are not supported.
- Choose the correct game when linking. Ruby/Sapphire share a save layout, as do
  FireRed/LeafGreen; the shared layout alone cannot distinguish the paired titles.

Close games/emulators using the linked saves before moving Pokemon. The launcher
blocks operations during its own running game session and uses the rebuilt native
game's per-save Windows mutex. Unrelated emulators do not participate in that mutex;
revision checks catch changed files but cannot prevent an emulator from later
writing an old in-memory save over an external edit.

### Checksums, generations, and recovery

A save is accepted only when a complete slot contains all 14 unique sections with
the original signature, correct section-size checksums, one consistent save counter,
and matching slot parity. Sections from different save generations are never mixed.
The newest coherent slot is selected, including the original counter-wrap behavior.

A move writes a new generation into the inactive slot, increments the save counter,
rotates sectors using the game's section order, and recalculates each section
checksum. The previous active slot remains a fallback. Unrelated Hall of Fame,
Trainer Tower/Hill, and recorded-battle sectors are retained. Within-save movement
leaves trainer, party, and map data unchanged; cross-save arrival only changes the
destination's corresponding Pokedex fields outside PC storage.

Transactions use a process/thread operation lock and durable backups under
`<library directory>/gen3-transfers/<transaction ID>`. Each journal records both
paths and the original/prepared SHA-256 hashes. Files are written through temporary
files, flushed, and atomically replaced; on Windows replacement requests
`MOVEFILE_WRITE_THROUGH`. For two-save movement, the destination is committed
before the source is cleared. A successful operation verifies both files before
marking the journal committed.

A failed operation restores both original files when their hashes still match a
known transaction state. An interrupted process leaves a prepared journal; the next
storage operation recovers it before reading or transferring. Recovery restores the
source first. If either file has changed externally, recovery refuses to overwrite
it and reports the retained backup/journal directory for review. Keep that directory
until the transaction is resolved. Completed and rolled-back backups remain available.

## Native gameplay and Mystery Gifts

FireRed is bundled. The other four library entries accept separately supplied
Windows game executables. Registration does not convert a ROM or supply another
game engine. Direct save browsing and transfer do not require those executables.

The native host supplies a game-panel HWND through `--launcher-parent <decimal HWND>`.
The bundled runtime creates a hidden SDL/OpenGL window, attaches it as a borderless
child, then renders into that panel and follows resizing. Embedded gameplay uses the
host's size/fullscreen state; native F11 does not detach the child. Native filters,
color correction, audio, input, and gameplay settings remain active.

Fullscreen covers the entire launcher and embedded game. F11 during gameplay
updates the shared setting and switches the host between borderless monitor size
and its previous window size. There is no gameplay toolbar. Esc opens a native
return confirmation; No resumes the game, and Yes returns to the launcher.

When the host shows the launcher after a game exits, it dispatches the explicit
`launcher-game-return` browser event and restores WebView focus. The JavaScript
handler clears pending launch/navigation flags and controller repeat state,
invalidates stale asynchronous callbacks, and resets the transition overlay so
it no longer intercepts input. It refreshes backend state and resumes launcher
audio. The normal status-poll game-exit path uses the same reset routine; existing
error or confirmation dialogs are preserved.

Native keyboard controls are arrows, X for A, Z for B, Enter for START, Backspace for
SELECT, A/S for L/R, and Tab for fast-forward. Ordinary gameplay saves through its
original in-game SAVE action. Closing gameplay does not force an in-game save.
Settings are stored in `settings.ini` in the native data directory; the backend
backs up replaced settings and imported native saves in its persistent library.

Mystery Gifts opens FireRed's original Wonder Card/Wonder News menu, including data
already in the native save. There is no configured internet gift distribution
service. No received gifts or Pokemon are fabricated. B returns to the launcher.

### Original storage debug CLI

The native original storage entry remains available for development and verification;
it is not the launcher's PC screen:

```powershell
.\dist\launcher-native\FireRedPC.exe --data-dir C:/path/to/save-directory
.\dist\launcher-native\FireRedPC.exe --data-dir C:/path/to/settings --save-file "C:/cartridges/My FireRed.sav"
.\dist\launcher-native\FireRedPC.exe --data-dir C:/path/to/save-directory --pc-boxes move
.\dist\launcher-native\FireRedPC.exe --data-dir C:/path/to/save-directory --mystery-gift
```

The optional `--pc-boxes view|organize|move|details` arguments all enter original Move
Pokemon; `withdraw`, `deposit`, and `items` select those original storage modes. In
this debug path only, B then NO at "Continue BOX operations?" (or B again), or CLOSE
BOX then YES, completes the original storage exit and calls `TrySavingData(SAVE_NORMAL)`.
The bridge backs up `<selected save>.launcher.bak` first and atomically flushes
the selected save through an adjacent temporary file.
The original game prevents exit while a Pokemon/item is held. Closing before the
original exit discards unsaved debug-session box edits. Native exit codes are 0 for
completion, 2 for invalid arguments/save, 3 for save/backup failure, and 4 for a busy
or unavailable save lock.

## Art provenance and reconstruction

The active title combines the exact user-selected Pokémon RECOMPILED logo with
the official FireRed/LeafGreen cover illustrations of Charizard and Venusaur. These
unmodified transparent illustrations use the dynamic, inward-facing cover poses.
Their source gallery identifies Ken Sugimori as the artist and cites a 2004
Australian press kit. The logo includes the complete two-line name; the main menu's
separate edition label is only `GEN 3`.

Research used original Pokémon Box title captures and FireRed, LeafGreen, and
Emerald title-screen references. The Box captures informed the saturated violet,
cobalt, and hot-pink field fading into white, the closely grouped logo and mascots,
and the outlined title prompt. The GBA references informed the simple hierarchy of
logo, mascot, and start prompt. The plain ownership notice and fade transitions
are separate launcher UI. Reference URLs and the locally inspected captures are
recorded in the asset manifest.

`backgrounds/title-energy.png` is a generated 1672 ? 941 texture based on an actual
Box screenshot, copied unchanged from the image generator output. It is a visual
reconstruction, not an original Pok?mon Box asset. Its exact prompt is in
`backgrounds/title-energy.prompt.txt`. The source PNG retains its violet/pink
colors; `title-effects.js` recolors the rendered fire theme to warm red and orange,
and the leaf theme to teal and lime, while preserving white areas.

The 1280 ? 720 WebGL renderer combines two distorted texture samples, stronger
moving waves, and animated gradient lighting. Separate sprite layers add original
FireRed title-screen flame frames in overlapping rising rows across the lower field.
These small title flares come from `game/graphics/title_screen/firered/flames`, not
the inactive large battle-flame sheet. Their original sequence, frame durations,
pixel colors, and transparency are preserved in the export; launcher row placement,
scaling, shadows, and opacity are composed separately. The source's later olive/teal
frames fade out in the renderer without recoloring the sprite pixels.

The leaf theme uses 44 original FRLG leaf sprite instances: 24 behind the mascot
illustrations and 20 in front. Frame animation, drifting, rotation, and foreground
shadows create depth. Illustration shadows also separate the artwork from the
background. A white sheen passes across a mask of the unchanged logo every 5.4
seconds; the logo PNG itself is not edited.

The theme is selected for each opening and remains the same when returning to the
title; when local storage is available, successive openings alternate fire and
leaf. Rendering runs at up to 30 frames per second and pauses on hidden pages.
Reduced-motion mode holds the backdrop and sprite composition still, shows a
stationary subset of leaves, and disables the logo sheen. If WebGL is unavailable,
an animated 640 ? 360 Canvas 2D fallback combines two warped texture passes and
moving gradient lighting before scaling to the title canvas; independent sprite
layers remain available. Earlier coastal backgrounds and the former procedural
wave-field experiment are inactive.

The main menu retains its laboratory background. Other interior backgrounds keep
the GameCube-inspired low-poly direction reconstructed from the six-panel layout
reference. Their props are baked into raster environments; characters, controls,
logo, and carousel boxes remain separate layers. These are not interactive 3D
laboratory scenes or lossless extractions from a layered source document.

| Asset | Location and source |
| --- | --- |
| Title composition | `launcher/assets/title.css`; full-screen backdrop with separate logo, official cover illustrations, and start prompt |
| Title energy texture | `launcher/assets/backgrounds/title-energy.png`; unchanged generated reconstruction from an original Box title reference, with exact prompt beside it |
| Animated title backdrop | `launcher/assets/title-effects.js`; dual texture waves, theme recoloring, moving lighting, and animated Canvas 2D fallback |
| Leaf title sprites | `launcher/assets/title-sprites/leaf.png`; original FRLG sprite frames, drawn in separate back/front layers |
| Fire title sprites | `launcher/assets/title-sprites/title-fire.png`; original FireRed title flares with dedicated `title-fire.json` and `title-fire-provenance.json` |
| Launcher sounds | `launcher/assets/sounds/{title_screen,menus_music,select}.mp3`; user-supplied files played by `launcher/assets/audio.js` |
| Inactive battle-flame sheet | `launcher/assets/title-sprites/fire.png`; original FRLG export retained for provenance, not rendered by the current title |
| Screen fades | `launcher/assets/screen-transitions.js`; persistent overlay covers the screen swap and asset-readiness wait |
| Five interior backgrounds | `launcher/assets/backgrounds/{main,games,boxes,gifts,options}.png`; generated reference-based reconstructions |
| Selected two-line logo | `launcher/assets/logo.png`; exact unmodified user-selected Pokémon RECOMPILED preview, no Poké Ball |
| Official title illustrations | `launcher/assets/characters/{charizard,venusaur}-boxart-official.png`; unmodified FRLG cover artwork from Bulbagarden's press-kit-sourced files |
| Five retail covers | `launcher/assets/covers/*.png`; unmodified scans from the Libretro GBA thumbnail archive |
| Laboratory character overlays | `launcher/assets/characters/{pikachu,mew}-3d.png`; official HOME renders from PokeAPI, not GameCube model rips |
| Carousel | `launcher/assets/carousel.js`; box geometry with cover textures, local Three.js under `assets/vendor` |
| UI controls and states | `launcher/styles.css`, `launcher/app.js`, `launcher/assets/ui` |
| Original PC art | `launcher/assets/storage/{icons,front,wallpapers}` and `cursor.png`; exported FRLG resources |

Exact sources, reconstruction prompts, and asset distinctions are recorded in
`launcher/assets/manifest.json`, `launcher/assets/prompts`, and
`launcher/assets/storage/manifest.json`. Original title frame sources, pixel checks,
palettes, and animation bindings are recorded in
`launcher/assets/title-sprites/{metadata,provenance}.json` and the separate
`title-fire.json` / `title-fire-provenance.json`; these records describe the
exported sprite sheets, not the generated title-energy texture. Earlier title
raster artwork and the previous general Pokémon illustrations remain on disk as
inactive alternatives in the main asset manifest.
Retail cover source:
<https://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy%20Advance/Named_Boxarts/>.
Title illustration sources: [Charizard](https://archives.bulbagarden.net/wiki/File:Boxart_Charizard.png)
and [Venusaur](https://archives.bulbagarden.net/wiki/File:Boxart_Venusaur.png).
Laboratory character source: <https://github.com/PokeAPI/sprites/tree/master/sprites/pokemon/other/home>.

## Source map and verification

| Component | Source |
| --- | --- |
| Native Windows host | `launcher/native/LauncherHost.cs`, `tools/build_launcher_host.ps1` |
| Launcher screens | `launcher/index.html`, `launcher/styles.css`, `launcher/app.js` |
| Title layout and animation | `launcher/assets/title.css`, `launcher/assets/title-effects.js`, `launcher/assets/title-sprites` |
| Screen transitions | `launcher/assets/screen-transitions.js` |
| Launcher audio | `launcher/assets/audio.js`, `launcher/assets/sounds/*.mp3` |
| Standalone PC UI | `launcher/assets/storage-ui.js`, `launcher/assets/storage-ui.css` |
| Local API/registrations/settings | `launcher/server.py` |
| Direct save codec and transactions | `launcher/gen3.py` |
| Authoritative FRLG data | `game/include/{global,pokemon,pokemon_storage_system}.h`, `game/src/{save,pokemon}.c`, `game/charmap.txt`, original data tables |
| Pinned Ruby/Emerald sources | `launcher/gen3-reference/sources.json` and referenced files |
| Asset export | `tools/export_storage_assets.py` |
| Optional native debug entry | `game/src/pc_launcher.c`, `port/include/pc_launcher.h` |
| Native save lock/embedding/persistence | `port/src/{main,video,save}.c` |

Run these checks from the repository root:

```powershell
python launcher/test_gen3.py
python launcher/test_server.py
python tests/gen3_native_reopen.py
python tests/launcher_native.py
python tests/launcher_save_path.py
python tests/launcher_host.py
```

Codec tests cover the captured real FireRed record, all 24 encryption orders, all 20
cross-game routes between the five source layouts, section/counter validation,
unchanged record bytes, destination Pokedex flags, occupied/stale/invalid refusals,
hard-link aliases, normal and interrupted rollback, post-replacement failure, and
external-edit protection. Ruby/Sapphire/Emerald/LeafGreen route fixtures are generated
from the original layouts and contain the existing captured record; these tests are
not claims of real cartridge gameplay testing for every title. No fixture is installed
into the launcher product or a user's save.

`tests/gen3_native_reopen.py` transfers within a copied real FireRed save and from a
generated Emerald layout into a copied real FireRed destination. It reopens both
using the original native storage and captures the box and Pokemon summary under
`build/gen3-native-reopen-*`. The native view showed the moved male Lv5 Rattata,
Quirky nature, original trainer/ID, item, and origin unchanged. The full encrypted
record remained identical and both save slots validated; native inspection left
the revised saves unchanged.

`tests/launcher_native.py` separately verifies the optional original storage debug
move/save/reload path, party/map preservation, backup, missing-save refusal,
Mystery Gift return, and save locking. Both native tests use isolated copies of
`build/deposit.sav`. They never use the linked product saves.

`tests/launcher_save_path.py` verifies an alternate save filename containing spaces
in a folder separate from settings. The native original-storage save updates the
selected file and adjacent backup, leaves the default cartridge untouched, and
reopens the original summary. It also verifies the native mutex protects the same
selected file across different settings directories and blocks direct transfers
while that cartridge is open. All files are isolated fixture copies.

`tests/launcher_host.py` runs an isolated native host and verifies monitor-sized
fullscreen, restoration of the previous window bounds, SDL child ownership and
size, absence of a gameplay toolbar, native F11 synchronization, and both branches
of the Esc confirmation. The test leaves its cartridge copy unchanged.
