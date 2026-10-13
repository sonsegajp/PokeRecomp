/* travel_io.c — the files behind travelling between regions (pc_travel.h):
 * travel.dat in the data directory, and which games are installed next to
 * the runner. Kept out of shared/pc_travel.c, whose game headers don't mix
 * with windows.h. */
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "pc_travel.h"

int PcTravelGameInstalled(const char *id)
{
    char dir[MAX_PATH], path[MAX_PATH], *slash;

    GetModuleFileNameA(NULL, dir, sizeof dir);
    if ((slash = strrchr(dir, '\\')) != NULL) *slash = 0;
    snprintf(path, sizeof path, "%s\\games\\%s.dll", dir, id);
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return 0;
    snprintf(path, sizeof path, "%s\\%s.pkmn", dir, id);
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

void PcTravelFullPath(const char *path, char *out, int size)
{
    if (!GetFullPathNameA(path, size, out, NULL) && size > 0) out[0] = 0;
}

int PcTravelSamePath(const char *a, const char *b)
{
    char fa[MAX_PATH], fb[MAX_PATH];
    if (!GetFullPathNameA(a, sizeof fa, fa, NULL) || !GetFullPathNameA(b, sizeof fb, fb, NULL)) return 0;
    return _stricmp(fa, fb) == 0;
}

int PcTravelRead(struct PcTravelFile *t)
{
    FILE *f = fopen(PC_TRAVEL_FILE, "rb");
    int ok;
    if (!f) return 0;
    ok = fread(t, sizeof *t, 1, f) == 1 && !memcmp(t->magic, PC_TRAVEL_MAGIC, 8);
    fclose(f);
    return ok;
}

/* to a temp file, then swapped in: never a half-written trip */
int PcTravelWrite(const struct PcTravelFile *t)
{
    FILE *f = fopen(PC_TRAVEL_FILE ".tmp", "wb");
    if (!f) return 0;
    if (fwrite(t, sizeof *t, 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return MoveFileExA(PC_TRAVEL_FILE ".tmp", PC_TRAVEL_FILE, MOVEFILE_REPLACE_EXISTING) != 0;
}

void PcTravelDelete(void)
{
    DeleteFileA(PC_TRAVEL_FILE);
}
