# Credits and Attribution

PokeRecomp is a native PC port built on other people's work. Attribution here is
load-bearing, not decorative — most of what makes this project possible was
written by someone else.

---

## Nintendo / Creatures / GAME FREAK

Pokémon FireRed, LeafGreen, Ruby, Sapphire, Gold, Silver, and Crystal — all
characters, artwork, music, sound effects, map data, script text, and related
content — are © **Nintendo**, **Creatures Inc.**, and **GAME FREAK inc.**

This project claims no rights to any of it, is not affiliated with or endorsed
by any of them, and **distributes none of their assets**. Running PokeRecomp
requires a ROM that you legally own; assets are extracted from that ROM on your
own machine at first launch.

---

## Upstream decompilations — the pret projects

Essentially all of the game logic in this port is the work of the
[pret](https://pret.github.io/) decompilation teams and their contributors,
representing many years of collective reverse engineering.

| Project | Used for |
|---|---|
| [pret/pokefirered](https://github.com/pret/pokefirered) | FireRed / LeafGreen — the base of this port |
| [pret/pokeruby](https://github.com/pret/pokeruby) | Ruby / Sapphire support, Hoenn content |
| [pret/pokegold](https://github.com/pret/pokegold) | Gold / Silver |
| [pret/pokecrystal](https://github.com/pret/pokecrystal) | Crystal |
| [pret/pokeemerald](https://github.com/pret/pokeemerald) | Reference for the platform layer (below) |

These repositories are published without a license. This port's own MIT license
covers only the PC port code — it cannot and does not relicense any upstream
decompilation work. See [LICENSE](LICENSE).

---

## Platform layer

The SDL2 platform layer was **not written for pokefirered**. It was adapted,
with compatibility fixes, from an existing port of a sibling decompilation:

**[Kurausukun/pokeemerald @ `pc_port`](https://github.com/Kurausukun/pokeemerald/tree/pc_port)**

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

They were subsequently adapted to pokefirered's codebase, which has different
APIs, structure, and feature set. The software PPU renderer, BIOS/DMA/sound
shims, and the shape of the PC build system all originate there.

---

## Build tools

| Tool | Author | License |
|---|---|---|
| bin2c, gbagfx, preproc, scaninc, rsfont, ramscrgen, mid2agb | YamaArashi | MIT |
| wav2agb | ipatix | MIT |
| gbafix | (GBA homebrew community) | GPLv3 — build-time only, never linked |

---

## Libraries

| Library | Author | License |
|---|---|---|
| [SDL2](https://libsdl.org/) / SDL2_mixer / SDL2_ttf | Sam Lantinga and contributors | zlib |
| [Dear ImGui](https://github.com/ocornut/imgui) | Omar Cornut | MIT |
| [stb](https://github.com/nothings/stb) (`stb_image`, `stb_image_write`, `stb_truetype`) | Sean Barrett | MIT / public domain |
| FreeType | The FreeType Project | FTL |
| Ogg / Vorbis / Opus / FLAC | Xiph.Org Foundation | BSD |
| FluidSynth, mpg123, LAME, libxmp, GLib | respective authors | LGPL |

Full texts and notices: [THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt).

---

## Audio references

Porting the GBA `m4a` sound engine drew on prior work by others, including
[agbplay](https://github.com/ipatix/agbplay) (ipatix) and the
`pokeemerald` PC port's CGB audio implementation.

---

## This port

The PC-specific code — the launcher, `.pkmn` archive format and ROM extractor,
cross-game box transfer UI, ImGui debug/enhancement menus, online multiplayer
client and dedicated server, Ruby/Sapphire and Gold/Silver integration, and the
PC build system extensions — was written for this project.

If your work is included here and is credited incorrectly or not at all, that's
an oversight rather than an intent — please open an issue and it will be fixed.
