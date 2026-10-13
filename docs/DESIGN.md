# FireRed PC — design

A native PC build of Pokémon FireRed: pret's `pokefirered` decompilation (in
`game/`, see `PRET_COMMIT.txt`) compiled for i686 Windows and linked against
our own platform layer (`port/`). On top: an in-game options screen
(resolution, filters, widescreen), online play, and a standalone modding suite.

## Build

- **Assets**: pret's own rules produce every converted asset (graphics, maps,
  music); `make modern` in `game/` with devkitARM also yields a reference ROM.
- **C**: every `game/src/*.c` goes through the C preprocessor, pret's `preproc`
  (INCBIN + charmap strings), then i686 GCC with `-mno-ms-bitfields
  -fshort-enums` so struct layouts match the ARM build that the data was
  written for. Platform code (`port/`) is compiled normally.
- **Data assembly** (`game/data/*.s`, songs, sound data): assembled with the
  real ARM assembler, then `tools/elf2x86.py` converts each object into i686
  assembly — same bytes, same symbols, ARM absolute relocations turned into
  x86 ones. No hand-converted assembly, no regex rewriting of macros.
- 32-bit build: the game's script bytecode and data tables embed 4-byte
  pointers, so pointers must be 32-bit.

## Runtime

- **Game fiber**: `AgbMain` runs on its own Windows fiber. Every place the
  game waits for the hardware (VBlankIntrWait, IntrWait, busy-waits on the
  VBlank flag) yields to the host fiber, which draws the frame, delivers
  interrupts, paces to the display and resumes the game. No busy spinning, no
  restructuring of game code.
- **Hardware memory**: VRAM, palette RAM, OAM and the I/O registers are arrays;
  the address macros in `gba/defines.h` / `gba/io_reg.h` point at them.
- **DMA**: `DmaSet` calls into our DMA unit. Immediate transfers happen at
  once; HBlank-timed channels are replayed by the renderer each scanline; the
  sound FIFO channels are replaced by our mixer.
- **Renderer**: a scanline renderer (modes 0-2, affine, windows, blending,
  mosaic) that runs the game's HBlank interrupt between lines, so raster
  effects (battle intros, wave effects) are exact. It renders at a
  configurable internal width (widescreen); the GPU scales the result and
  applies the selected filter.
- **Audio**: pret's m4a sequencer (C) unchanged; the ARM mixer is replaced by
  our C mixer at 48 kHz with interpolated resampling (and a GBA-accurate
  mode), feeding SDL through a ring buffer.
- **Saves**: the flash chip is a 128 KB array backed by a `.sav` file, written
  atomically when the game commits a sector.

## Later milestones

- In-game options screen: resolution, scaling, filters, widescreen.
- Online: accounts/friends server, shared overworld, link trades/battles.
- Editor suite (standalone): maps/regions, Pokémon, trainers, music.
