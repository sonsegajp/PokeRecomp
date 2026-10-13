// PC port: dump the game's own Pokémon rules as JSON (--dump-legality FILE)
// for the online servers' legality checks (server/legality.py). Everything
// comes from the tables the game itself uses, so the server and the game can
// never disagree about what is legal.
#include "global.h"
#ifdef PLATFORM_PC
#include <stdio.h>
#include "pokemon.h"
#include "party_menu.h"
#include "overworld.h"
#include "region_map.h"
#include "wild_encounter.h"
#include "battle_main.h"
#include "data.h"
#include "characters.h"
#include "constants/maps.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/species.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const u16 gEggMoves[];
u32 PcSpeciesCanLearnTMHM(u16 species, u8 tm);         // pokemon.c
bool8 PcSpeciesCanLearnTutor(u16 species, u8 tutor);    // party_menu.c
u16 PcTutorMove(u8 tutor);                              // party_menu.c

// The game's charset -> UTF-8, for names in the JSON.
static void Name(FILE *f, const u8 *s)
{
    fputc('"', f);
    for (; *s != EOS; s++)
    {
        u8 c = *s;
        if (c >= CHAR_A && c <= CHAR_Z) fputc('A' + c - CHAR_A, f);
        else if (c >= CHAR_a && c <= CHAR_z) fputc('a' + c - CHAR_a, f);
        else if (c >= CHAR_0 && c <= CHAR_9) fputc('0' + c - CHAR_0, f);
        else if (c == CHAR_SPACE) fputc(' ', f);
        else if (c == CHAR_PERIOD) fputc('.', f);
        else if (c == CHAR_HYPHEN) fputc('-', f);
        else if (c == CHAR_EXCL_MARK) fputc('!', f);
        else if (c == CHAR_QUESTION_MARK) fputc('?', f);
        else if (c == CHAR_COMMA) fputc(',', f);
        else if (c == CHAR_SLASH) fputc('/', f);
        else if (c == CHAR_COLON) fputc(':', f);
        else if (c == CHAR_SGL_QUOTE_RIGHT) fputc('\'', f);
        else if (c == CHAR_MALE) fputs("\\u2642", f);
        else if (c == CHAR_FEMALE) fputs("\\u2640", f);
        else if (c == CHAR_e_ACUTE) fputs("\\u00e9", f);
    }
    fputc('"', f);
}

// Wild land tables of this version with their place names, and the names
// the servers' announcements use (online world events).
static void WorldData(FILE *f)
{
    int i, s;
    u8 name[32];
    static struct SaveBlock1 sNoSave;   // GetMapName peeks at the player's location

    if (gSaveBlock1Ptr == NULL)
        gSaveBlock1Ptr = &sNoSave;
    fprintf(f, ",\n\"wild\":[");
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[i];
        const struct WildPokemonInfo *land = h->landMonsInfo;
        fprintf(f, "%s\n{\"map\":[%u,%u],\"name\":", i ? "," : "", h->mapGroup, h->mapNum);
        GetMapName(name, Overworld_GetMapHeaderByGroupAndId(h->mapGroup, h->mapNum)->regionMapSectionId, 0);
        Name(f, name);
        fprintf(f, ",\"land\":[");
        for (s = 0; land && s < LAND_WILD_COUNT; s++)
            fprintf(f, "%s[%u,%u,%u]", s ? "," : "", land->wildPokemon[s].species,
                    land->wildPokemon[s].minLevel, land->wildPokemon[s].maxLevel);
        fprintf(f, "]}");
    }
    fprintf(f, "],\n\"names\":{");
    for (s = 1; s < NUM_SPECIES; s++)
    {
        fprintf(f, "%s\"%d\":", s > 1 ? "," : "", s);
        Name(f, gSpeciesNames[s]);
    }
    fprintf(f, "},\n\"type_names\":[");
    for (s = 0; s < NUMBER_OF_MON_TYPES; s++)
    {
        if (s)
            fputc(',', f);
        Name(f, gTypeNames[s]);
    }
    fprintf(f, "],\n\"types\":{");
    for (s = 1; s < NUM_SPECIES; s++)
        fprintf(f, "%s\"%d\":[%u,%u]", s > 1 ? "," : "", s, gSpeciesInfo[s].types[0], gSpeciesInfo[s].types[1]);
    fprintf(f, "}");
}

