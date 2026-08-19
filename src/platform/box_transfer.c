/*
 * Pokemon Box — Transfer System for PokeRecomp
 * Side-by-side box viewer using GBA ROM assets (icons, palettes).
 * Reads pokefirered.sav and pokeleafgreen.sav directly.
 */
#ifdef PLATFORM_SDL2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>

#define STB_TRUETYPE_STATIC
#include "../../lib/stb_truetype.h"

/* ── GBA ROM data externs ── */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed   int   s32;

extern const u8  *const gMonIconTable[];
extern const u16 gMonIconPalettes[][16];
extern const u8  gMonIconPaletteIndices[];
extern const u8  gSpeciesNames[][11];

/* ── Save format constants (GBA layout) ── */
#define SECTOR_SIZE       4096
#define SECTOR_DATA_SIZE  3968
#define NUM_SECTORS_SLOT  14
#define SAVE_SIGNATURE    0x08012025
#define FLASH_SIZE        (32 * SECTOR_SIZE)

#define TOTAL_BOXES    14
#define IN_BOX_COUNT   30
#define BOX_COLS       6
#define BOX_ROWS       5
#define BOX_NAME_LEN   9
#define NICK_LEN       10
#define PLAYER_NAME_LEN 8
#define NUM_SPECIES_MAX 412

#define STORAGE_OFFSET_CURRENT_BOX  0x0000
#define STORAGE_OFFSET_BOXES        0x0004
#define STORAGE_OFFSET_BOX_NAMES    0x8344
#define STORAGE_OFFSET_WALLPAPERS   0x83C2
#define STORAGE_TOTAL_SIZE          0x83D0

#define BOXMON_SIZE 80
#define PARTYMON_SIZE 100

/* FR/LG SaveBlock1 party offsets */
#define SB1_PARTY_COUNT_FRLG  0x0034
#define SB1_PARTY_FRLG        0x0038
#define SB1_MAX_SIZE_FRLG   0x3D68

/* Ruby/Sapphire SaveBlock1 party offsets */
#define SB1_PARTY_COUNT_RS    0x0234
#define SB1_PARTY_RS          0x0238
#define SB1_MAX_SIZE_RS     0x3D68

#define SB1_MAX_SIZE          0x3D68

/* ── Gen 2 constants ── */
#define GEN2_SAVE_SIZE       0x8000
#define GEN2_BOXES           14
#define GEN2_MONS_PER_BOX    20
#define GEN2_BOXMON_SIZE     33
#define GEN2_PARTYMON_SIZE   48
#define GEN2_BOX_SIZE        0x440
#define GEN2_NAME_LEN        11

/* Gen 2 SRAM offsets (flat file = bank * 0x2000 + offset_in_bank) */
/* Bank 2 = file offset 0x4000 */
#define GEN2_BANK2           0x4000
#define GEN2_CHECK1          (GEN2_BANK2 + 0x0001)
#define GEN2_PLAYERDATA      (GEN2_BANK2 + 0x0002)
#define GEN2_PLAYER_NAME     (GEN2_BANK2 + 0x0004)
#define GEN2_POKEMONDATA     (GEN2_BANK2 + 0x019B)
#define GEN2_PARTY_COUNT     (GEN2_BANK2 + 0x019B)
#define GEN2_PARTY_SPECIES   (GEN2_BANK2 + 0x019C)
#define GEN2_PARTY_MONS      (GEN2_BANK2 + 0x01A3)
#define GEN2_PARTY_OTS       (GEN2_BANK2 + 0x0283)
#define GEN2_PARTY_NICKS     (GEN2_BANK2 + 0x02B1)
#define GEN2_CUR_BOX         (GEN2_BANK2 + 0x02D6)
#define GEN2_BOX_NAMES       (GEN2_BANK2 + 0x02D9)
#define GEN2_ACTIVE_BOX      (GEN2_BANK2 + 0x0700)
#define GEN2_CHECKSUM        (GEN2_BANK2 + 0x0500)
/* Bank 3 = file offset 0x6000: Boxes 1-7 */
#define GEN2_BOXES_1_7       0x6000
/* After bank 3 we need to check if data continues */

/* GBA sector data sizes for checksum (matches sSaveSlotLayout) */
static const u16 sSectorDataSizes[NUM_SECTORS_SLOT] = {
    0x0F24,                             /* sector  0: SaveBlock2 */
    0x0F80, 0x0F80, 0x0F80, 0x0EE8,    /* sectors 1-4: SaveBlock1 */
    0x0F80, 0x0F80, 0x0F80, 0x0F80,    /* sectors 5-12: PokemonStorage */
    0x0F80, 0x0F80, 0x0F80, 0x0F80,
    0x07D0                              /* sector 13: PokemonStorage tail */
};

/* Substruct position table: sSubPos[personality%24][substructType] = position */
static const u8 sSubPos[24][4] = {
    {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,3,1,2},{0,2,3,1},{0,3,2,1},
    {1,0,2,3},{1,0,3,2},{2,0,1,3},{3,0,1,2},{2,0,3,1},{3,0,2,1},
    {1,2,0,3},{1,3,0,2},{2,1,0,3},{3,1,0,2},{2,3,0,1},{3,2,0,1},
    {1,2,3,0},{1,3,2,0},{2,1,3,0},{3,1,2,0},{2,3,1,0},{3,2,1,0},
};

/* Wallpaper background colors (RGB approximations) */
static const float sWallpaperColors[16][3] = {
    {0.12f, 0.35f, 0.12f},  /* FOREST */
    {0.35f, 0.35f, 0.40f},  /* CITY */
    {0.50f, 0.40f, 0.22f},  /* DESERT */
    {0.40f, 0.45f, 0.18f},  /* SAVANNA */
    {0.40f, 0.28f, 0.18f},  /* CRAG */
    {0.50f, 0.15f, 0.08f},  /* VOLCANO */
    {0.60f, 0.68f, 0.78f},  /* SNOW */
    {0.22f, 0.18f, 0.28f},  /* CAVE */
    {0.55f, 0.58f, 0.30f},  /* BEACH */
    {0.10f, 0.22f, 0.42f},  /* SEAFLOOR */
    {0.18f, 0.38f, 0.48f},  /* RIVER */
    {0.38f, 0.52f, 0.72f},  /* SKY */
    {0.10f, 0.06f, 0.22f},  /* STARS */
    {0.48f, 0.28f, 0.32f},  /* POKECENTER */
    {0.38f, 0.32f, 0.28f},  /* TILES */
    {0.42f, 0.42f, 0.45f},  /* SIMPLE */
};

/* ── Shiny check ── */
static int IsShiny(u32 otId, u32 personality) {
    u16 a = (u16)(otId >> 16) ^ (u16)(otId & 0xFFFF);
    u16 b = (u16)(personality >> 16) ^ (u16)(personality & 0xFFFF);
    return (a ^ b) < 8;
}

/* ── Parsed box mon info ── */
struct BtMon {
    u16 species;
    u16 heldItem;
    u8  level;
    u8  isEgg;
    u8  hasSpecies;
    u8  isShiny;
    u32 personality;
    u32 otId;
    char nickname[NICK_LEN + 1];
    char speciesName[NICK_LEN + 1];
};

struct BtBox {
    struct BtMon mons[IN_BOX_COUNT];
    char name[BOX_NAME_LEN + 1];
    u8 wallpaper;
};

struct BtSave {
    struct BtBox boxes[TOTAL_BOXES];
    struct BtMon party[6];
    u8   partyCount;
    u8   currentBox;
    char playerName[PLAYER_NAME_LEN + 1];
    u8   valid;
    int  gameIdx;
    u32  partyCountOff;
    u32  partyOff;
    u8   gen2Raw[6][GEN2_PARTYMON_SIZE]; /* raw Gen 2 party data for transfers */
    u8   gen2Sram[GEN2_SAVE_SIZE];       /* full Gen 2 SRAM for write-back */
    int  gen2PartyBase;                   /* offset of party in SRAM */
    u8   storage[STORAGE_TOTAL_SIZE];
    u8   saveBlock1[SB1_MAX_SIZE];
    u8   flash[FLASH_SIZE];
    int  activeSlotBase;
    u8   sectorMap[NUM_SECTORS_SLOT]; /* physical sector for each logical id */
};

/* ── UI constants ── */
#define BT_W   960
#define BT_H   740
#define ICON_SZ  40
#define CELL_SZ  46
#define GRID_X_OFF 30
#define GRID_Y_OFF 80
#define SIDE_W  (BT_W / 2)
#define INFO_H  100
#define GAME_BTN_H 28
#define GAME_BAR_Y 2
#define PARTY_H  52

/* ── GBA text decode ── */
static char GbaChar(u8 c) {
    if (c == 0x00) return ' ';
    if (c >= 0xA1 && c <= 0xAA) return '0' + (c - 0xA1);
    if (c == 0xAB) return '!';
    if (c == 0xAC) return '?';
    if (c == 0xAD) return '.';
    if (c == 0xAE) return '-';
    if (c == 0xB5) return 'M';  /* male */
    if (c == 0xB6) return 'F';  /* female */
    if (c >= 0xBB && c <= 0xD4) return 'A' + (c - 0xBB);
    if (c >= 0xD5 && c <= 0xEE) return 'a' + (c - 0xD5);
    if (c == 0xF0) return ':';
    if (c == 0xFF) return '\0';
    return '?';
}

/* Gen 2 text decode (different encoding from GBA) */
static char Gen2Char(u8 c) {
    if (c == 0x50) return '\0'; /* terminator */
    if (c == 0x7F) return ' ';
    if (c >= 0x80 && c <= 0x99) return 'A' + (c - 0x80);
    if (c >= 0x9A && c <= 0xB3) return 'a' + (c - 0x9A);
    if (c >= 0xF6 && c <= 0xFF) return '0' + (c - 0xF6);
    if (c == 0xE3) return '-';
    if (c == 0xE8) return '.';
    if (c == 0xEF) return '!'; /* male */
    if (c == 0xF5) return '?'; /* female */
    return '?';
}

static void Gen2ToAscii(const u8 *src, char *dst, int maxLen) {
    int i;
    for (i = 0; i < maxLen; i++) {
        if (src[i] == 0x50) break;
        dst[i] = Gen2Char(src[i]);
    }
    dst[i] = '\0';
}

