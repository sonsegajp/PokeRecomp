#ifdef PORTABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#include "global.h"
#include "platform/rom_loader.h"
#include "platform/pkmn_archive.h"

#ifdef PORTABLE
#include "game_version.h"
#define PKMN_FILE (gIsLeafGreen ? "pokeleafgreen.pkmn" : "pokefirered.pkmn")
#else
#define PKMN_FILE "pokefirered.pkmn"
#endif

static struct PkmnArchive sArchive;
static bool8 sLoaded = FALSE;

bool8 IsRomLoaded(void)
{
    return sLoaded;
}

const u8 *GetRomBase(void)
{
    return sLoaded ? PkmnArchive_GetRomData(&sArchive) : NULL;
}

#ifdef _WIN32
static bool8 ShowRomDialog(char *outPath, int maxLen)
{
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    outPath[0] = '\0';

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "GBA ROM Files (*.gba)\0*.gba\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = outPath;
    ofn.nMaxFile = maxLen;
    ofn.lpstrTitle = gIsLeafGreen ? "Select Pokemon LeafGreen ROM" : "Select Pokemon FireRed ROM";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    return GetOpenFileNameA(&ofn) ? TRUE : FALSE;
}
#endif

static bool8 ExtractRomToPkmn(void)
{
#ifdef _WIN32
    char romPath[MAX_PATH] = {0};
    if (!ShowRomDialog(romPath, MAX_PATH))
    {
        printf("No ROM selected.\n");
        return FALSE;
    }

    printf("Extracting ROM to %s...\n", PKMN_FILE);
    if (!PkmnArchive_CreateFromRom(romPath, PKMN_FILE))
        return FALSE;

    return TRUE;
#else
    const char *defaultNames[] = {
        "pokefirered.gba", "Pokemon FireRed.gba",
        "pokemon_firered.gba", "baserom.gba", "firered.gba", NULL
    };
    for (int i = 0; defaultNames[i]; i++)
    {
        FILE *f = fopen(defaultNames[i], "rb");
        if (f)
        {
            fclose(f);
            printf("Found ROM: %s\n", defaultNames[i]);
            if (PkmnArchive_CreateFromRom(defaultNames[i], PKMN_FILE))
                return TRUE;
        }
    }
    printf("No ROM found. Place a Pokemon FireRed (US) .gba file in the current directory.\n");
    return FALSE;
#endif
}

bool8 PromptAndLoadRom(void)
{
    FILE *f = fopen(PKMN_FILE, "rb");
    if (f)
    {
        fclose(f);
        if (PkmnArchive_Open(&sArchive, PKMN_FILE))
        {
            sLoaded = TRUE;
            extern void AssetLoader_Init(void);
            AssetLoader_Init();
            return TRUE;
        }
        printf("Existing %s is corrupt, re-extracting...\n", PKMN_FILE);
    }

    printf("No %s found. A ROM is needed to generate game data.\n\n", PKMN_FILE);

    for (;;)
    {
#ifdef _WIN32
        {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "PokeRecomp needs to extract game data from a\n"
            "Pokemon %s (US) ROM (.gba) to create\n"
            "%s.\n\n"
            "Would you like to select a ROM now?",
            gIsLeafGreen ? "LeafGreen" : "FireRed", PKMN_FILE);
        int result = MessageBoxA(NULL, msg, "PokeRecomp", MB_YESNO | MB_ICONQUESTION);
        if (result != IDYES)
            return FALSE;
        }
#endif

        if (!ExtractRomToPkmn())
            return FALSE;

#ifdef _WIN32
        {
        char msg2[128];
        snprintf(msg2, sizeof(msg2), "%s created successfully!", PKMN_FILE);
        MessageBoxA(NULL, msg2, "PokeRecomp", MB_OK | MB_ICONINFORMATION);
        }
        break;
#else
        break;
#endif
    }

    if (PkmnArchive_Open(&sArchive, PKMN_FILE))
    {
        sLoaded = TRUE;
        return TRUE;
    }

    return FALSE;
}

bool8 LoadRomFromFile(const char *path)
{
    return PkmnArchive_CreateFromRom(path, PKMN_FILE)
        && PkmnArchive_Open(&sArchive, PKMN_FILE)
        && (sLoaded = TRUE);
}

#endif // PORTABLE
