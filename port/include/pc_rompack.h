/* pc_rompack.h — the game's content comes from the player's ROM (see port/src/rompack.c). */
#ifndef PC_ROMPACK_H
#define PC_ROMPACK_H

#include <wchar.h>

enum {
    PC_ROMPACK_OK = 0,
    PC_ROMPACK_WRONG_ROM = 1,    /* not a ROM (revision) this game knows */
    PC_ROMPACK_CANT_READ = 2,
    PC_ROMPACK_CANT_WRITE = 3,
    PC_ROMPACK_NO_TABLE = 4,     /* a development build with its data inside */
};

/* Call first thing in the game's main. Fills the game's taken-out data back
 * in from <game>.pkmn (--pkmn PATH, default next to the runner), making it
 * from the player's ROM if it isn't there yet. Returns 0 if the game can't
 * run (no usable ROM). */
int PcRomPackLoad(int argc, char **argv);

/* ROM import: check `rom` and unpack this game's content into `out` (.pkmn). */
int PcRomPackExtract(const wchar_t *rom, const wchar_t *out);
int PcRomPackExtractProgress(const wchar_t *rom, const wchar_t *out, void (*cb)(int percent, void *ctx), void *ctx);

/* "firered.pkmn" etc. */
const wchar_t *PcRomPackName(void);

#endif
