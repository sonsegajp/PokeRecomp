/* dll_api.c — what a game DLL (games/firered.dll, leafgreen.dll, emerald.dll)
 * offers the programs that load it at runtime: the launcher and the runner
 * (PokeRecompGame.exe). Nothing links against a game; they find these by name
 * with GetProcAddress. See docs/GAME_DLL.md. */
#include <stdint.h>
#include <wchar.h>
#include <SDL.h>

#include "pc_rompack.h"

#define PR_API __declspec(dllexport)
#define POKERECOMP_ABI 1

struct PokeRecompGameInfo {
    uint32_t abi;           /* POKERECOMP_ABI */
    const char *id;         /* "firered", "leafgreen", "emerald" */
    const char *title;      /* "Pokemon FireRed" (UTF-8) */
    const char *region;     /* "kanto", "hoenn" */
    const wchar_t *pkmn;    /* the asset file it runs off: "firered.pkmn" */
};

#if defined(EMERALD)
static const struct PokeRecompGameInfo sInfo = {POKERECOMP_ABI, "emerald", "Pok\xC3\xA9mon Emerald", "hoenn", NULL};
#elif defined(LEAFGREEN)
static const struct PokeRecompGameInfo sInfo = {POKERECOMP_ABI, "leafgreen", "Pok\xC3\xA9mon LeafGreen", "kanto", NULL};
#else
static const struct PokeRecompGameInfo sInfo = {POKERECOMP_ABI, "firered", "Pok\xC3\xA9mon FireRed", "kanto", NULL};
#endif

int PcGameMain(int argc, char **argv);   /* port/src/main.c, built with -Dmain=PcGameMain */

PR_API const struct PokeRecompGameInfo *PokeRecomp_GameInfo(void)
{
    static struct PokeRecompGameInfo info;
    info = sInfo;
    info.pkmn = PcRomPackName();
    return &info;
}

/* ROM import: unpack the player's ROM into a .pkmn file. Returns
 * PC_ROMPACK_OK, or _WRONG_ROM / _CANT_READ / _CANT_WRITE / _NO_TABLE. */
PR_API int PokeRecomp_Extract(const wchar_t *rom, const wchar_t *pkmn)
{
    return PcRomPackExtract(rom, pkmn);
}

/* The same, reporting progress (0-100) as it goes: for the launcher's progress bar. */
PR_API int PokeRecomp_ExtractProgress(const wchar_t *rom, const wchar_t *pkmn,
                                      void (*progress)(int percent, void *ctx), void *ctx)
{
    return PcRomPackExtractProgress(rom, pkmn, progress, ctx);
}

/* Another game's ROM: if it's the one this game takes visitors' sprites from
 * (Emerald for FireRed/LeafGreen, FireRed or LeafGreen for Emerald), unpack
 * them into `pkmn` (NULL: <game>_<region>.pkmn next to the runner).
 * PC_ROMPACK_WRONG_ROM if it isn't. */
PR_API int PokeRecomp_ExtractForeign(const wchar_t *rom, const wchar_t *pkmn)
{
    return PcRomPackExtractForeign(rom, pkmn);
}

/* Run the game (the same arguments the game always took: --pkmn, --data-dir,
 * --save-file, --launcher-parent, --headless, ...). Returns the exit code.
 * One run per process. */
PR_API int PokeRecomp_Run(int argc, char **argv)
{
    SDL_SetMainReady();      /* no SDL2main: we are not the program's entry point */
    return PcGameMain(argc, argv);
}
