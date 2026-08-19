#ifndef GUARD_PLATFORM_ROM_LOADER_H
#define GUARD_PLATFORM_ROM_LOADER_H

#include "global.h"

#define FIRERED_ROM_SIZE    (16 * 1024 * 1024)
#define ROM_CONFIG_FILE     "rom_path.txt"

bool8 LoadRomFromFile(const char *path);
bool8 PromptAndLoadRom(void);
const u8 *GetRomBase(void);
bool8 IsRomLoaded(void);

#endif // GUARD_PLATFORM_ROM_LOADER_H
