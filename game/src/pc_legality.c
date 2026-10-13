// PC port: dump the game's own Pokémon rules as JSON (--dump-legality FILE)
// for the online servers' legality checks (server/legality.py). Everything
// comes from the tables the game itself uses, so the server and the game can
// never disagree about what is legal.
#include "global.h"
#ifdef PLATFORM_PC
#include <stdio.h>
#include "pokemon.h"
#include "party_menu.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/species.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const u16 gEggMoves[];
u32 PcSpeciesCanLearnTMHM(u16 species, u8 tm);         // pokemon.c
bool8 PcSpeciesCanLearnTutor(u16 species, u8 tutor);    // party_menu.c
u16 PcTutorMove(u8 tutor);                              // party_menu.c

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
    fprintf(f, "\n}}\n");
    fclose(f);
}
#endif