/* Gen 2 species name table (first 251 Pokemon) */
static const char *sGen2SpeciesNames[] = {
    "???",
    "BULBASAUR","IVYSAUR","VENUSAUR","CHARMANDER","CHARMELEON","CHARIZARD",
    "SQUIRTLE","WARTORTLE","BLASTOISE","CATERPIE","METAPOD","BUTTERFREE",
    "WEEDLE","KAKUNA","BEEDRILL","PIDGEY","PIDGEOTTO","PIDGEOT",
    "RATTATA","RATICATE","SPEAROW","FEAROW","EKANS","ARBOK",
    "PIKACHU","RAICHU","SANDSHREW","SANDSLASH","NIDORAN F","NIDORINA",
    "NIDOQUEEN","NIDORAN M","NIDORINO","NIDOKING","CLEFAIRY","CLEFABLE",
    "VULPIX","NINETALES","JIGGLYPUFF","WIGGLYTUFF","ZUBAT","GOLBAT",
    "ODDISH","GLOOM","VILEPLUME","PARAS","PARASECT","VENONAT","VENOMOTH",
    "DIGLETT","DUGTRIO","MEOWTH","PERSIAN","PSYDUCK","GOLDUCK",
    "MANKEY","PRIMEAPE","GROWLITHE","ARCANINE","POLIWAG","POLIWHIRL",
    "POLIWRATH","ABRA","KADABRA","ALAKAZAM","MACHOP","MACHOKE","MACHAMP",
    "BELLSPROUT","WEEPINBELL","VICTREEBEL","TENTACOOL","TENTACRUEL",
    "GEODUDE","GRAVELER","GOLEM","PONYTA","RAPIDASH","SLOWPOKE","SLOWBRO",
    "MAGNEMITE","MAGNETON","FARFETCHD","DODUO","DODRIO","SEEL","DEWGONG",
    "GRIMER","MUK","SHELLDER","CLOYSTER","GASTLY","HAUNTER","GENGAR",
    "ONIX","DROWZEE","HYPNO","KRABBY","KINGLER","VOLTORB","ELECTRODE",
    "EXEGGCUTE","EXEGGUTOR","CUBONE","MAROWAK","HITMONLEE","HITMONCHAN",
    "LICKITUNG","KOFFING","WEEZING","RHYHORN","RHYDON","CHANSEY",
    "TANGELA","KANGASKHAN","HORSEA","SEADRA","GOLDEEN","SEAKING",
    "STARYU","STARMIE","MR.MIME","SCYTHER","JYNX","ELECTABUZZ",
    "MAGMAR","PINSIR","TAUROS","MAGIKARP","GYARADOS","LAPRAS",
    "DITTO","EEVEE","VAPOREON","JOLTEON","FLAREON","PORYGON",
    "OMANYTE","OMASTAR","KABUTO","KABUTOPS","AERODACTYL","SNORLAX",
    "ARTICUNO","ZAPDOS","MOLTRES","DRATINI","DRAGONAIR","DRAGONITE",
    "MEWTWO","MEW",
    "CHIKORITA","BAYLEEF","MEGANIUM","CYNDAQUIL","QUILAVA","TYPHLOSION",
    "TOTODILE","CROCONAW","FERALIGATR","SENTRET","FURRET","HOOTHOOT","NOCTOWL",
    "LEDYBA","LEDIAN","SPINARAK","ARIADOS","CROBAT","CHINCHOU","LANTURN",
    "PICHU","CLEFFA","IGGLYBUFF","TOGEPI","TOGETIC","NATU","XATU",
    "MAREEP","FLAAFFY","AMPHAROS","BELLOSSOM","MARILL","AZUMARILL",
    "SUDOWOODO","POLITOED","HOPPIP","SKIPLOOM","JUMPLUFF","AIPOM",
    "SUNKERN","SUNFLORA","YANMA","WOOPER","QUAGSIRE","ESPEON","UMBREON",
    "MURKROW","SLOWKING","MISDREAVUS","UNOWN","WOBBUFFET","GIRAFARIG",
    "PINECO","FORRETRESS","DUNSPARCE","GLIGAR","STEELIX","SNUBBULL",
    "GRANBULL","QWILFISH","SCIZOR","SHUCKLE","HERACROSS","SNEASEL",
    "TEDDIURSA","URSARING","SLUGMA","MAGCARGO","SWINUB","PILOSWINE",
    "CORSOLA","REMORAID","OCTILLERY","DELIBIRD","MANTINE","SKARMORY",
    "HOUNDOUR","HOUNDOOM","KINGDRA","PHANPY","DONPHAN","PORYGON2",
    "STANTLER","SMEARGLE","TYROGUE","HITMONTOP","SMOOCHUM","ELEKID",
    "MAGBY","MILTANK","BLISSEY","RAIKOU","ENTEI","SUICUNE",
    "LARVITAR","PUPITAR","TYRANITAR","LUGIA","HO-OH","CELEBI",
};
#define GEN2_NUM_SPECIES 251

/* ── Gen 2 save parser ── */
static int ParseGen2Save(struct BtSave *save, const char *path) {
    int gameIdx = save->gameIdx;
    memset(save, 0, sizeof(*save));
    save->gameIdx = gameIdx;

    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fileSize < GEN2_SAVE_SIZE) { fclose(f); return 0; }

    u8 sram[GEN2_SAVE_SIZE];
    if (fread(sram, 1, GEN2_SAVE_SIZE, f) != GEN2_SAVE_SIZE) { fclose(f); return 0; }
    fclose(f);

    /* Store full SRAM for write-back */
    memcpy(save->gen2Sram, sram, GEN2_SAVE_SIZE);
    save->gen2PartyBase = -1;

    /* Find the save base by searching for sentinel 0x63 */
    int saveBase = -1;
    for (int i = 0; i < GEN2_SAVE_SIZE - 0x800; i++) {
        if (sram[i] == 0x63) {
            /* Check if player name follows (text bytes 0x80-0xB3 or terminator 0x50) */
            u8 nameStart = sram[i + 3]; /* +1=sPlayerData, +2=wPlayerID(2), +3 would be wrong... */
            /* sCheckValue1 is 1 byte before sGameData. sGameData starts at i+1.
               wPlayerID at sGameData+0 (2 bytes), wPlayerName at sGameData+2 */
            u8 nb = sram[i + 1 + 2]; /* first byte of player name */
            if ((nb >= 0x80 && nb <= 0xB3) || nb == 0x50) {
                saveBase = i; /* i = offset of sCheckValue1 */
                break;
            }
        }
    }
    if (saveBase < 0) return 0;

    int gameDataBase = saveBase + 1; /* sGameData starts right after sCheckValue1 */

    /* Player name: wPlayerID(2) + wPlayerName(11) */
    Gen2ToAscii(&sram[gameDataBase + 2], save->playerName, 7);

    /* Find party data: scan for count byte (1-6) followed by valid species */
    int partyBase = -1;
    for (int off = gameDataBase + 0x100; off < gameDataBase + 0x1000 && off < GEN2_SAVE_SIZE - 0x200; off++) {
        u8 cnt = sram[off];
        if (cnt >= 1 && cnt <= 6) {
            int valid = 1;
            for (int k = 0; k < cnt; k++) {
                u8 sp = sram[off + 1 + k];
                if (sp == 0 || sp > GEN2_NUM_SPECIES) { valid = 0; break; }
            }
            if (valid && sram[off + 1 + cnt] == 0xFF) { /* terminator after species list */
                partyBase = off;
                break;
            }
        }
    }

    /* Default box names for Gen 2 */
    save->currentBox = 0;
    for (int b = 0; b < GEN2_BOXES && b < TOTAL_BOXES; b++) {
        snprintf(save->boxes[b].name, BOX_NAME_LEN, "BOX %d", b + 1);
        save->boxes[b].wallpaper = b % 16;
    }

    /* Try to find wCurBox and wBoxNames by scanning for box name text */
    for (int off = gameDataBase + 0x200; off < gameDataBase + 0x500 && off < GEN2_SAVE_SIZE - 140; off++) {
        /* wBoxNames: 14 names of 9 bytes each. First should be text. */
        u8 first = sram[off];
        if (first >= 0x80 && first <= 0x99) { /* uppercase letter */
            int looksLikeNames = 1;
            for (int b = 1; b < 3; b++) {
                u8 nb = sram[off + b * 9];
                if (nb < 0x80 || nb > 0xB3) { looksLikeNames = 0; break; }
            }
            if (looksLikeNames) {
                /* wCurBox is typically 3 bytes before wBoxNames */
                int curBoxOff = off - 3;
                if (curBoxOff >= 0 && sram[curBoxOff] < GEN2_BOXES)
                    save->currentBox = sram[curBoxOff] & 0x0F;
                for (int b = 0; b < GEN2_BOXES && b < TOTAL_BOXES; b++)
                    Gen2ToAscii(&sram[off + b * 9], save->boxes[b].name, 8);
                break;
            }
        }
    }

    /* Party */
    save->gen2PartyBase = partyBase;
    if (partyBase < 0) { save->valid = 1; return 1; }

    save->partyCount = sram[partyBase];
    if (save->partyCount > 6) save->partyCount = 6;

    for (int p = 0; p < (int)save->partyCount; p++) {
        u8 speciesId = sram[partyBase + 1 + p];
        int monOff = partyBase + 1 + 6 + 1 + p * GEN2_PARTYMON_SIZE; /* count(1) + species(6) + terminator(1) */
        u8 *mon = &sram[monOff];
        struct BtMon *bt = &save->party[p];

        /* Store raw Gen 2 data for transfer conversion */
        if (monOff + GEN2_PARTYMON_SIZE <= GEN2_SAVE_SIZE)
            memcpy(save->gen2Raw[p], mon, GEN2_PARTYMON_SIZE);

        bt->species = speciesId;
        bt->heldItem = mon[0x01];
        bt->level = mon[0x1F]; /* current level in party struct */
        bt->personality = speciesId; /* no personality in Gen 2 */
        bt->otId = (mon[0x06] << 8) | mon[0x07];
        bt->isShiny = 0;
        bt->isEgg = 0;

        /* Gen 2 shiny: DVs at offset 0x15-0x16 */
        u8 dvHi = mon[0x15];
        u8 dvLo = mon[0x16];
        u8 atkDV = (dvHi >> 4) & 0xF;
        u8 defDV = dvHi & 0xF;
        u8 spdDV = (dvLo >> 4) & 0xF;
        u8 spcDV = dvLo & 0xF;
        bt->isShiny = (atkDV == 10 && defDV == 10 && spdDV == 10 && spcDV == 10);

        /* Nickname: after all mons + OT names */
        {
            int otsOff = partyBase + 1 + 6 + 1 + 6 * GEN2_PARTYMON_SIZE;
            int nicksOff = otsOff + 6 * GEN2_NAME_LEN;
            if (nicksOff + (p + 1) * GEN2_NAME_LEN <= GEN2_SAVE_SIZE)
                Gen2ToAscii(&sram[nicksOff + p * GEN2_NAME_LEN], bt->nickname, 10);
        }

        /* Species name */
        if (speciesId > 0 && speciesId <= GEN2_NUM_SPECIES)
            strncpy(bt->speciesName, sGen2SpeciesNames[speciesId], NICK_LEN);
        else
            strcpy(bt->speciesName, "???");
    }

    /* Parse boxes from SRAM */
    /* Active box: scan for a box structure after the main save in the same bank */
    /* A box starts with count(1) + species[20] + terminator(0xFF) + mon data */
    /* Stored boxes are in subsequent SRAM banks (7 per bank) */

    /* Helper: try to parse a box at a given offset */
    #define TRY_PARSE_BOX(boxIdx, addr) do { \
        if ((addr) + GEN2_BOX_SIZE <= GEN2_SAVE_SIZE) { \
            u8 cnt = sram[(addr)]; \
            if (cnt <= GEN2_MONS_PER_BOX) { \
                for (int s = 0; s < (int)cnt && s < IN_BOX_COUNT; s++) { \
                    u8 sid = sram[(addr) + 1 + s]; \
                    if (sid == 0 || sid > GEN2_NUM_SPECIES) continue; \
                    int moff = (addr) + 1 + GEN2_MONS_PER_BOX + 1 + s * GEN2_BOXMON_SIZE; \
                    u8 *md = &sram[moff]; \
                    struct BtMon *bt = &save->boxes[(boxIdx)].mons[s]; \
                    bt->species = sid; \
                    bt->heldItem = md[0x01]; \
                    bt->level = md[0x1F]; \
                    bt->personality = sid; \
                    bt->otId = (md[0x06] << 8) | md[0x07]; \
                    bt->isEgg = 0; \
                    bt->isShiny = (((md[0x15]>>4)&0xF)==10 && (md[0x15]&0xF)==10 && \
                                   ((md[0x16]>>4)&0xF)==10 && (md[0x16]&0xF)==10); \
                    int nkOff = (addr) + 1 + GEN2_MONS_PER_BOX + 1 + GEN2_MONS_PER_BOX * GEN2_BOXMON_SIZE \
                                + GEN2_MONS_PER_BOX * GEN2_NAME_LEN + s * GEN2_NAME_LEN; \
                    if (nkOff + GEN2_NAME_LEN <= GEN2_SAVE_SIZE) \
                        Gen2ToAscii(&sram[nkOff], bt->nickname, 10); \
                    if (sid > 0 && sid <= GEN2_NUM_SPECIES) \
                        strncpy(bt->speciesName, sGen2SpeciesNames[sid], NICK_LEN); \
                    else strcpy(bt->speciesName, "???"); \
                } \
            } \
        } \
    } while(0)

    /* Gen 2 PC box parsing disabled — SRAM layout varies between
       Gold/Silver/Crystal and the PC port's recompiled save format.
       Party data is reliable; box data needs exact offset mapping. */

    #undef TRY_PARSE_BOX

    save->valid = 1;
    return 1;
}

static void GbaToAscii(const u8 *src, char *dst, int maxLen) {
    int i;
    for (i = 0; i < maxLen; i++) {
        if (src[i] == 0xFF) break;
        dst[i] = GbaChar(src[i]);
    }
    dst[i] = '\0';
}

/* ── Save file checksum ── */
static u16 CalcSectorChecksum(const u8 *data, u16 size) {
    u32 sum = 0;
    for (u16 i = 0; i < size / 4; i++) {
        u32 val;
        memcpy(&val, data + i * 4, 4);
        sum += val;
    }
    return (u16)((sum >> 16) + sum);
}

