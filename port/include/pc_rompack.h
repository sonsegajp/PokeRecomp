/* pc_rompack.h — the game's content comes from the player's ROM (see port/src/rompack.c). */
#ifndef PC_ROMPACK_H
#define PC_ROMPACK_H

/* Call first thing in main(). Fills the exe's taken-out game data back in
 * from <game>.pak (making it from the player's ROM on first run).
 * Returns 0 if the game can't run (no usable ROM). */
int PcRomPackLoad(int argc, char **argv);

#endif
