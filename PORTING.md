# How PokeRecomp Works

This document describes what it actually took to turn a GBA decompilation into
a native PC executable — the architecture, the traps, and the techniques that
turned out to matter.

It is written for someone who wants to understand or extend the port, not for
someone who just wants to play it.

---

## 1. The premise: recompiled, not emulated

The pret decompilations are real C. Given a compiler that targets ARM and a
linker script that lays out a cartridge, they reproduce the original ROM
byte-for-byte. But nothing about the *C* is ARM-specific — what's ARM-specific
is everything the C talks to: memory-mapped hardware registers, BIOS calls,
DMA, the tile-based PPU, and the sound hardware.

So the port compiles the same C for x86 and replaces the hardware with software:

```
   GBA build                        PC build
   ---------                        --------
   game C sources          same     game C sources
   ARM cross-compiler       ->      i686-w64-mingw32-gcc
   REG_BASE = 0x04000000    ->      a 1 KB array in .bss
   PPU draws tiles          ->      software renderer -> OpenGL texture
   m4a on GBA sound HW      ->      m4a mixed to an SDL2 audio callback
   cartridge flash          ->      a 128 KB array backed by a .sav file
   link cable               ->      TCP
```

The result runs at native speed with no interpreter or JIT anywhere.

---

## 2. Faking the hardware

### Registers

The single most important trick. On GBA, `REG_BASE` is the literal address
`0x04000000` and every `REG_DISPCNT`-style macro is an offset from it. On PC
that address is unmapped, so `include/gba/io_reg.h` swaps the constant for a
real array:

```c
#ifndef PORTABLE
#define REG_BASE 0x4000000
#else
extern unsigned char REG_BASE[];      // defined in src/platform/bios.c
#endif
```

This has to hold across the *assembly* sources too, which is why `ASFLAGS`
carries `--defsym PORTABLE=1` — `constants/gba_constants.inc` otherwise emits
`.set REG_BASE, 0x4000000` as an absolute linker symbol and the two definitions
silently fight. The verification is `nm PokeRecomp.exe | grep REG_BASE`: it must
resolve to `.bss`, never to `04000000`.

### BIOS, DMA, and the PPU

`src/platform/` reimplements the rest:

| File | Replaces |
|---|---|
| `bios.c` | GBA BIOS SWI calls, `REG_BASE` storage |
| `dma.c` | DMA channels |
| `gba_easy_draw.c` / `gba_fast_draw.c` | The tile/sprite PPU — two renderer strategies, selected by `TILE_RENDERER` |
| `cgb_audio.c` | Game Boy Color PSG channels |
| `sdl2.c` | Window, GL context, input, audio device, main loop, save file I/O |
| `nostd.c` | Freestanding libc bits the GBA build assumed |

The renderer composites backgrounds and sprites into a framebuffer each frame
and uploads it as an OpenGL texture, which is also what makes libretro shader
support possible.

### Audio

The GBA `m4a` sequencer is real code in the decomp, so it ports directly — it
just needed somewhere to write samples. `sdl2.c` opens an SDL2 device at
**42048 Hz** (matching the GBA's mixing rate closely enough to keep tempo
correct) and drives `m4a` from the audio callback, with `cgb_audio.c` handling
the PSG channels alongside it.

### Saves

Cartridge flash becomes `FLASH_BASE`, a 128 KB array. `ReadSaveFile()` loads
`pokefirered.sav` / `pokeleafgreen.sav` into it at boot and writes it back on
save. The game's own sector logic — 14 sectors of 4 KB, each with an id,
checksum, `0x08012025` signature, and counter in its footer — runs unmodified
on top.

---

## 3. The build pipeline

The C compiles almost unchanged. The **assembly** does not: `data/*.s` is full
of ARM-flavoured GAS that x86 GAS rejects. Each file goes through a seven-stage
conversion:

1. `preproc` — charmap text substitution
2. `cpp` — C preprocessor expansion
3. `preproc -ie` — include expansion
4. **pseudo-op conversion** (sed) — `.4byte`→`.int`, `.2byte`→`.short`,
   `.word`→`.int`, strip `.size`, strip `@` comments while preserving `\@` macro
   counters via a `__MACRO_CTR__` round-trip
5. `tools/wrap_macro_args.py` — parenthesise macro arguments containing
   top-level operators, which x86 GAS otherwise splits into extra arguments
