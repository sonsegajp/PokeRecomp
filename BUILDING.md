# Building PokeRecomp

PokeRecomp is a native PC build of the pret Pokémon decompilations. It compiles
the game's C sources for x86 instead of ARM and links them against an SDL2
platform layer, producing a standalone `PokeRecomp.exe`.

**No ROM is required to build.** A ROM is required to *run* — the launcher
extracts assets from your own legally-owned ROM at first launch. See
[Running](#running) below.

---

## 1. Prerequisites

The build targets **32-bit Windows** via MSYS2/MinGW. The 32-bit target is not
optional: the game code assumes 32-bit pointers throughout.

### Install MSYS2

Download and install from [msys2.org](https://www.msys2.org/). Then open the
**MSYS2 MINGW32** shell (not MINGW64, not UCRT64) and install the toolchain:

```bash
pacman -Syu
pacman -S --needed \
    mingw-w64-i686-gcc \
    mingw-w64-i686-SDL2 \
    mingw-w64-i686-SDL2_mixer \
    mingw-w64-i686-SDL2_ttf \
    mingw-w64-i686-pkg-config \
    make git python
```

The Makefile expects the MinGW32 tree at `/c/msys64/mingw32`. If you installed
MSYS2 elsewhere, override it:

```bash
make -f Makefile_pc SDL_DIR=/your/path/mingw32
```

### Vendored dependencies

Nothing to fetch. Dear ImGui is vendored in `lib/imgui/` (with its SDL2 and
OpenGL3 backends in `lib/imgui/backends/`), and the `stb_*.h` headers are in
`lib/`. Both are MIT licensed; see [THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt).

---

## 2. What is and isn't in this repository

A clone contains everything needed to build and run **FireRed and LeafGreen**,
except a ROM and the SDL2 runtime DLLs.

### Included

| Item | Where | Note |
|---|---|---|
| Game source and data | `src/`, `data/`, `include/` | The decompilation itself |
| PC platform layer | `src/platform/` | SDL2, software PPU, audio, net, launcher |
| Dear ImGui | `lib/imgui/` | Vendored source, MIT |
| stb headers | `lib/` | MIT / public domain |
| Launcher art and audio | `deploy/launcher/` | 268 files; the launcher loads these at runtime and they cannot be regenerated from a ROM |
| Build tool sources | `tools/` | Compiled automatically by the Makefile |

### Deliberately excluded

| Item | Why | How to get it |
|---|---|---|
| **ROMs, `.pkmn` archives** | Copyrighted. Never distribute. | Supply your own; the launcher extracts on first run |
| **Save files, `accounts/`** | Player data | Created as you play |
| **Extracted assets** (`deploy/assets/`, `extracted_assets/`) | Derived from your ROM | Generated at runtime |
| **SDL2 and support DLLs** | 123 files, ~122 MB of redistributables | `pacman`, then copy — see below |
| **Build output** (`build/`, `*.exe`, `*.o`, `*.map`) | Regenerated | `make` |
| **Ruby/Gold prefixed objects** | Built from separate repositories | See §3, *Multi-game support* |

### Getting the runtime DLLs

The build links against SDL2 but does not copy its DLLs. After installing the
MSYS2 packages, copy them next to the executable:

```bash
cp /c/msys64/mingw32/bin/SDL2.dll        deploy/
cp /c/msys64/mingw32/bin/SDL2_mixer.dll  deploy/
cp /c/msys64/mingw32/bin/SDL2_ttf.dll    deploy/
```

SDL2_mixer and SDL2_ttf pull in further dependencies (FLAC, Ogg, Vorbis, Opus,
mpg123, FreeType, libpng, zlib and others). The reliable way to catch them all
is to launch the executable and copy whatever Windows reports as missing, or to
copy the closure in one go:

```bash
for f in $(ldd deploy/PokeRecomp.exe | grep -i mingw32 | awk '{print $3}'); do
    cp -u "$f" deploy/
done
```

Do **not** copy `libreadline8.dll` — nothing links against it, and it is GPLv3.

### What you actually get

With the above done, `deploy/PokeRecomp.exe` runs, the launcher is fully
skinned, and **FireRed and LeafGreen are playable**. Ruby, Sapphire, Gold,
Silver and Crystal will not appear as playable entries unless you also build
the sibling repositories described in §3.

---

## 3. Building

From the repository root, in the **MINGW32** shell:

```bash
make -f Makefile_pc -j$(nproc)
```

This produces `PokeRecomp.exe` in the repository root. The first build compiles
the entire game (several thousand translation units) and takes a while;
incremental builds only relink.

### Dedicated server

```bash
make -f Makefile_pc server
```

Produces `PokeServer.exe`. See [networking-explanation.md](networking-explanation.md)
for the protocol and server configuration.

### Useful targets

| Target | Effect |
|---|---|
| `rom` (default) | Build `PokeRecomp.exe` |
| `server` | Build `PokeServer.exe` |
| `tidy` | Remove build output, keep generated assets |
| `mostlyclean` | `tidy` plus generated assets |
| `clean` | Everything, including host tools |
| `clean-platform` | Remove only `src/platform/` objects |

### Multi-game support (optional)

Ruby/Sapphire and Gold/Silver/Crystal support link in objects built from
sibling repositories, compiled with symbol prefixes (`rb_`, `gd_`, `sv_`) so
their globals don't collide with FireRed's:

```
../pokeruby/build/pc_prefixed/        → rb_*   (Ruby/Sapphire)
../pokegold-native/build/pc_prefixed/ → gd_*, sv_*  (Gold/Silver)
```

If those directories are absent the wildcards resolve to nothing and the build
still succeeds — you simply get a FireRed/LeafGreen-only binary. Do **not** run
a full re-prefix over an existing `pc_prefixed/` tree; prefix single files as
needed (see [BUILD_FIXES.md](BUILD_FIXES.md)).

---

## 4. Deploying

The runtime looks for its DLLs and data in the working directory. The
conventional layout lives in `deploy/`:

```
deploy/
  PokeRecomp.exe          <- copy your build here
  PokeServer.exe
  SDL2.dll, SDL2_mixer.dll, SDL2_ttf.dll, ...
  launcher/               <- launcher art and fonts
  pokefirered.pkmn        <- generated on first run, never distributed
  pokefirered.sav         <- your save
```

Deploy just the executable — never overwrite the `.pkmn` archives or `.sav`
files, which hold extracted assets and player data:

```bash
cp PokeRecomp.exe deploy/PokeRecomp.exe
```

To produce a distributable archive, use the packaging script, which stages from
an explicit allow-list and refuses to build an archive containing ROM data,
saves, accounts, or logs:

```bash
tools/pack_release.sh PokeRecomp-v1.0
```

---

## 5. Running

On first launch the launcher will ask for a ROM. Select a GBA ROM you legally
own; it is verified against the supported game codes (`BPRE`, `BPGE`, `AXVE`,
`AXPE`) and converted into a local `.pkmn` archive. That archive stays on your
machine.

Controls default to Z/X for A/B, arrow keys for the D-pad, and **F1** toggles
the ImGui debug menu (display, audio, controls, enhancements, online, warps,
and cheats).

---

## 6. Troubleshooting

[BUILD_FIXES.md](BUILD_FIXES.md) documents every known build failure in detail:
temp-file errors, the `REG_BASE` conflict, the assembly conversion pipeline,
ImGui compilation, and unresolved symbols.

Two things worth knowing up front:

**Check make's real exit status.** Piping make through `tail` or `head` masks
failures, because the pipeline reports the *last* command's status:

```bash
make -f Makefile_pc > build.log 2>&1; echo "exit=$?"   # correct
make -f Makefile_pc | tail -20                          # hides failures
```

**The link tolerates unresolved symbols.** The link step passes
`-Wl,--warn-unresolved-symbols` and redirects stderr to `linker_errors.txt`,
so a missing function does **not** fail the build — it produces a binary that
jumps to a bogus address when that function is called. After any build that
touches cross-module references, check:

```bash
grep "undefined reference" linker_errors.txt | sed 's/.*undefined reference to //' | sort -u
```

References prefixed `rb_`, `gd_`, or `sv_` against libc functions
(`rb__fputc`, `gd__vfprintf`, …) are known artifacts of symbol prefixing and
are harmless. Anything else is a latent crash.