/* ── Save file parsing ── */
static int ParseSave(struct BtSave *save, const char *path) {
    int gameIdx = save->gameIdx;
    u32 pcOff = save->partyCountOff;
    u32 ppOff = save->partyOff;
    memset(save, 0, sizeof(*save));
    save->gameIdx = gameIdx;
    save->partyCountOff = pcOff;
    save->partyOff = ppOff;

    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fileSize < FLASH_SIZE) { fclose(f); return 0; }

    if (fread(save->flash, 1, FLASH_SIZE, f) != FLASH_SIZE) { fclose(f); return 0; }
    fclose(f);

    /* Find active save slot by comparing counters */
    u32 counter1 = 0, counter2 = 0;
    int found1 = 0, found2 = 0;

    for (int i = 0; i < NUM_SECTORS_SLOT; i++) {
        u8 *sec = &save->flash[i * SECTOR_SIZE];
        u32 sig;
        memcpy(&sig, sec + SECTOR_DATA_SIZE + 116 + 4, 4); /* signature at footer+4 */
        if (sig == SAVE_SIGNATURE) {
            u32 cnt;
            memcpy(&cnt, sec + SECTOR_DATA_SIZE + 116 + 8, 4); /* counter at footer+8 */
            if (!found1 || cnt > counter1) { counter1 = cnt; found1 = 1; }
        }
    }
    for (int i = 0; i < NUM_SECTORS_SLOT; i++) {
        u8 *sec = &save->flash[(i + NUM_SECTORS_SLOT) * SECTOR_SIZE];
        u32 sig;
        memcpy(&sig, sec + SECTOR_DATA_SIZE + 116 + 4, 4);
        if (sig == SAVE_SIGNATURE) {
            u32 cnt;
            memcpy(&cnt, sec + SECTOR_DATA_SIZE + 116 + 8, 4);
            if (!found2 || cnt > counter2) { counter2 = cnt; found2 = 1; }
        }
    }

    if (!found1 && !found2) return 0;

    int slotBase;
    if (found1 && found2)
        slotBase = (counter1 >= counter2) ? 0 : NUM_SECTORS_SLOT;
    else
        slotBase = found1 ? 0 : NUM_SECTORS_SLOT;

    save->activeSlotBase = slotBase;

    /* Map logical sector IDs to physical sectors, reconstruct data */
    u8 saveBlock2[SECTOR_DATA_SIZE];
    memset(saveBlock2, 0, sizeof(saveBlock2));
    memset(save->saveBlock1, 0, SB1_MAX_SIZE);
    memset(save->storage, 0, STORAGE_TOTAL_SIZE);
    memset(save->sectorMap, 0xFF, sizeof(save->sectorMap));

    for (int i = 0; i < NUM_SECTORS_SLOT; i++) {
        int phys = slotBase + i;
        u8 *sec = &save->flash[phys * SECTOR_SIZE];
        u32 sig;
        memcpy(&sig, sec + SECTOR_DATA_SIZE + 116 + 4, 4);
        if (sig != SAVE_SIGNATURE) continue;

        u16 id;
        memcpy(&id, sec + SECTOR_DATA_SIZE + 116, 2);
        if (id >= NUM_SECTORS_SLOT) continue;

        save->sectorMap[id] = (u8)phys;

        if (id == 0) {
            memcpy(saveBlock2, sec, sSectorDataSizes[0]);
        }
        else if (id >= 1 && id <= 4) {
            u32 sb1Off = (id - 1) * (u32)SECTOR_DATA_SIZE;
            u16 copySize = sSectorDataSizes[id];
            if (sb1Off + copySize > SB1_MAX_SIZE)
                copySize = (u16)(SB1_MAX_SIZE - sb1Off);
            memcpy(&save->saveBlock1[sb1Off], sec, copySize);
        }
        else if (id >= 5 && id <= 13) {
            u32 storageOff = (id - 5) * (u32)SECTOR_DATA_SIZE;
            u16 copySize = sSectorDataSizes[id];
            if (storageOff + copySize > STORAGE_TOTAL_SIZE)
                copySize = (u16)(STORAGE_TOTAL_SIZE - storageOff);
            memcpy(&save->storage[storageOff], sec, copySize);
        }
    }

    /* Extract player name from SaveBlock2 */
    GbaToAscii(saveBlock2, save->playerName, PLAYER_NAME_LEN);

    /* Extract current box */
    save->currentBox = save->storage[STORAGE_OFFSET_CURRENT_BOX];
    if (save->currentBox >= TOTAL_BOXES) save->currentBox = 0;

    /* Extract box names and wallpapers */
    for (int b = 0; b < TOTAL_BOXES; b++) {
        GbaToAscii(&save->storage[STORAGE_OFFSET_BOX_NAMES + b * BOX_NAME_LEN],
                   save->boxes[b].name, BOX_NAME_LEN);
        save->boxes[b].wallpaper = save->storage[STORAGE_OFFSET_WALLPAPERS + b];
        if (save->boxes[b].wallpaper >= 16) save->boxes[b].wallpaper = 15;
    }

    /* Decrypt and extract each BoxPokemon */
    for (int b = 0; b < TOTAL_BOXES; b++) {
        for (int s = 0; s < IN_BOX_COUNT; s++) {
            u32 monOff = STORAGE_OFFSET_BOXES + (b * IN_BOX_COUNT + s) * BOXMON_SIZE;
            u8 *raw = &save->storage[monOff];
            struct BtMon *mon = &save->boxes[b].mons[s];

            u32 personality, otId;
            memcpy(&personality, raw, 4);
            memcpy(&otId, raw + 4, 4);
            mon->personality = personality;
            mon->otId = otId;

            /* Decode nickname (unencrypted, GBA encoding) */
            GbaToAscii(raw + 8, mon->nickname, NICK_LEN);

            u8 flags = raw[0x13];
            mon->hasSpecies = (flags >> 1) & 1;
            mon->isEgg = (flags >> 2) & 1;

            if (personality == 0 && otId == 0) {
                mon->species = 0;
                mon->level = 0;
                mon->heldItem = 0;
                continue;
            }

            /* Decrypt secure data (work on a copy) */
            u32 secure[12];
            memcpy(secure, raw + 0x20, 48);
            u32 key = personality ^ otId;
            for (int j = 0; j < 12; j++) secure[j] ^= key;

            /* Find growth substruct (type 0) */
            int growthPos = sSubPos[personality % 24][0];
            u8 *growth = (u8 *)&secure[growthPos * 3]; /* 12 bytes = 3 u32s per substruct */

            u16 species;
            memcpy(&species, growth, 2);
            if (species > NUM_SPECIES_MAX) species = 0;
            mon->species = species;

            u16 heldItem;
            memcpy(&heldItem, growth + 2, 2);
            mon->heldItem = heldItem;
            mon->isShiny = IsShiny(otId, personality);

            /* Get level from experience (simplified: use misc substruct metLevel) */
            int miscPos = sSubPos[personality % 24][3];
            u8 *misc = (u8 *)&secure[miscPos * 3];
            mon->level = misc[2] & 0x7F; /* metLevel is bits 0-6 of byte offset 2 in misc */

            /* Decode species name */
            if (species > 0 && species <= NUM_SPECIES_MAX) {
                GbaToAscii(gSpeciesNames[species], mon->speciesName, NICK_LEN);
            } else {
                strcpy(mon->speciesName, "???");
            }
        }
    }

    /* Extract party from SaveBlock1 (game-specific offsets) */
    {
    u32 pcOff = save->partyCountOff;
    u32 ppOff = save->partyOff;
    save->partyCount = save->saveBlock1[pcOff];
    if (save->partyCount > 6) save->partyCount = 6;

    for (int p = 0; p < (int)save->partyCount; p++) {
        u8 *raw = &save->saveBlock1[ppOff + p * PARTYMON_SIZE];
        struct BtMon *mon = &save->party[p];

        u32 personality, otId;
        memcpy(&personality, raw, 4);
        memcpy(&otId, raw + 4, 4);
        mon->personality = personality;
        mon->otId = otId;

        GbaToAscii(raw + 8, mon->nickname, NICK_LEN);
        u8 flags = raw[0x13];
        mon->hasSpecies = (flags >> 1) & 1;
        mon->isEgg = (flags >> 2) & 1;

        if (personality == 0 && otId == 0) {
            mon->species = mon->level = mon->heldItem = 0;
            continue;
        }

        u32 secure[12];
        memcpy(secure, raw + 0x20, 48);
        u32 key = personality ^ otId;
        for (int j = 0; j < 12; j++) secure[j] ^= key;

        int growthPos = sSubPos[personality % 24][0];
        u8 *growth = (u8 *)&secure[growthPos * 3];
        u16 species;
        memcpy(&species, growth, 2);
        if (species > NUM_SPECIES_MAX) species = 0;
        mon->species = species;
        u16 heldItem;
        memcpy(&heldItem, growth + 2, 2);
        mon->heldItem = heldItem;
        mon->isShiny = IsShiny(otId, personality);

        /* Get actual level from party struct (right after BoxPokemon data) */
        mon->level = raw[BOXMON_SIZE + 4]; /* offset: status(4) + level(1) */

        if (species > 0 && species <= NUM_SPECIES_MAX)
            GbaToAscii(gSpeciesNames[species], mon->speciesName, NICK_LEN);
        else
            strcpy(mon->speciesName, "???");
    }
    for (int p = save->partyCount; p < 6; p++)
        memset(&save->party[p], 0, sizeof(struct BtMon));

    /* Skip party compaction on load — only compact during explicit transfers */
    }

    save->valid = 1;
    return 1;
}

/* ── Write modified storage back to save file ── */
static int WriteSave(struct BtSave *save, const char *path) {
    if (!save->valid) return 0;

    /* Write SaveBlock1 (sectors 1-4) back — includes party data */
    for (int id = 1; id <= 4; id++) {
        u8 phys = save->sectorMap[id];
        if (phys == 0xFF) continue;

        u8 *sec = &save->flash[phys * SECTOR_SIZE];
        u32 sb1Off = (id - 1) * (u32)SECTOR_DATA_SIZE;
        u16 copySize = sSectorDataSizes[id];
        if (sb1Off + copySize > SB1_MAX_SIZE)
            copySize = (u16)(SB1_MAX_SIZE - sb1Off);
        memcpy(sec, &save->saveBlock1[sb1Off], copySize);

        u16 checksum = CalcSectorChecksum(sec, sSectorDataSizes[id]);
        memcpy(sec + SECTOR_DATA_SIZE + 116 + 2, &checksum, 2);
    }

    /* Write PokemonStorage (sectors 5-13) back — includes box data */
    for (int id = 5; id <= 13; id++) {
        u8 phys = save->sectorMap[id];
        if (phys == 0xFF) continue;

        u8 *sec = &save->flash[phys * SECTOR_SIZE];
        u32 storageOff = (id - 5) * (u32)SECTOR_DATA_SIZE;
        u16 copySize = sSectorDataSizes[id];
        if (storageOff + copySize > STORAGE_TOTAL_SIZE)
            copySize = (u16)(STORAGE_TOTAL_SIZE - storageOff);
        memcpy(sec, &save->storage[storageOff], copySize);

        u16 checksum = CalcSectorChecksum(sec, sSectorDataSizes[id]);
        memcpy(sec + SECTOR_DATA_SIZE + 116 + 2, &checksum, 2);
    }

    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(save->flash, 1, FLASH_SIZE, f);
    fclose(f);
    return 1;
}

/* ── Write Gen 2 SRAM save back to file ── */
static int WriteGen2Save(struct BtSave *save, const char *path) {
    if (!save->valid || save->gen2PartyBase < 0) return 0;

    int pb = save->gen2PartyBase;

    /* Write party count */
    save->gen2Sram[pb] = save->partyCount;

    /* Write species list + terminator */
    for (int p = 0; p < 6; p++) {
        if (p < save->partyCount)
            save->gen2Sram[pb + 1 + p] = (u8)save->party[p].species;
        else
            save->gen2Sram[pb + 1 + p] = 0;
    }
    save->gen2Sram[pb + 1 + save->partyCount] = 0xFF;

    /* Write raw mon data back from gen2Raw */
    int monsOff = pb + 1 + 6 + 1; /* count + 6 species + terminator */
    for (int p = 0; p < save->partyCount; p++) {
        if (monsOff + (p + 1) * GEN2_PARTYMON_SIZE <= GEN2_SAVE_SIZE)
            memcpy(&save->gen2Sram[monsOff + p * GEN2_PARTYMON_SIZE],
                   save->gen2Raw[p], GEN2_PARTYMON_SIZE);
    }
    /* Zero remaining party slots */
    for (int p = save->partyCount; p < 6; p++) {
        if (monsOff + (p + 1) * GEN2_PARTYMON_SIZE <= GEN2_SAVE_SIZE)
            memset(&save->gen2Sram[monsOff + p * GEN2_PARTYMON_SIZE], 0, GEN2_PARTYMON_SIZE);
    }

    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(save->gen2Sram, 1, GEN2_SAVE_SIZE, f);
    fclose(f);
    printf("box_transfer: Gen 2 save written: %s\n", path);
    return 1;
}