6. `as` — assemble
7. `objcopy --prefix-symbol _` — Win32 symbol mangling

Miss step 7 and the C can't find the data. Miss step 5 and you get
"too many arguments" from GAS on files that look fine.

---

## 4. Where 32-bit x86 differs from ARM

The decomp's C is portable in principle and full of assumptions in practice.
The recurring theme is that **struct layout is not the same**, and code that
walked memory by hand broke in ways that looked like graphics bugs.

The clearest example: `sizeof(union AffineAnimCmd)` is **8 on ARM but 6 on
x86**. `RunAffineAnimFromTaskData` indexed it with `<< 3` through a `void*`,
which silently walked into neighbouring commands and corrupted battle
animations like Defense Curl. The fix was typed pointer indexing so the compiler
computes the stride.

Others of the same family:

- **String pointer range checks.** `StringCopy` had a check rejecting pointers
  outside the GBA's ROM range — on PC that rejected legitimate heap/`.data`
  strings, which killed level, max-HP, and location text.
- **`EXT_CTRL_CODE_PALETTE`** was a no-op, so palette control codes in text did
  nothing until it was wired to `PutWindowRectTilemapOverridePalette`.
- **Save sector footers.** Signature at `+4`, counter at `+8` — the port had
  them at `+8`/`+12`, which made saves look invalid.

### Reading past the end of arrays

The most dangerous class, because the GBA tolerated it. Code that read a fixed
64 entries out of a variable-length table was harmless on a cartridge — it read
whatever ROM sat next door. On PC, where that table is a real allocation, the
same read walks off a page and segfaults.

`LoadSaveblockObjEventScripts` did exactly this: it copied script pointers for
all `OBJECT_EVENT_TEMPLATES_COUNT` (64) slots regardless of the map's actual
`objectEventCount`, reading up to ~1.5 KB past the array. It now clamps to the
real count.

**If you are chasing a crash in ported code, suspect an unbounded loop over a
variable-length table first.**

---

## 5. Asset extraction and the `.pkmn` format

The port ships no game data, so it has to get everything from the user's ROM at
runtime.

`tools/rom_extractor.py` and `src/platform/pkmn_archive.c` read a ROM, verify
its header game code (`BPRE`, `BPGE`, `AXVE`, `AXPE`), and write a `.pkmn`
archive — a 64-byte header (magic `PKMN`, version, flags, ROM size, CRC32, game
code and title) followed by the ROM image and an extracted-asset table.

At runtime `asset_loader.c` prefers loose extracted assets under
`extracted_assets/frlg/` when present, falling back to ROM data otherwise. It
also applies JSON data overrides (`data/pokemon.json`, `data/moves.json`) onto
the species and move tables, which is what makes stat editing possible without
rebuilding.

> Those overrides target `gSpeciesInfo[]` and `gBattleMoves[]`, which are
> declared `const` and therefore live in read-only pages. Writing to them
> faulted on every field — 386 faults for species, 355 for moves, every launch —
> so the overrides had never actually applied. `AssetLoader_Unprotect()` now
> calls `VirtualProtect`/`mprotect` first.

---

## 6. Running several games in one binary

Ruby/Sapphire and Gold/Silver/Crystal are separate decompilations with their own
globals — `gMain`, `gSaveBlock1Ptr`, and several thousand more — that collide
head-on with FireRed's.

The solution is **symbol prefixing**: those projects are compiled separately and
every symbol in their objects is renamed with `objcopy` to carry a prefix:

```
../pokeruby/build/pc_prefixed/        -> rb_*
../pokegold-native/build/pc_prefixed/ -> gd_*, sv_*
```

The FireRed binary links those objects and calls in through a small number of
entry points (`rb_RunPokeRuby`, `gd_RunPokeGold`, `sv_RunPokeGold`), each of
which takes over the existing SDL window and GL context and returns to the
launcher when the player exits.

The rule learned the hard way: **prefix only system and library symbols, never
FireRed cross-calls**. Redirecting game constants and functions produced a Ruby
build that ran on FireRed's data. Prefixing also over-reaches onto libc calls,
which is where the residual `rb__fputc` / `gd__vfprintf` unresolved references
come from.

---

## 7. Launcher, storage, and multiplayer

