# Native PC Port — Credits and Attribution

This PC port builds on the work of others. Attribution here is load-bearing:
the platform layer was not written for pokefirered — it was adapted, with
compatibility fixes, from an existing port of a sibling decompilation.

## Upstream

- **pret/pokefirered** — the source decompilation this port builds on.
  https://github.com/pret/pokefirered

## Platform layer

- **Kurausukun/pokeemerald @ pc_port** — the SDL2-based platform layer,
  software PPU renderer, BIOS/DMA/sound shims, and `Makefile_pc` build
  system were initially imported from the `pc_port` branch of this fork
  of `pret/pokeemerald`.
  https://github.com/Kurausukun/pokeemerald/tree/pc_port

  Files initially imported from that tree:
  - `src/platform/bios.c`
  - `src/platform/cgb_audio.c`
  - `src/platform/dma.c`
  - `src/platform/gba_easy_draw.c`
  - `src/platform/gba_fast_draw.c`
  - `src/platform/nostd.c`
  - `src/platform/sdl2.c`
  - `src/platform/win32.c`
  - `include/platform.h`
  - `include/platform/dma.h`
  - `include/platform/framedraw.h`
  - `include/pc_screen_effect.h`
  - `Makefile_pc`

  These files were subsequently adapted to pokefirered's codebase (different
  APIs, structure, and feature set).

## Assets

This port ships **no Nintendo-owned assets**. A user-supplied ROM of
Pokémon FireRed or LeafGreen (which they legally own) is required —
assets are extracted from the user-supplied ROM at build or first run.

All Pokémon characters, artwork, music, sound effects, map data, script
text, and related content are © Nintendo, Creatures Inc., and GAME FREAK.
This project claims no rights to any of the above.