/* ── Update a single box slot in the raw storage ── */
static void WriteBoxMonToStorage(struct BtSave *save, int box, int slot, const u8 *rawMon) {
    u32 off = STORAGE_OFFSET_BOXES + (box * IN_BOX_COUNT + slot) * BOXMON_SIZE;
    memcpy(&save->storage[off], rawMon, BOXMON_SIZE);
}

static void ReadBoxMonFromStorage(struct BtSave *save, int box, int slot, u8 *rawMon) {
    u32 off = STORAGE_OFFSET_BOXES + (box * IN_BOX_COUNT + slot) * BOXMON_SIZE;
    memcpy(rawMon, &save->storage[off], BOXMON_SIZE);
}

/* ── Re-parse a single box slot after modification ── */
static void ReparseBoxMon(struct BtSave *save, int box, int slot) {
    u32 monOff = STORAGE_OFFSET_BOXES + (box * IN_BOX_COUNT + slot) * BOXMON_SIZE;
    u8 *raw = &save->storage[monOff];
    struct BtMon *mon = &save->boxes[box].mons[slot];

    u32 personality, otId;
    memcpy(&personality, raw, 4);
    memcpy(&otId, raw + 4, 4);
    mon->personality = personality;
    mon->otId = otId;

    GbaToAscii(raw + 8, mon->nickname, NICK_LEN);
    u8 flags = raw[0x13];
    mon->hasSpecies = (flags >> 1) & 1;
    mon->isEgg = (flags >> 2) & 1;

    if (personality == 0 && otId == 0) {
        mon->species = mon->level = mon->heldItem = 0;
        strcpy(mon->speciesName, "");
        return;
    }

    u32 secure[12];
    memcpy(secure, raw + 0x20, 48);
    u32 key = personality ^ otId;
    for (int j = 0; j < 12; j++) secure[j] ^= key;

    int growthPos = sSubPos[personality % 24][0];
    u8 *growth = (u8 *)&secure[growthPos * 3];
    u16 species;
    memcpy(&species, growth, 2);
    if (species > NUM_SPECIES_MAX) species = 0;
    mon->species = species;
    u16 heldItem;
    memcpy(&heldItem, growth + 2, 2);
    mon->heldItem = heldItem;
    mon->isShiny = IsShiny(otId, personality);

    int miscPos = sSubPos[personality % 24][3];
    u8 *misc = (u8 *)&secure[miscPos * 3];
    mon->level = misc[2] & 0x7F;

    if (species > 0 && species <= NUM_SPECIES_MAX)
        GbaToAscii(gSpeciesNames[species], mon->speciesName, NICK_LEN);
    else
        strcpy(mon->speciesName, "???");
}

/* ── Icon texture cache ── */
#define ICON_CACHE_SIZE (NUM_SPECIES_MAX + 64)
static GLuint sIconTex[ICON_CACHE_SIZE];

static GLuint CreateIconTexture(u16 species) {
    if (species == 0 || species >= ICON_CACHE_SIZE) return 0;
    const u8 *tiles = gMonIconTable[species];
    if (!tiles) return 0;
    u8 palIdx = gMonIconPaletteIndices[species];
    if (palIdx > 2) palIdx = 0;
    const u16 *pal = gMonIconPalettes[palIdx];

    u8 rgba[32 * 32 * 4];
    memset(rgba, 0, sizeof(rgba));

    for (int ty = 0; ty < 4; ty++) {
        for (int tx = 0; tx < 4; tx++) {
            int tileIdx = ty * 4 + tx;
            const u8 *td = &tiles[tileIdx * 32]; /* 32 bytes per 8x8 4bpp tile */
            for (int py = 0; py < 8; py++) {
                for (int px = 0; px < 8; px++) {
                    u8 byte = td[py * 4 + px / 2];
                    u8 pIdx = (px & 1) ? ((byte >> 4) & 0xF) : (byte & 0xF);
                    int imgX = tx * 8 + px;
                    int imgY = ty * 8 + py;
                    int off = (imgY * 32 + imgX) * 4;
                    if (pIdx == 0) {
                        rgba[off] = rgba[off+1] = rgba[off+2] = rgba[off+3] = 0;
                    } else {
                        u16 c = pal[pIdx];
                        rgba[off+0] = (u8)(((c      ) & 0x1F) * 255 / 31);
                        rgba[off+1] = (u8)(((c >>  5) & 0x1F) * 255 / 31);
                        rgba[off+2] = (u8)(((c >> 10) & 0x1F) * 255 / 31);
                        rgba[off+3] = 255;
                    }
                }
            }
        }
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); /* CLAMP_TO_EDGE */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    return tex;
}

static GLuint GetIconTex(u16 species) {
    if (species == 0 || species >= ICON_CACHE_SIZE) return 0;
    if (!sIconTex[species])
        sIconTex[species] = CreateIconTexture(species);
    return sIconTex[species];
}

static void FreeIconCache(void) {
    for (int i = 0; i < ICON_CACHE_SIZE; i++) {
        if (sIconTex[i]) { glDeleteTextures(1, &sIconTex[i]); sIconTex[i] = 0; }
    }
}

