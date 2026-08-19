#ifdef PORTABLE
#include "global.h"
#include "characters.h"

// ---------------------------------------------------------------------------
// Hoenn map section names.
//
// src/region_map.c calls Hoenn_GetMapsecName() at the top of GetMapName(), for
// EVERY map name popup — Kanto maps included. Until this file existed the
// symbol was never defined anywhere, and the PC link runs with
// -Wl,--warn-unresolved-symbols, so the build happily produced a binary in
// which that call jumped to a bogus address. It returned a garbage non-NULL
// pointer, GetMapName fed it to StringCopy, StringCopy scanned for an EOS
// (0xFF) that was never coming, and the resulting overrun smashed the stack —
// crashing a second or two after any save was loaded and the overworld drew
// its first map name popup.
//
// Contract: return a pointer to an EOS-terminated, FRLG-encoded string for
// Hoenn map sections, or NULL for anything else. NULL is safe: Hoenn MAPSECs
// (208+) are above MAPSEC_NONE (197), so GetMapName's vanilla path falls into
// its StringFill branch and renders a blank name instead of faulting.
// ---------------------------------------------------------------------------

// The in-use Hoenn MAPSEC base. NOTE: this is 208, matching the fly-destination
// table in src/region_map.c (SetFlyWarpDestination). It deliberately does NOT
// use HOENN_MAPSEC_BASE from constants/hoenn.h, which says 210 and disagrees
// with every actual use site.
#define HOENN_MAPSEC_FIRST 208

static const char *const sHoennMapsecNames[] = {
    "LITTLEROOT TOWN",   // 208
    "OLDALE TOWN",       // 209
    "PETALBURG CITY",    // 210
    "RUSTBORO CITY",     // 211
    "DEWFORD TOWN",      // 212
    "SLATEPORT CITY",    // 213
    "MAUVILLE CITY",     // 214
    "VERDANTURF TOWN",   // 215
    "FALLARBOR TOWN",    // 216
    "LAVARIDGE TOWN",    // 217
    "FORTREE CITY",      // 218
    "LILYCOVE CITY",     // 219
    "MOSSDEEP CITY",     // 220
    "SOOTOPOLIS CITY",   // 221
    "EVER GRANDE CITY",  // 222
    "PACIFIDLOG TOWN",   // 223
};

#define HOENN_MAPSEC_COUNT ((int)(sizeof(sHoennMapsecNames) / sizeof(sHoennMapsecNames[0])))

// Longest name above is 16 chars; leave room for that plus EOS and slack.
#define HOENN_NAME_BUF_SIZE 24

// Encode ASCII into the FRLG text encoding, always EOS-terminating.
static void EncodeGameString(u8 *dst, const char *src, int dstSize)
{
    int i = 0;

    for (; src[i] != '\0' && i < dstSize - 1; i++)
    {
        char c = src[i];

        if (c >= 'A' && c <= 'Z')      dst[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') dst[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') dst[i] = 0xA1 + (c - '0');
        else if (c == ' ')             dst[i] = 0x00;   // CHAR_SPACE
        else if (c == '.')             dst[i] = 0xAD;
        else if (c == '-')             dst[i] = 0xAE;
        else                           dst[i] = 0x00;
    }

    dst[i] = EOS;   // 0xFF — the terminator StringCopy is looking for
}

const u8 *Hoenn_GetMapsecName(u16 mapsec)
{
    static u8 sNameBuf[HOENN_NAME_BUF_SIZE];
    int idx;

    if (mapsec < HOENN_MAPSEC_FIRST)
        return NULL;

    idx = mapsec - HOENN_MAPSEC_FIRST;
    if (idx >= HOENN_MAPSEC_COUNT)
        return NULL;    // Hoenn route / unnamed section: let the caller blank it

    EncodeGameString(sNameBuf, sHoennMapsecNames[idx], HOENN_NAME_BUF_SIZE);
    return sNameBuf;
}

#endif // PORTABLE