#define EGG_MOVES_SPECIES_OFFSET 20000
#define EGG_MOVES_TERMINATOR 0xFFFF

static void Moves(FILE *f, const char *key, const u16 *moves, int n)
{
    int i;
    fprintf(f, "\"%s\":[", key);
    for (i = 0; i < n; i++)
        fprintf(f, "%s%u", i ? "," : "", moves[i]);
    fprintf(f, "]");
}

void PcDumpLegality(const char *path)
{
    FILE *f = fopen(path, "w");
    u16 species, i;
    static u16 list[512];
    int n;

    if (!f)
        return;
    fprintf(f, "{\"num_species\":%d,\"moves_count\":%d,\"items_count\":%d,"
               "\"old_unown\":[%d,%d],\"max_level\":%d,\n",
            NUM_SPECIES, MOVES_COUNT, ITEMS_COUNT, SPECIES_OLD_UNOWN_B, SPECIES_OLD_UNOWN_Z, MAX_LEVEL);

    fprintf(f, "\"exp\":[");
    for (i = 0; i < 6; i++)
    {
        int lv;
        fprintf(f, "%s[", i ? "," : "");
        for (lv = 0; lv <= MAX_LEVEL; lv++)
            fprintf(f, "%s%lu", lv ? "," : "", (unsigned long)gExperienceTables[i][lv]);
        fprintf(f, "]");
    }
    fprintf(f, "],\n\"species\":{\n");

    for (species = 1; species < NUM_SPECIES; species++)
    {
        const struct SpeciesInfo *s = &gSpeciesInfo[species];
        const u16 *learnset = gLevelUpLearnsets[species];

        fprintf(f, "%s\"%u\":{\"base\":[%u,%u,%u,%u,%u,%u],\"growth\":%u,\"abilities\":[%u,%u],\"gender\":%u,",
                species > 1 ? ",\n" : "", species,
                s->baseHP, s->baseAttack, s->baseDefense, s->baseSpeed, s->baseSpAttack, s->baseSpDefense,
                s->growthRate, s->abilities[0], s->abilities[1], s->genderRatio);

        // level-up: [move, level] pairs (LEVEL_UP_MOVE packs level << 9 | move)
        fprintf(f, "\"levelup\":[");
        for (n = 0; learnset && learnset[n] != 0xFFFF; n++)
            fprintf(f, "%s[%u,%u]", n ? "," : "", learnset[n] & 0x1FF, learnset[n] >> 9);
        fprintf(f, "],");

        n = 0;
        for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
            if (PcSpeciesCanLearnTMHM(species, i))
                list[n++] = ItemIdToBattleMoveId(ITEM_TM01 + i);
        Moves(f, "tmhm", list, n);
        fprintf(f, ",");

        n = 0;
        for (i = 0; i < TUTOR_MOVE_COUNT + 3; i++)
            if (PcSpeciesCanLearnTutor(species, i))
                list[n++] = PcTutorMove(i);
        Moves(f, "tutor", list, n);
        fprintf(f, ",");

        n = 0;
        for (i = 0; gEggMoves[i] != EGG_MOVES_TERMINATOR; i++)
        {
            if (gEggMoves[i] == species + EGG_MOVES_SPECIES_OFFSET)
            {
                for (i++; gEggMoves[i] != EGG_MOVES_TERMINATOR && gEggMoves[i] < EGG_MOVES_SPECIES_OFFSET; i++)
                    list[n++] = gEggMoves[i];
                break;
            }
        }
        Moves(f, "egg", list, n);

        fprintf(f, ",\"evolves_to\":[");
        for (i = 0, n = 0; i < EVOS_PER_MON; i++)
            if (gEvolutionTable[species][i].method)
                fprintf(f, "%s%u", n++ ? "," : "", gEvolutionTable[species][i].targetSpecies);
        fprintf(f, "]}");
    }
    fprintf(f, "\n}");
    WorldData(f);
    fprintf(f, "}\n");
    fclose(f);
}
#endif