/* ── GL drawing helpers ── */
static void btDrawQuad(GLuint tex, float x, float y, float w, float h, float a) {
    if (!tex) return;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(1,1,1,a);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(x,y);
    glTexCoord2f(1,0); glVertex2f(x+w,y);
    glTexCoord2f(1,1); glVertex2f(x+w,y+h);
    glTexCoord2f(0,1); glVertex2f(x,y+h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

static void btDrawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

static void btDrawGradV(float x, float y, float w, float h,
                        float r1, float g1, float b1, float a1,
                        float r2, float g2, float b2, float a2) {
    glBegin(GL_QUADS);
    glColor4f(r1,g1,b1,a1); glVertex2f(x,y);   glVertex2f(x+w,y);
    glColor4f(r2,g2,b2,a2); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

static void btDrawBorder(float x, float y, float w, float h, float t,
                         float r, float g, float b, float a) {
    btDrawRect(x,y,w,t,r,g,b,a);
    btDrawRect(x,y+h-t,w,t,r,g,b,a);
    btDrawRect(x,y,t,h,r,g,b,a);
    btDrawRect(x+w-t,y,t,h,r,g,b,a);
}

/* ── Font ── */
static GLuint sFontTex, sFontSmTex;
static stbtt_bakedchar sFontCh[96], sFontSmCh[96];

static GLuint btBakeFont(const char *path, float sz, stbtt_bakedchar *chars) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *ttf = (unsigned char *)malloc(len);
    fread(ttf, 1, len, f); fclose(f);
    unsigned char bmp[512*512];
    stbtt_BakeFontBitmap(ttf, 0, sz, bmp, 512, 512, 32, 96, chars);
    free(ttf);
    unsigned char *rgba = (unsigned char *)malloc(512*512*4);
    for (int i = 0; i < 512*512; i++) {
        rgba[i*4]=255; rgba[i*4+1]=255; rgba[i*4+2]=255; rgba[i*4+3]=bmp[i];
    }
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    free(rgba);
    return t;
}

static float btText(const char *s, float x, float y, GLuint ft, stbtt_bakedchar *ch,
                    float r, float g, float b, float a) {
    if (!ft || !s) return x;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ft);
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS);
    float sx = x;
    while (*s) {
        if (*s == '\n') { y += 22; x = sx; s++; continue; }
        if (*s >= 32 && *s < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(ch, 512, 512, *s - 32, &x, &y, &q, 1);
            glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
        }
        s++;
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    return x;
}

static float btTextWidth(const char *s, stbtt_bakedchar *ch) {
    float w = 0;
    while (*s) {
        if (*s >= 32 && *s < 128) w += ch[*s - 32].xadvance;
        s++;
    }
    return w;
}

static void btTextC(const char *s, float cx, float y, GLuint ft, stbtt_bakedchar *ch,
                    float r, float g, float b) {
    btText(s, cx - btTextWidth(s, ch) / 2, y, ft, ch, r, g, b, 1);
}

/* ── Game definitions ── */
enum {
    GAME_FIRERED = 0,
    GAME_LEAFGREEN,
    GAME_RUBY,
    GAME_SAPPHIRE,
    GAME_GOLD,
    GAME_SILVER,
    NUM_GAMES
};

static const struct {
    const char *name;
    const char *savePath;
    float color[3];
    int gen;
    u32 partyCountOff;
    u32 partyOff;
} sGameDefs[NUM_GAMES] = {
    {"FIRERED",    "pokefirered.sav",   {0.9f, 0.3f, 0.2f}, 3, SB1_PARTY_COUNT_FRLG, SB1_PARTY_FRLG},
    {"LEAFGREEN",  "pokeleafgreen.sav", {0.4f, 0.8f, 0.4f}, 3, SB1_PARTY_COUNT_FRLG, SB1_PARTY_FRLG},
    {"RUBY",       "pokeruby.sav",      {0.8f, 0.15f,0.15f},3, SB1_PARTY_COUNT_RS,   SB1_PARTY_RS},
    {"SAPPHIRE",   "pokesapphire.sav",  {0.2f, 0.4f, 0.85f},3, SB1_PARTY_COUNT_RS,   SB1_PARTY_RS},
    {"GOLD",       "pokegold.sav",      {0.85f,0.7f, 0.1f}, 2, 0, 0},
    {"SILVER",     "pokesilver.sav",    {0.7f, 0.7f, 0.8f}, 2, 0, 0},
};

/* ── UI state ── */
static struct BtSave sSaves[NUM_GAMES];
static int sGameIdx[2];    /* which game is shown on each side */
static int sCurSide;       /* 0=left, 1=right */
static int sCurBox[2];     /* current box per side */
static int sCurSlot[2];    /* cursor slot (0-29) per side */
static int sHolding;       /* 1 if holding a pokemon */
static int sHoldSide;
static int sHoldBox;
static int sHoldSlot;
static int sHoldGame;      /* which game the held mon came from */
static int sHeldFromParty; /* 1 if grabbed from party (100 bytes), 0 if from box (80 bytes) */
static u8  sHeldRaw[PARTYMON_SIZE]; /* big enough for both box (80) and party (100) */
static struct BtMon sHeldMon;
static int sFrame;
static int sUnsaved;

static int sMouseX, sMouseY, sMouseClicked;
static int sHoverSide, sHoverBox, sHoverSlot;
static int sHoverValid; /* 1 if mouse is over a valid pokemon slot */

/* Slot encoding: 0-29 = box grid, 30-35 = party */
#define PARTY_SLOT_BASE 30
#define IS_PARTY_SLOT(s) ((s) >= PARTY_SLOT_BASE)
#define PARTY_IDX(s) ((s) - PARTY_SLOT_BASE)

/* ── Gen 2 → Gen 3 stat conversion ── */
static u32 sRandState = 0x12345678;
static u32 BoxRand(void) { sRandState = sRandState * 1103515245 + 12345; return sRandState; }

static int ConvertGen2ToGen3(const u8 *gen2Mon, int gen2Species, const char *nickname,
                             u16 srcOtId, const char *trainerName, u8 *outBoxMon)
{
    memset(outBoxMon, 0, BOXMON_SIZE);
    if (gen2Species <= 0 || gen2Species > GEN2_NUM_SPECIES) return 0;

    /* Generate personality value */
    u32 personality = BoxRand() ^ (BoxRand() << 16);
    u32 otId = (u32)srcOtId | ((u32)(BoxRand() & 0xFFFF) << 16);

    /* Write header */
    memcpy(outBoxMon + 0x00, &personality, 4);
    memcpy(outBoxMon + 0x04, &otId, 4);

    /* Convert nickname: Gen 2 encoding → GBA encoding */
    u8 gbaNick[11];
    memset(gbaNick, 0xFF, 11);
    for (int i = 0; i < 10 && nickname[i]; i++) {
        char c = nickname[i];
        if (c >= 'A' && c <= 'Z') gbaNick[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') gbaNick[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') gbaNick[i] = 0xA1 + (c - '0');
        else if (c == ' ') gbaNick[i] = 0x00;
        else if (c == '-') gbaNick[i] = 0xAE;
        else if (c == '.') gbaNick[i] = 0xAD;
        else if (c == '!') gbaNick[i] = 0xAB;
        else if (c == '?') gbaNick[i] = 0xAC;
        else gbaNick[i] = 0xAC;
    }
    memcpy(outBoxMon + 0x08, gbaNick, 10);
    outBoxMon[0x12] = 0x02; /* language: English */
    outBoxMon[0x13] = 0x02; /* hasSpecies flag */

    /* OT name: convert trainer name from ASCII to GBA encoding */
    u8 otName[8];
    memset(otName, 0xFF, 8);
    if (trainerName) {
        for (int i = 0; i < 7 && trainerName[i]; i++) {
            char c = trainerName[i];
            if (c >= 'A' && c <= 'Z') otName[i] = 0xBB + (c - 'A');
            else if (c >= 'a' && c <= 'z') otName[i] = 0xD5 + (c - 'a');
            else if (c >= '0' && c <= '9') otName[i] = 0xA1 + (c - '0');
            else if (c == ' ') otName[i] = 0x00;
            else otName[i] = 0xAC;
        }
    }
    memcpy(outBoxMon + 0x14, otName, 7);

    /* Build substructs (unencrypted first) */
    u8 substructs[48];
    memset(substructs, 0, 48);

    /* Substruct 0: Growth */
    u16 species = (u16)gen2Species;
    memcpy(substructs + 0, &species, 2);
    /* held item: skip conversion for now (set to 0) */
    /* experience: copy from Gen 2 (3 bytes big-endian → 4 bytes little-endian) */
    u32 exp = ((u32)gen2Mon[0x08] << 16) | ((u32)gen2Mon[0x09] << 8) | gen2Mon[0x0A];
    memcpy(substructs + 4, &exp, 4);
    substructs[9] = 70; /* friendship */

    /* Substruct 1: Attacks */
    for (int m = 0; m < 4; m++) {
        u16 move = gen2Mon[0x02 + m];
        memcpy(substructs + 12 + m * 2, &move, 2);
        substructs[12 + 8 + m] = gen2Mon[0x17 + m]; /* PP */
    }

    /* Substruct 2: EVs (convert stat exp → EVs) */
    /* Gen 2: 2 bytes each (0-65535), 5 stats. Gen 3: 1 byte each (0-255), 6 stats, total cap 510 */
    /* Formula: EV = floor(sqrt(statExp)) capped at 252, then scale to fit 510 total */
    {
        int rawEvs[6];
        int evTotal = 0;
        for (int e = 0; e < 5; e++) {
            u16 statExp = ((u16)gen2Mon[0x0B + e*2] << 8) | gen2Mon[0x0C + e*2];
            int ev = (int)sqrtf((float)statExp);
            if (ev > 252) ev = 252;
            if (e < 4) {
                rawEvs[e] = ev; /* HP, ATK, DEF, SPD */
            } else {
                rawEvs[4] = ev / 2;     /* SPC → SPATK */
                rawEvs[5] = ev - ev/2;  /* SPC → SPDEF */
            }
        }
        /* Scale to fit 510 cap */
        for (int e = 0; e < 6; e++) evTotal += rawEvs[e];
        if (evTotal > 510) {
            for (int e = 0; e < 6; e++)
                rawEvs[e] = rawEvs[e] * 510 / evTotal;
        }
        for (int e = 0; e < 6; e++)
            substructs[24 + e] = (u8)(rawEvs[e] > 255 ? 255 : rawEvs[e]);
    }

    /* Substruct 3: Misc (IVs from DVs, met data) */
    u8 dvHi = gen2Mon[0x15];
    u8 dvLo = gen2Mon[0x16];
    u8 atkDV = (dvHi >> 4) & 0xF;
    u8 defDV = dvHi & 0xF;
    u8 spdDV = (dvLo >> 4) & 0xF;
    u8 spcDV = dvLo & 0xF;
    u8 hpDV = ((atkDV & 1) << 3) | ((defDV & 1) << 2) | ((spdDV & 1) << 1) | (spcDV & 1);

    /* Convert DVs (0-15) to IVs (0-31): IV = DV * 2 + 1 */
    u32 ivs = 0;
    ivs |= (u32)((hpDV * 2 + 1) & 0x1F);
    ivs |= (u32)((atkDV * 2 + 1) & 0x1F) << 5;
    ivs |= (u32)((defDV * 2 + 1) & 0x1F) << 10;
    ivs |= (u32)((spdDV * 2 + 1) & 0x1F) << 15;
    ivs |= (u32)((spcDV * 2 + 1) & 0x1F) << 20;
    ivs |= (u32)((spcDV * 2 + 1) & 0x1F) << 25;

    substructs[36] = gen2Mon[0x1C]; /* pokerus */
    substructs[37] = 0; /* metLocation: 0 = "traveled through time" */
    /* metLevel(7) | metGame(4) | pokeball(4) | otGender(1) */
    u16 metInfo = (gen2Mon[0x1F] & 0x7F); /* met level = current level */
    metInfo |= ((u16)0 << 7);  /* metGame = 0 */
    metInfo |= ((u16)4 << 11); /* pokeball = Poke Ball */
    memcpy(substructs + 38, &metInfo, 2);
    memcpy(substructs + 40, &ivs, 4);

    /* Shuffle substructs based on personality */
    u8 shuffled[48];
    int order = personality % 24;
    for (int t = 0; t < 4; t++) {
        int pos = sSubPos[order][t];
        memcpy(shuffled + pos * 12, substructs + t * 12, 12);
    }

    /* Calculate checksum */
    u16 checksum = 0;
    for (int i = 0; i < 24; i++) {
        u16 word;
        memcpy(&word, shuffled + i * 2, 2);
        checksum += word;
    }
    memcpy(outBoxMon + 0x1C, &checksum, 2);

    /* Encrypt and store */
    u32 key = personality ^ otId;
    u32 encrypted[12];
    memcpy(encrypted, shuffled, 48);
    for (int i = 0; i < 12; i++) encrypted[i] ^= key;
    memcpy(outBoxMon + 0x20, encrypted, 48);

    return 1;
}

/* ── Transfer confirmation state ── */
static int sShowTransferConfirm;
static char sTransferMonName[32];
static int sTransferProgress; /* 0=not active, 1-100=progress */

/* ── Safe party data builder from BoxPokemon ── */
static void BuildPartyDataFromBox(u8 *partyData, const u8 *boxRaw, const struct BtMon *mon)
{
    memset(partyData, 0, PARTYMON_SIZE);
    memcpy(partyData, boxRaw, BOXMON_SIZE);

    u8 level = mon->level > 0 ? mon->level : 5;
    /* Party extension layout after BoxPokemon (80 bytes):
       +0: u32 status
       +4: u8  level
       +5: u8  mail
       +6: u16 hp
       +8: u16 maxHP
       +10: u16 attack
       +12: u16 defense
       +14: u16 speed
       +16: u16 spAttack
       +18: u16 spDefense */

    partyData[BOXMON_SIZE + 4] = level;

    /* Set reasonable stat estimates based on level to prevent crashes.
       Real stats get recalculated by the game on next heal/battle. */
    u16 estHP = (u16)(level * 2 + 15);
    u16 estStat = (u16)(level + 8);

    /* HP */
    memcpy(&partyData[BOXMON_SIZE + 6], &estHP, 2);
    memcpy(&partyData[BOXMON_SIZE + 8], &estHP, 2);
    /* Attack, Defense, Speed, SpAtk, SpDef */
    memcpy(&partyData[BOXMON_SIZE + 10], &estStat, 2);
    memcpy(&partyData[BOXMON_SIZE + 12], &estStat, 2);
    memcpy(&partyData[BOXMON_SIZE + 14], &estStat, 2);
    memcpy(&partyData[BOXMON_SIZE + 16], &estStat, 2);
    memcpy(&partyData[BOXMON_SIZE + 18], &estStat, 2);
}

/* Forward declarations */
static void PickUp(void);
static void Place(void);
static void CancelHold(void);

/* ── Game selector bar ── */
static void DrawGameSelector(int side) {
    float ox = side * (float)SIDE_W;
    float btnW = (float)SIDE_W / NUM_GAMES;
    int activeGame = sGameIdx[side];

    for (int g = 0; g < NUM_GAMES; g++) {
        float bx = ox + g * btnW;
        int isActive = (g == activeGame);
        int hasData = sSaves[g].valid;

        /* Button background */
        if (isActive) {
            btDrawRect(bx, GAME_BAR_Y, btnW - 1, GAME_BTN_H,
                       sGameDefs[g].color[0] * 0.6f,
                       sGameDefs[g].color[1] * 0.6f,
                       sGameDefs[g].color[2] * 0.6f, 0.95f);
            btDrawRect(bx, GAME_BAR_Y + GAME_BTN_H - 2, btnW - 1, 2,
                       sGameDefs[g].color[0],
                       sGameDefs[g].color[1],
                       sGameDefs[g].color[2], 1.0f);
        } else {
            btDrawRect(bx, GAME_BAR_Y, btnW - 1, GAME_BTN_H,
                       0.12f, 0.12f, 0.16f, 0.9f);
        }

        /* Gen label */
        float textAlpha = hasData ? 1.0f : 0.35f;
        float cr = isActive ? 1.0f : sGameDefs[g].color[0] * 0.8f;
        float cg = isActive ? 1.0f : sGameDefs[g].color[1] * 0.8f;
        float cb = isActive ? 1.0f : sGameDefs[g].color[2] * 0.8f;

        const char *shortNames[] = {"FR", "LG", "RB", "SA", "GD", "SV"};
        btTextC(shortNames[g], bx + btnW / 2, GAME_BAR_Y + 18,
                sFontSmTex, sFontSmCh, cr, cg, cb);

        /* Handle click */
        if (sMouseClicked && !sHolding &&
            sMouseX >= (int)bx && sMouseX < (int)(bx + btnW) &&
            sMouseY >= GAME_BAR_Y && sMouseY < GAME_BAR_Y + GAME_BTN_H) {
            sGameIdx[side] = g;
            sCurBox[side] = sSaves[g].valid ? sSaves[g].currentBox : 0;
            sCurSlot[side] = 0;
            sCurSide = side;
        }
    }
}

/* ── Party display ── */
static void DrawPartyRow(struct BtSave *save, int side) {
    float ox = side * (float)SIDE_W;
    float py = BT_H - INFO_H - PARTY_H - 5;

    btDrawRect(ox + 15, py, SIDE_W - 30, PARTY_H, 0.08f, 0.1f, 0.15f, 0.85f);
    btText("PARTY", ox + 20, py + 14, sFontSmTex, sFontSmCh, 0.6f, 0.6f, 0.7f, 1);

    if (!save->valid || save->partyCount == 0) return;

    float cellW = 42;
    float startX = ox + 80;
    for (int p = 0; p < (int)save->partyCount; p++) {
        struct BtMon *mon = &save->party[p];
        float cx = startX + p * (cellW + 4);
        float cy = py + 4;

        btDrawRect(cx, cy, cellW, cellW, 0, 0, 0, 0.3f);

        if (mon->species > 0 && mon->species <= NUM_SPECIES_MAX) {
            GLuint tex = GetIconTex(mon->species);
            if (tex) {
                float pad = (cellW - 36) / 2.0f;
                btDrawQuad(tex, cx + pad, cy + pad, 36, 36, 1);
            }
            if (mon->isShiny) {
                btText("*", cx + cellW - 10, cy + 2, sFontSmTex, sFontSmCh,
                       1, 0.85f, 0.0f, 1);
            }
        }

        /* Cursor highlight on party */
        if (side == sCurSide && IS_PARTY_SLOT(sCurSlot[side]) &&
            PARTY_IDX(sCurSlot[side]) == p) {
            float pulse = 0.5f + 0.3f * sinf(sFrame * 0.12f);
            btDrawBorder(cx, cy, cellW, cellW, 2, 1, 0.85f, 0.2f, pulse);
        }

        /* Mouse hover + click = grab/place */
        if (sMouseX >= (int)cx && sMouseX < (int)(cx + cellW) &&
            sMouseY >= (int)cy && sMouseY < (int)(cy + cellW)) {
            btDrawBorder(cx, cy, cellW, cellW, 2, 1, 1, 1, 0.5f);

            if (sMouseClicked) {
                sCurSide = side;
                sCurSlot[side] = PARTY_SLOT_BASE + p;
                if (!sHolding) PickUp();
                else Place();
                sMouseClicked = 0;
            }
        }
    }
}

/* ── Drawing ── */
static void DrawBoxPanel(struct BtSave *save, int side, int boxIdx) {
    float ox = side * SIDE_W;
    float bx = ox + 15;
    float by = GAME_BTN_H + 40;
    float bw = SIDE_W - 30;
    float bh = BT_H - INFO_H - PARTY_H - by - 20;

    /* Box background */
    u8 wp = save->boxes[boxIdx].wallpaper;
    const float *wc = sWallpaperColors[wp < 16 ? wp : 15];
    btDrawGradV(bx, by, bw, bh, wc[0]*0.7f, wc[1]*0.7f, wc[2]*0.7f, 0.9f,
                                  wc[0]*0.4f, wc[1]*0.4f, wc[2]*0.4f, 0.9f);
    btDrawBorder(bx-2, by-2, bw+4, bh+4, 2, 0.5f, 0.55f, 0.65f, 0.8f);

    /* Game header */
    int gameId = sGameIdx[side];
    const char *gameName = sGameDefs[gameId].name;
    float hdrR = sGameDefs[gameId].color[0];
    float hdrG = sGameDefs[gameId].color[1];
    float hdrB = sGameDefs[gameId].color[2];
    float hdrY = GAME_BTN_H + GAME_BAR_Y + 4;
    btDrawRect(ox, hdrY, (float)SIDE_W, 30, 0.06f, 0.06f, 0.12f, 0.95f);
    if (save->playerName[0]) {
        char hdr[64];
        snprintf(hdr, sizeof(hdr), "%s - %s", gameName, save->playerName);
        btTextC(hdr, ox + SIDE_W/2.0f, hdrY + 20, sFontSmTex, sFontSmCh, hdrR, hdrG, hdrB);
    } else {
        btTextC(gameName, ox + SIDE_W/2.0f, hdrY + 20, sFontSmTex, sFontSmCh, hdrR, hdrG, hdrB);
    }

    /* Box name and navigation */
    {
        char boxLabel[32];
        snprintf(boxLabel, sizeof(boxLabel), "< %s >", save->boxes[boxIdx].name);
        btTextC(boxLabel, bx + bw/2, by + 18, sFontTex, sFontCh, 1, 1, 1);
    }

    /* Pokemon grid */
    float gridW = BOX_COLS * CELL_SZ;
    float gridH = BOX_ROWS * CELL_SZ;
    float gx = bx + (bw - gridW) / 2;
    float gy = by + 30;

    for (int row = 0; row < BOX_ROWS; row++) {
        for (int col = 0; col < BOX_COLS; col++) {
            int slot = row * BOX_COLS + col;
            struct BtMon *mon = &save->boxes[boxIdx].mons[slot];
            float cx = gx + col * CELL_SZ;
            float cy = gy + row * CELL_SZ;

            /* Cell background */
            btDrawRect(cx+1, cy+1, CELL_SZ-2, CELL_SZ-2, 0, 0, 0, 0.25f);

            /* Draw icon if occupied */
            if (mon->species > 0 && mon->species <= NUM_SPECIES_MAX) {
                /* Skip if this is the held pokemon's origin slot */
                if (sHolding && sHoldSide == side && sHoldBox == boxIdx && sHoldSlot == slot)
                    continue;

                GLuint tex = GetIconTex(mon->species);
                if (tex) {
                    float pad = (CELL_SZ - ICON_SZ) / 2.0f;
                    btDrawQuad(tex, cx + pad, cy + pad, ICON_SZ, ICON_SZ, 1);
                }

                if (mon->isShiny) {
                    btText("*", cx + CELL_SZ - 12, cy + 2, sFontTex, sFontCh,
                           1, 0.85f, 0.0f, 1);
                }
                if (mon->isEgg) {
                    btText("EGG", cx + 2, cy + CELL_SZ - 4, sFontSmTex, sFontSmCh,
                           1, 0.9f, 0.5f, 0.8f);
                }
            }

            /* Mouse hover detection */
            if (sMouseX >= (int)cx && sMouseX < (int)(cx + CELL_SZ) &&
                sMouseY >= (int)cy && sMouseY < (int)(cy + CELL_SZ)) {
                btDrawBorder(cx, cy, CELL_SZ, CELL_SZ, 2, 1, 1, 1, 0.5f);
                sHoverSide = side;
                sHoverBox = boxIdx;
                sHoverSlot = slot;
                sHoverValid = 1;

                /* Left click = grab/place (like Z) */
                if (sMouseClicked) {
                    sCurSide = side;
                    sCurSlot[side] = slot;
                    sCurBox[side] = boxIdx;
                    if (!sHolding) PickUp();
                    else Place();
                    sMouseClicked = 0;
                }
            }

            /* Cursor highlight */
            if (side == sCurSide && slot == sCurSlot[side] && boxIdx == sCurBox[side]) {
                float pulse = 0.5f + 0.3f * sinf(sFrame * 0.12f);
                btDrawBorder(cx, cy, CELL_SZ, CELL_SZ, 2,
                             1, 0.85f, 0.2f, pulse);
            }
        }
    }

    /* Box count display */
    {
        int count = 0;
        for (int i = 0; i < IN_BOX_COUNT; i++)
            if (save->boxes[boxIdx].mons[i].species > 0) count++;
        char countStr[16];
        snprintf(countStr, sizeof(countStr), "%d/30", count);
        btText(countStr, bx + bw - 50, gy + gridH + 14, sFontSmTex, sFontSmCh,
               0.7f, 0.7f, 0.75f, 1);
    }
}

static void DrawInfoPanel(void) {
    float py = BT_H - INFO_H;
    btDrawRect(0, py, BT_W, INFO_H, 0.06f, 0.08f, 0.14f, 0.95f);
    btDrawRect(0, py, BT_W, 2, 0.4f, 0.45f, 0.55f, 0.6f);

    struct BtSave *save = &sSaves[sGameIdx[sCurSide]];
    if (!save->valid) {
        btTextC("No save data", BT_W/2.0f, py + 45, sFontTex, sFontCh, 0.5f, 0.5f, 0.55f);
        return;
    }

    int slot = sCurSlot[sCurSide];

    /* Show held pokemon info if holding, otherwise show cursor target */
    struct BtMon *mon;
    if (sHolding) {
        mon = &sHeldMon;
    } else if (IS_PARTY_SLOT(slot)) {
        int p = PARTY_IDX(slot);
        mon = (p < save->partyCount) ? &save->party[p] : NULL;
    } else {
        int box = sCurBox[sCurSide];
        mon = &save->boxes[box].mons[slot];
    }
    if (!mon) {
        btTextC("Empty slot", BT_W/2.0f, py + 45, sFontTex, sFontCh, 0.5f, 0.5f, 0.55f);
        return;
    }

    if (mon->species == 0) {
        btTextC(sHolding ? "Holding: (empty)" : "Empty slot",
                BT_W/2.0f, py + 45, sFontTex, sFontCh, 0.5f, 0.5f, 0.55f);
    } else {
        /* Icon */
        GLuint tex = GetIconTex(mon->species);
        if (tex) btDrawQuad(tex, 20, py + 10, 64, 64, 1);

        /* Species name and nickname */
        float tx = 100;
        char info[128];
        if (mon->isEgg) {
            snprintf(info, sizeof(info), "EGG (%s)", mon->speciesName);
        } else if (strcmp(mon->nickname, mon->speciesName) != 0 && mon->nickname[0]) {
            snprintf(info, sizeof(info), "\"%s\" (%s)", mon->nickname, mon->speciesName);
        } else {
            snprintf(info, sizeof(info), "%s", mon->speciesName);
        }
        btText(info, tx, py + 30, sFontTex, sFontCh, 1, 1, 1, 1);

        if (!mon->isEgg && mon->level > 0) {
            char lvl[16];
            snprintf(lvl, sizeof(lvl), "Lv.%d", mon->level);
            btText(lvl, tx, py + 52, sFontTex, sFontCh, 0.8f, 0.9f, 1, 1);
        }

        if (sHolding) {
            btText("[HOLDING]", tx + 200, py + 30, sFontSmTex, sFontSmCh,
                   1, 0.8f, 0.2f, 1);
        }
    }

    /* Controls help */
    btText("[Z] Grab/Place  [X] Cancel  [Tab] Switch side  [Q/E] Box  [1-6] Game  [Esc] Exit",
           15, BT_H - 18, sFontSmTex, sFontSmCh, 0.45f, 0.45f, 0.5f, 1);

    if (sUnsaved) {
        btText("* UNSAVED CHANGES *", BT_W - 200, BT_H - 18,
               sFontSmTex, sFontSmCh, 1, 0.4f, 0.3f, 1);
    }
}

static void DrawHoverTooltip(void) {
    if (!sHoverValid) return;

    struct BtSave *save = &sSaves[sGameIdx[sHoverSide]];
    if (!save->valid) return;

    struct BtMon *mon = &save->boxes[sHoverBox].mons[sHoverSlot];
    if (mon->species == 0 || mon->species > NUM_SPECIES_MAX) return;

    float tx = sMouseX + 16.0f;
    float ty = sMouseY + 4.0f;
    float tw = 180, th = 70;

    /* Keep tooltip on screen */
    if (tx + tw > BT_W) tx = sMouseX - tw - 4;
    if (ty + th > BT_H) ty = sMouseY - th - 4;

    /* Background */
    btDrawRect(tx, ty, tw, th, 0.05f, 0.05f, 0.1f, 0.92f);
    btDrawBorder(tx, ty, tw, th, 1, 0.5f, 0.5f, 0.6f, 0.8f);

    /* Icon */
    GLuint tex = GetIconTex(mon->species);
    if (tex) btDrawQuad(tex, tx + 4, ty + 4, 40, 40, 1);

    /* Species & nickname */
    float ix = tx + 48;
    if (mon->isEgg) {
        btText("Egg", ix, ty + 18, sFontTex, sFontCh, 1, 0.9f, 0.5f, 1);
    } else {
        btText(mon->speciesName, ix, ty + 18, sFontTex, sFontCh, 1, 1, 1, 1);
        if (mon->nickname[0] && strcmp(mon->nickname, mon->speciesName) != 0) {
            char nn[32];
            snprintf(nn, sizeof(nn), "\"%s\"", mon->nickname);
            btText(nn, ix, ty + 34, sFontSmTex, sFontSmCh, 0.7f, 0.8f, 0.9f, 1);
        }
    }

    /* Level + shiny */
    if (!mon->isEgg && mon->level > 0) {
        char lvl[32];
        snprintf(lvl, sizeof(lvl), "Lv.%d%s", mon->level, mon->isShiny ? "  * SHINY *" : "");
        btText(lvl, ix, ty + 50, sFontSmTex, sFontSmCh,
               mon->isShiny ? 1.0f : 0.8f,
               mon->isShiny ? 0.85f : 0.9f,
               mon->isShiny ? 0.0f : 1.0f, 1);
    }
}

static void DrawHeldIcon(int mx, int my) {
    if (!sHolding || sHeldMon.species == 0) return;
    GLuint tex = GetIconTex(sHeldMon.species);
    if (tex) btDrawQuad(tex, mx - ICON_SZ/2.0f, my - ICON_SZ/2.0f, ICON_SZ, ICON_SZ, 0.85f);
}

/* ── Transfer logic ── */
static void PickUp(void) {
    struct BtSave *save = &sSaves[sGameIdx[sCurSide]];
    if (!save->valid) return;
    int slot = sCurSlot[sCurSide];

    if (IS_PARTY_SLOT(slot)) {
        int p = PARTY_IDX(slot);
        if (p >= save->partyCount) return;
        struct BtMon *mon = &save->party[p];
        if (mon->species == 0) return;
        if (save->partyCount <= 1) return;

        if (sGameDefs[save->gameIdx].gen == 2) {
            memcpy(sHeldRaw, save->gen2Raw[p], GEN2_PARTYMON_SIZE);
        } else {
            memcpy(sHeldRaw, &save->saveBlock1[save->partyOff + p * PARTYMON_SIZE], PARTYMON_SIZE);
        }
        sHeldMon = *mon;
        sHeldFromParty = 1;
        sHoldSide = sCurSide;
        sHoldBox = -1;
        sHoldSlot = slot;
        sHoldGame = sGameIdx[sCurSide];
        sHolding = 1;

        /* Remove from party: shift remaining members down */
        for (int i = p; i < save->partyCount - 1; i++) {
            memcpy(&save->saveBlock1[save->partyOff + i * PARTYMON_SIZE],
                   &save->saveBlock1[save->partyOff + (i+1) * PARTYMON_SIZE], PARTYMON_SIZE);
            save->party[i] = save->party[i+1];
        }
        save->partyCount--;
        save->saveBlock1[save->partyCountOff] = save->partyCount;
        memset(&save->saveBlock1[save->partyOff + save->partyCount * PARTYMON_SIZE], 0, PARTYMON_SIZE);
        memset(&save->party[save->partyCount], 0, sizeof(struct BtMon));
    } else {
        int box = sCurBox[sCurSide];
        struct BtMon *mon = &save->boxes[box].mons[slot];
        if (mon->species == 0) return;

        ReadBoxMonFromStorage(save, box, slot, sHeldRaw);
        sHeldMon = *mon;
        sHeldFromParty = 0;
        sHoldSide = sCurSide;
        sHoldBox = box;
        sHoldSlot = slot;
        sHoldGame = sGameIdx[sCurSide];
        sHolding = 1;

        u8 empty[BOXMON_SIZE];
        memset(empty, 0, sizeof(empty));
        WriteBoxMonToStorage(save, box, slot, empty);
        ReparseBoxMon(save, box, slot);
    }
}

static void Place(void) {
    int destGame = sGameIdx[sCurSide];
    struct BtSave *dest = &sSaves[destGame];
    if (!dest->valid) return;

    /* Block Gen 3 → Gen 2 (backward transfer not allowed) */
    if (sGameDefs[sHoldGame].gen == 3 && sGameDefs[destGame].gen == 2)
        return;

    /* Gen 2 → Gen 3: show confirmation prompt */
    if (sGameDefs[sHoldGame].gen == 2 && sGameDefs[destGame].gen == 3) {
        snprintf(sTransferMonName, sizeof(sTransferMonName), "%s", sHeldMon.speciesName);
        sShowTransferConfirm = 1;
        return; /* Place will be called again after confirmation */
    }

    int slot = sCurSlot[sCurSide];

    if (IS_PARTY_SLOT(slot)) {
        int p = PARTY_IDX(slot);
        if (!sHeldFromParty) {
            /* Box → Party: only into existing slot (swap) or append if < 6 */
            if (p < dest->partyCount) {
                /* Swap with existing party member */
                u8 tempRaw[PARTYMON_SIZE];
                struct BtMon tempMon;
                memcpy(tempRaw, &dest->saveBlock1[dest->partyOff + p * PARTYMON_SIZE], PARTYMON_SIZE);
                tempMon = dest->party[p];

                /* Write held mon into party */
                u8 partyData[PARTYMON_SIZE];
                if (sHeldFromParty) {
                    memcpy(partyData, sHeldRaw, PARTYMON_SIZE);
                } else {
                    BuildPartyDataFromBox(partyData, sHeldRaw, &sHeldMon);
                }
                memcpy(&dest->saveBlock1[dest->partyOff + p * PARTYMON_SIZE], partyData, PARTYMON_SIZE);
                dest->party[p] = sHeldMon;

                /* Pick up the party mon that was there */
                memcpy(sHeldRaw, tempRaw, PARTYMON_SIZE);
                sHeldMon = tempMon;
                sHeldFromParty = 1;
                sHoldSide = sCurSide;
                sHoldSlot = slot;
                sHoldGame = destGame;
            } else if (dest->partyCount < 6) {
                /* Append to party */
                u8 partyData[PARTYMON_SIZE];
                if (sHeldFromParty) {
                    memcpy(partyData, sHeldRaw, PARTYMON_SIZE);
                } else {
                    BuildPartyDataFromBox(partyData, sHeldRaw, &sHeldMon);
                }
                memcpy(&dest->saveBlock1[dest->partyOff + dest->partyCount * PARTYMON_SIZE], partyData, PARTYMON_SIZE);
                dest->party[dest->partyCount] = sHeldMon;
                dest->partyCount++;
                dest->saveBlock1[dest->partyCountOff] = dest->partyCount;
                sHolding = 0;
            }
        } else {
            /* Party → Party */
            if (p < dest->partyCount) {
                /* Swap */
                u8 tempRaw[PARTYMON_SIZE];
                struct BtMon tempMon;
                memcpy(tempRaw, &dest->saveBlock1[dest->partyOff + p * PARTYMON_SIZE], PARTYMON_SIZE);
                tempMon = dest->party[p];

                memcpy(&dest->saveBlock1[dest->partyOff + p * PARTYMON_SIZE], sHeldRaw, PARTYMON_SIZE);
                dest->party[p] = sHeldMon;

                memcpy(sHeldRaw, tempRaw, PARTYMON_SIZE);
                sHeldMon = tempMon;
                sHoldSide = sCurSide;
                sHoldSlot = slot;
                sHoldGame = destGame;
            } else if (dest->partyCount < 6) {
                /* Append */
                memcpy(&dest->saveBlock1[dest->partyOff + dest->partyCount * PARTYMON_SIZE], sHeldRaw, PARTYMON_SIZE);
                dest->party[dest->partyCount] = sHeldMon;
                dest->partyCount++;
                dest->saveBlock1[dest->partyCountOff] = dest->partyCount;
                sHolding = 0;
            }
        }
    } else {
        int box = sCurBox[sCurSide];
        struct BtMon *existing = &dest->boxes[box].mons[slot];

        if (existing->species > 0) {
            u8 tempRaw[BOXMON_SIZE];
            struct BtMon tempMon;
            ReadBoxMonFromStorage(dest, box, slot, tempRaw);
            tempMon = *existing;

            WriteBoxMonToStorage(dest, box, slot, sHeldRaw);
            ReparseBoxMon(dest, box, slot);

            memcpy(sHeldRaw, tempRaw, BOXMON_SIZE);
            sHeldMon = tempMon;
            sHeldFromParty = 0;
            sHoldSide = sCurSide;
            sHoldBox = box;
            sHoldSlot = slot;
        } else {
            WriteBoxMonToStorage(dest, box, slot, sHeldRaw);
            ReparseBoxMon(dest, box, slot);
            sHolding = 0;
        }
    }
    sUnsaved = 1;
}

static void PerformGen2ToGen3Transfer(void) {
    int destGame = sGameIdx[sCurSide];
    struct BtSave *dest = &sSaves[destGame];
    if (!dest->valid) return;

    int slot = sCurSlot[sCurSide];
    if (IS_PARTY_SLOT(slot)) return; /* only to boxes for cross-gen */

    int box = sCurBox[sCurSide];

    /* Convert Gen 2 mon to Gen 3 BoxPokemon */
    u8 gen3Box[BOXMON_SIZE];
    /* Find the Gen 2 raw data — use the held raw bytes */
    /* sHeldRaw contains the Gen 2 party/box mon raw data */
    if (!ConvertGen2ToGen3(sHeldRaw, sHeldMon.species, sHeldMon.nickname,
                           (u16)sHeldMon.otId, sSaves[sHoldGame].playerName, gen3Box)) {
        sHolding = 0;
        return;
    }

    /* Write to destination box */
    struct BtMon *existing = &dest->boxes[box].mons[slot];
    if (existing->species > 0) return; /* don't overwrite — only empty slots */

    WriteBoxMonToStorage(dest, box, slot, gen3Box);
    ReparseBoxMon(dest, box, slot);
    sHolding = 0;
    sUnsaved = 1;
    sTransferProgress = 0;

    /* Auto-save destination so the Pokemon persists in the box.
       User can then withdraw in-game for proper stat calculation. */
    WriteSave(dest, sGameDefs[destGame].savePath);

    /* Also save the Gen 2 source (party member was removed) */
    if (sGameDefs[sHoldGame].gen == 2)
        WriteGen2Save(&sSaves[sHoldGame], sGameDefs[sHoldGame].savePath);

    printf("box_transfer: Transfer complete — saves auto-written\n");
}

static void CancelHold(void) {
    if (!sHolding) return;
    struct BtSave *src = &sSaves[sHoldGame];

    if (sHeldFromParty) {
        /* Restore to party: append at end */
        if (src->partyCount < 6) {
            memcpy(&src->saveBlock1[src->partyOff + src->partyCount * PARTYMON_SIZE],
                   sHeldRaw, PARTYMON_SIZE);
            src->party[src->partyCount] = sHeldMon;
            src->partyCount++;
            src->saveBlock1[src->partyCountOff] = src->partyCount;
        }
    } else {
        WriteBoxMonToStorage(src, sHoldBox, sHoldSlot, sHeldRaw);
        ReparseBoxMon(src, sHoldBox, sHoldSlot);
    }
    sHolding = 0;
}

/* ── Main entry ── */
void RunBoxTransferUI(SDL_Window *win, SDL_GLContext glCtx) {
    /* Initialize icon cache */
    memset(sIconTex, 0, sizeof(sIconTex));
    sFrame = 0;
    sHolding = 0;
    sUnsaved = 0;
    sCurSide = 0;
    sCurBox[0] = sCurBox[1] = 0;
    sCurSlot[0] = sCurSlot[1] = 0;

    /* Bake fonts */
    sFontTex = btBakeFont("C:\\Windows\\Fonts\\consola.ttf", 20, sFontCh);
    sFontSmTex = btBakeFont("C:\\Windows\\Fonts\\consola.ttf", 13, sFontSmCh);

    /* Load all saves */
    for (int i = 0; i < NUM_GAMES; i++) {
        sSaves[i].gameIdx = i;
        sSaves[i].partyCountOff = sGameDefs[i].partyCountOff;
        sSaves[i].partyOff = sGameDefs[i].partyOff;
        if (sGameDefs[i].gen == 3) {
            if (ParseSave(&sSaves[i], sGameDefs[i].savePath))
                printf("box_transfer: %s save loaded (%s, box %d)\n",
                       sGameDefs[i].name, sSaves[i].playerName, sSaves[i].currentBox);
        } else if (sGameDefs[i].gen == 2) {
            if (ParseGen2Save(&sSaves[i], sGameDefs[i].savePath))
                printf("box_transfer: %s save loaded (%s)\n",
                       sGameDefs[i].name, sSaves[i].playerName);
        }
    }

    /* Debug */
    {
        FILE *dbg = fopen("box_debug.txt", "w");
        if (dbg) {
            for (int g = 0; g < NUM_GAMES; g++) {
                struct BtSave *sv = &sSaves[g];
                fprintf(dbg, "=== %s (gen %d) ===\n", sGameDefs[g].name, sGameDefs[g].gen);
                fprintf(dbg, "valid=%d partyCountOff=0x%X partyOff=0x%X\n",
                        sv->valid, sv->partyCountOff, sv->partyOff);

                if (sGameDefs[g].gen == 3 && sv->valid) {
                    fprintf(dbg, "SB1 at partyCountOff:");
                    for (int i = 0; i < 20; i++)
                        fprintf(dbg, " %02X", sv->saveBlock1[sv->partyCountOff + i]);
                    fprintf(dbg, "\n");
                }

                fprintf(dbg, "player='%s' partyCount=%d\n", sv->playerName, sv->partyCount);
                for (int p = 0; p < sv->partyCount; p++)
                    fprintf(dbg, "  party[%d]: species=%d lvl=%d name='%s'%s\n",
                            p, sv->party[p].species, sv->party[p].level,
                            sv->party[p].speciesName, sv->party[p].isShiny ? " *SHINY*" : "");
                for (int b = 0; b < TOTAL_BOXES; b++) {
                    int count = 0;
                    for (int s = 0; s < IN_BOX_COUNT; s++)
                        if (sv->boxes[b].mons[s].species > 0) count++;
                    if (count > 0)
                        fprintf(dbg, "  box[%d] '%s': %d pokemon\n", b, sv->boxes[b].name, count);
                }

                if (sGameDefs[g].gen == 2 && sv->valid) {
                    /* Dump raw Gen 2 SRAM at key offsets */
                    FILE *sf = fopen(sGameDefs[g].savePath, "rb");
                    if (sf) {
                        u8 peek[32];
                        fseek(sf, GEN2_CHECK1, SEEK_SET);
                        fread(peek, 1, 1, sf);
                        fprintf(dbg, "  sram[0x%X] sentinel = 0x%02X (expect 0x63)\n", GEN2_CHECK1, peek[0]);
                        fseek(sf, GEN2_PLAYER_NAME, SEEK_SET);
                        fread(peek, 1, 16, sf);
                        fprintf(dbg, "  sram[0x%X] name hex:", GEN2_PLAYER_NAME);
                        for (int i = 0; i < 16; i++) fprintf(dbg, " %02X", peek[i]);
                        fprintf(dbg, "\n");
                        fseek(sf, GEN2_PARTY_COUNT, SEEK_SET);
                        fread(peek, 1, 16, sf);
                        fprintf(dbg, "  sram[0x%X] party hex:", GEN2_PARTY_COUNT);
                        for (int i = 0; i < 16; i++) fprintf(dbg, " %02X", peek[i]);
                        fprintf(dbg, "\n");
                        fseek(sf, 0, SEEK_END);
                        fprintf(dbg, "  file size: %ld\n", ftell(sf));
                        fclose(sf);
                    }
                }
                fprintf(dbg, "\n");
            }
            fclose(dbg);
        }
    }

    /* Pick first two valid saves for each side */
    sGameIdx[0] = GAME_FIRERED;
    sGameIdx[1] = GAME_LEAFGREEN;
    if (sSaves[sGameIdx[0]].valid) sCurBox[0] = sSaves[sGameIdx[0]].currentBox;
    if (sSaves[sGameIdx[1]].valid) sCurBox[1] = sSaves[sGameIdx[1]].currentBox;

    /* Resize window for box transfer */
    SDL_SetWindowSize(win, BT_W, BT_H);
    SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowTitle(win, "PokeRecomp - Pokemon Box Transfer");

    int running = 1;
    int mouseX = 0, mouseY = 0;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = 0; break; }
            if (ev.type == SDL_MOUSEMOTION) { mouseX = ev.motion.x; mouseY = ev.motion.y; }
            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                sMouseClicked = 1;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_RIGHT) {
                CancelHold();
            }

            if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;

                if (k == SDLK_ESCAPE) {
                    if (sHolding) {
                        CancelHold();
                    } else {
                        if (sUnsaved) {
                            for (int i = 0; i < NUM_GAMES; i++) {
                                if (sSaves[i].valid) {
                                    if (sGameDefs[i].gen == 3)
                                        WriteSave(&sSaves[i], sGameDefs[i].savePath);
                                    else if (sGameDefs[i].gen == 2)
                                        WriteGen2Save(&sSaves[i], sGameDefs[i].savePath);
                                }
                            }
                            printf("box_transfer: all saves written\n");
                        }
                        running = 0;
                    }
                    break;
                }

                /* Navigation */
                {
                    int cs = sCurSlot[sCurSide];
                    struct BtSave *sv = &sSaves[sGameIdx[sCurSide]];
                    int maxParty = sv->valid ? sv->partyCount : 0;

                    if (k == SDLK_LEFT || k == SDLK_a) {
                        if (IS_PARTY_SLOT(cs)) {
                            int p = PARTY_IDX(cs);
                            if (p > 0) sCurSlot[sCurSide] = PARTY_SLOT_BASE + p - 1;
                        } else {
                            int col = cs % BOX_COLS;
                            int row = cs / BOX_COLS;
                            col = (col - 1 + BOX_COLS) % BOX_COLS;
                            sCurSlot[sCurSide] = row * BOX_COLS + col;
                        }
                    }
                    if (k == SDLK_RIGHT || k == SDLK_d) {
                        if (IS_PARTY_SLOT(cs)) {
                            int p = PARTY_IDX(cs);
                            if (p < maxParty - 1) sCurSlot[sCurSide] = PARTY_SLOT_BASE + p + 1;
                        } else {
                            int col = cs % BOX_COLS;
                            int row = cs / BOX_COLS;
                            col = (col + 1) % BOX_COLS;
                            sCurSlot[sCurSide] = row * BOX_COLS + col;
                        }
                    }
                    if (k == SDLK_UP || k == SDLK_w) {
                        if (IS_PARTY_SLOT(cs)) {
                            int col = PARTY_IDX(cs);
                            if (col >= BOX_COLS) col = BOX_COLS - 1;
                            sCurSlot[sCurSide] = (BOX_ROWS - 1) * BOX_COLS + col;
                        } else {
                            int col = cs % BOX_COLS;
                            int row = cs / BOX_COLS;
                            if (row == 0) row = BOX_ROWS - 1;
                            else row--;
                            sCurSlot[sCurSide] = row * BOX_COLS + col;
                        }
                    }
                    if (k == SDLK_DOWN || k == SDLK_s) {
                        if (!IS_PARTY_SLOT(cs)) {
                            int col = cs % BOX_COLS;
                            int row = cs / BOX_COLS;
                            if (row == BOX_ROWS - 1 && maxParty > 0) {
                                int p = col < maxParty ? col : maxParty - 1;
                                sCurSlot[sCurSide] = PARTY_SLOT_BASE + p;
                            } else {
                                row = (row + 1) % BOX_ROWS;
                                sCurSlot[sCurSide] = row * BOX_COLS + col;
                            }
                        }
                    }
                }

                /* Switch side */
                if (k == SDLK_TAB) {
                    sCurSide = 1 - sCurSide;
                }

                /* Switch box */
                if (k == SDLK_q || k == SDLK_PAGEUP) {
                    sCurBox[sCurSide] = (sCurBox[sCurSide] - 1 + TOTAL_BOXES) % TOTAL_BOXES;
                }
                if (k == SDLK_e || k == SDLK_PAGEDOWN) {
                    sCurBox[sCurSide] = (sCurBox[sCurSide] + 1) % TOTAL_BOXES;
                }

                /* Grab / Place */
                if (k == SDLK_z || k == SDLK_RETURN) {
                    if (!sHolding) PickUp();
                    else Place();
                }

                /* Cancel hold */
                if (k == SDLK_x || k == SDLK_BACKSPACE) {
                    CancelHold();
                }

                /* Switch game on active side (1-6) */
                if (k >= SDLK_1 && k <= SDLK_6) {
                    int newGame = k - SDLK_1;
                    if (newGame < NUM_GAMES && !sHolding) {
                        sGameIdx[sCurSide] = newGame;
                        sCurBox[sCurSide] = sSaves[newGame].valid ? sSaves[newGame].currentBox : 0;
                        sCurSlot[sCurSide] = 0;
                    }
                }

                /* Manual save */
                if (k == SDLK_F5) {
                    for (int i = 0; i < NUM_GAMES; i++) {
                        if (sSaves[i].valid) {
                            if (sGameDefs[i].gen == 3)
                                WriteSave(&sSaves[i], sGameDefs[i].savePath);
                            else if (sGameDefs[i].gen == 2)
                                WriteGen2Save(&sSaves[i], sGameDefs[i].savePath);
                        }
                    }
                    sUnsaved = 0;
                    printf("box_transfer: all saves written (manual)\n");
                }
            }
        }

        sFrame++;
        sMouseX = mouseX;
        sMouseY = mouseY;

        /* Render */
        {
            int ww, wh;
            SDL_GetWindowSize(win, &ww, &wh);
            float sx = (float)ww / BT_W, sy = (float)wh / BT_H;
            float sc = sx < sy ? sx : sy;
            int vw = (int)(BT_W * sc), vh = (int)(BT_H * sc);
            glViewport((ww - vw) / 2, (wh - vh) / 2, vw, vh);
        }
        glClearColor(0.04f, 0.04f, 0.08f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        glOrtho(0, BT_W, BT_H, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sHoverValid = 0;

        /* Game selector buttons for both sides */
        DrawGameSelector(0);
        DrawGameSelector(1);

        /* Divider */
        btDrawRect(SIDE_W - 1, 0, 2, BT_H - INFO_H, 0.3f, 0.35f, 0.45f, 0.6f);

        /* Side indicator */
        {
            float ix = sCurSide * SIDE_W;
            float indicY = GAME_BTN_H + GAME_BAR_Y + 34;
            btDrawRect(ix, indicY, (float)SIDE_W, 2, 1, 0.8f, 0.2f, 0.6f);
        }

        /* Draw box panels for both sides */
        for (int side = 0; side < 2; side++) {
            int gid = sGameIdx[side];
            struct BtSave *sv = &sSaves[gid];
            float ox = side * (float)SIDE_W;

            if (sv->valid) {
                DrawBoxPanel(sv, side, sCurBox[side]);
            } else {
                btDrawRect(ox + 15, 50, SIDE_W - 30, BT_H - INFO_H - 70,
                           0.08f, 0.08f, 0.12f, 0.8f);
                char noSave[64];
                snprintf(noSave, sizeof(noSave), "No %s save", sGameDefs[gid].name);
                btTextC(noSave, ox + SIDE_W / 2.0f, BT_H / 2.0f - 50,
                        sFontTex, sFontCh,
                        sGameDefs[gid].color[0] * 0.6f,
                        sGameDefs[gid].color[1] * 0.6f,
                        sGameDefs[gid].color[2] * 0.6f);
                char savePath[64];
                snprintf(savePath, sizeof(savePath), "(%s)", sGameDefs[gid].savePath);
                btTextC(savePath, ox + SIDE_W / 2.0f, BT_H / 2.0f - 25,
                        sFontSmTex, sFontSmCh, 0.4f, 0.4f, 0.45f);
            }
        }

        /* Party rows */
        DrawPartyRow(&sSaves[sGameIdx[0]], 0);
        DrawPartyRow(&sSaves[sGameIdx[1]], 1);

        DrawInfoPanel();
        DrawHoverTooltip();

        /* Transfer confirmation dialog */
        if (sShowTransferConfirm) {
            float dw = 360, dh = 120;
            float dx = (BT_W - dw) / 2, dy = (BT_H - dh) / 2;
            btDrawRect(0, 0, BT_W, BT_H, 0, 0, 0, 0.5f);
            btDrawRect(dx, dy, dw, dh, 0.1f, 0.1f, 0.18f, 0.98f);
            btDrawBorder(dx, dy, dw, dh, 2, 0.6f, 0.5f, 0.2f, 1);

            char msg[128];
            snprintf(msg, sizeof(msg), "Transfer %s to Gen 3?", sTransferMonName);
            btTextC(msg, BT_W/2.0f, dy + 30, sFontTex, sFontCh, 1, 0.9f, 0.5f);
            btTextC("They can only move forward!", BT_W/2.0f, dy + 55, sFontSmTex, sFontSmCh, 0.7f, 0.6f, 0.4f);

            /* Yes / No buttons */
            float btnW2 = 80, btnH2 = 30;
            float yesX = BT_W/2.0f - btnW2 - 10, noX = BT_W/2.0f + 10;
            float btnY = dy + dh - 42;
            btDrawRect(yesX, btnY, btnW2, btnH2, 0.2f, 0.5f, 0.2f, 0.9f);
            btTextC("YES", yesX + btnW2/2, btnY + 20, sFontTex, sFontCh, 1, 1, 1);
            btDrawRect(noX, btnY, btnW2, btnH2, 0.5f, 0.2f, 0.2f, 0.9f);
            btTextC("NO", noX + btnW2/2, btnY + 20, sFontTex, sFontCh, 1, 1, 1);

            if (sMouseClicked) {
                if (sMouseX >= (int)yesX && sMouseX < (int)(yesX+btnW2) &&
                    sMouseY >= (int)btnY && sMouseY < (int)(btnY+btnH2)) {
                    sShowTransferConfirm = 0;
                    sTransferProgress = 1;
                    sMouseClicked = 0;
                }
                if (sMouseX >= (int)noX && sMouseX < (int)(noX+btnW2) &&
                    sMouseY >= (int)btnY && sMouseY < (int)(btnY+btnH2)) {
                    sShowTransferConfirm = 0;
                    CancelHold();
                    sMouseClicked = 0;
                }
            }
        }

        /* Transfer progress bar */
        if (sTransferProgress > 0 && sTransferProgress <= 100) {
            float dw = 300, dh = 60;
            float dx = (BT_W - dw) / 2, dy = (BT_H - dh) / 2;
            btDrawRect(0, 0, BT_W, BT_H, 0, 0, 0, 0.5f);
            btDrawRect(dx, dy, dw, dh, 0.1f, 0.1f, 0.18f, 0.98f);
            btDrawBorder(dx, dy, dw, dh, 2, 0.4f, 0.5f, 0.6f, 1);
            btTextC("Translating stats...", BT_W/2.0f, dy + 22, sFontTex, sFontCh, 0.8f, 0.9f, 1);

            float barX = dx + 20, barY = dy + 35, barW = dw - 40, barH = 14;
            btDrawRect(barX, barY, barW, barH, 0.05f, 0.05f, 0.1f, 1);
            float fill = (float)sTransferProgress / 100.0f;
            btDrawRect(barX+1, barY+1, (barW-2)*fill, barH-2, 0.3f, 0.7f, 0.3f, 1);

            sTransferProgress += 4;
            if (sTransferProgress > 100) {
                PerformGen2ToGen3Transfer();
                sTransferProgress = 0;
            }
        }

        if (sHolding)
            DrawHeldIcon(mouseX, mouseY);

        sMouseClicked = 0;

        SDL_GL_SwapWindow(win);
        SDL_Delay(16);
    }

    /* Cleanup */
    FreeIconCache();
    if (sFontTex) { glDeleteTextures(1, &sFontTex); sFontTex = 0; }
    if (sFontSmTex) { glDeleteTextures(1, &sFontSmTex); sFontSmTex = 0; }

    /* Restore launcher window */
    SDL_SetWindowSize(win, 960, 540);
    SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowTitle(win, "PokeRecomp");
}

#endif /* PLATFORM_SDL2 */