**Launcher** (`launcher.c`) — a self-contained OpenGL UI hosted by Brigette:
generation select, per-game box art carousel, ROM import when a `.pkmn` is
missing, and an entry point into the storage UI. It creates the window and GL
context that the games then reuse.

**Box transfer** (`box_transfer.c`) — a dual-pane PC storage browser across all
supported games, including Gen 2 ↔ Gen 3 conversion (DVs → IVs, StatExp → EVs,
the Gen 3 encryption layer, OT name synthesis) and Gen 2 SRAM write-back.

**Multiplayer** — a TCP client, a dedicated server, and a shim that impersonates
the Game Link Cable so the original battle and trade engines work unmodified
over the network. Fully documented in
[networking-explanation.md](networking-explanation.md).

---

## 8. Debugging a recompiled game

Ordinary techniques work, with two caveats worth knowing before you lose a day.

### The crash guard hides bugs

`src/platform/crash_guard.c` installs a vectored exception handler that catches
*every* access violation and forces execution onward — either by faking a
function epilogue using `EBP` as a frame pointer, or by setting `EIP += 1` and
resuming mid-instruction.

It was added to survive battle-animation faults, and it does keep the game
running. But `EBP` is not a frame pointer under `-O2`, so the fake unwind jumps
to whatever happened to be on the stack. **A recoverable fault becomes stack
corruption**, and the eventual crash has no relationship to the original bug.
One session's log had 83,017 swallowed faults hiding three real bugs.

If you are debugging, remember the guard is lying to you.

### gdb sees faults first

```bash
gdb -batch -x cmds.txt --args ./PokeRecomp.exe
```

gdb receives first-chance exceptions *before* the app's handler, so it stops at
the real fault with a real backtrace — which is how the map-name-popup crash was
found in one shot:

```
#0  StringCopy
#1  GetMapName
#2  MapNamePopupPrintMapNameOnWindow
#3  Task_MapNamePopup
```

A consequence worth remembering: **the game crashes harder under gdb than
without it**, because faults the guard would have papered over now stop the
process. That is a feature.

### Symbolizing a crash without a debugger

When you only have a `crash_debug.log` — runtime addresses, no symbols — the
logged instruction bytes are enough. Windows ASLR relocates at 64 KB
granularity, so **the low 16 bits of a runtime address equal the low 16 bits of
the static address**. Search `PokeRecomp.exe` for the logged byte pattern,
convert file offsets to VAs through the PE section table, and look the result up
in `PokeRecomp.map`. Matching low-16 bits confirms the hit, and the delta
between two faults in the same run cross-checks it.

### The linker will not stop you

The link runs with `-Wl,--warn-unresolved-symbols`, sends stderr to
`linker_errors.txt`, and ends in `exit 0`. A function that exists nowhere
produces a *successful build* whose call jumps to a bogus address.

This one flag caused three separate crashes:

- `Hoenn_GetMapsecName` — declared in `GetMapName`, defined nowhere, called for
  **every** map name popup including Kanto. It returned garbage, `StringCopy`
  scanned it for a terminator that never came, and the stack was destroyed a
  second after any save loaded.
- `LoadSaveblockMapHeader` / `LoadSaveblockObjEventScripts` — `static` in
  `overworld.c` but declared `extern` and called from `net_client.c`, so online
  login jumped into nothing.
- `gWildMonHeaders` — `poke_radar.c` missed the `gIsLeafGreen ? _LG : _FR`
  shim that the other encounter files use.

Check after every build:

```bash
grep "undefined reference" linker_errors.txt | sed 's/.*undefined reference to //' | sort -u
```

---

## 9. Known issues

- **Ruby/Sapphire** — battle HUD garbles until a VRAM refresh (opening and
  closing the bag fixes it), attack animations corrupt the display, and the game
  can hang entering battle. Root cause is a missing `dma3_manager.c` on the Ruby
  side: `ProcessDma3Requests` / `RequestDma3Copy` don't exist there.
- **Residual unresolved symbols** — `rb_`/`gd_`/`sv_` prefixed libc references.
  Harmless in practice, but they are what prevents making the link strict, which
  is the real fix for the whole class of bugs in §8.
- **Optimization sensitivity** — some areas are built at lower optimization
  because higher levels expose latent undefined behaviour. That is a symptom,
  not a solution.
