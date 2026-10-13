/* pc_travel.h — travelling between regions: a Kanto trainer visiting Hoenn's
 * servers (in Emerald) or a Hoenn trainer visiting Kanto's (in FireRed or
 * LeafGreen), as themselves, with their party.
 *
 * The game a trainer leaves saves, writes travel.dat next to its save (the
 * data directory) and exits with PC_EXIT_TRAVEL. The runner then starts the
 * other region's game with --travel and the save named in the file. That
 * game boots straight into the field (no title), takes the travelling party,
 * and joins the world named in the file.
 *
 * While away, travel.dat holds the party: the home save keeps a stale copy
 * that the home game replaces from the file the next time it starts (by
 * travel or not), so the party is only ever in one place. Plain C, no game
 * types: the runner reads it too. */
#ifndef PC_TRAVEL_H
#define PC_TRAVEL_H

#include <stdint.h>

#define PC_EXIT_TRAVEL 77           /* game exit code: start the game in travel.dat */
#define PC_TRAVEL_FILE "travel.dat"
#define PC_TRAVEL_MAGIC "PKTRAVL1"

enum {
    PC_TRAVEL_AWAY,                 /* the trainer is visiting destGame */
    PC_TRAVEL_RETURNING,            /* ...and is on the way home */
};

struct PcTravelFile {
    char magic[8];
    uint8_t state;                  /* PC_TRAVEL_* */
    uint8_t homeGame, destGame;     /* PC_NET_GAME_* */
    uint8_t gender;
    char homeId[16], destId[16];    /* game DLL ids: "firered", "emerald" */
    char homeSave[260], destSave[260];  /* absolute paths */
    char server[32];                /* the world to join on arrival ("" = none) */
    uint8_t name[8];                /* trainer name (game charset) */
    uint8_t otId[4];
    uint8_t partyCount;
    uint8_t pad[3];
    uint8_t party[6][100];          /* struct Pokemon (Gen 3: the same in every game) */
};

/* ---- files (port/src/travel_io.c; travel.dat is in the current directory) ---- */
int PcTravelGameInstalled(const char *id);  /* games\<id>.dll and <id>.pkmn next to the runner */
void PcTravelFullPath(const char *path, char *out, int size);
int PcTravelSamePath(const char *a, const char *b);
int PcTravelRead(struct PcTravelFile *t);
int PcTravelWrite(const struct PcTravelFile *t);
void PcTravelDelete(void);

/* ---- the game side (shared/pc_travel.c) ---- */
extern int gPcTravelArrive;         /* started with --travel: skip the title */
int PcTravelBoot(void);             /* after the save loads at boot: 1 = went straight to the field */
int PcTravelNewGame(void);          /* in CB2_NewGame before the map loads: 1 = a visitor's first
                                       arrival (warp to the starting town set) */
void PcTravelOnSave(void);          /* TrySavingData: keep travel.dat's party current */
void PcTravelFieldUpdate(void);     /* every overworld frame */
int PcTravelDepart(int serverIndex);/* leave for another region's world: saves and exits */
int PcTravelVisitor(void);          /* this trainer is visiting: their PC_NET_GAME_* (home), else -1 */
const char *PcTravelTakeJoin(void); /* the world to join on arrival, once; NULL if none */

/* A visitor looks like their own region's hero (include/pc_travel_game.h):
 * their sprite in every state, battle back pic, trainer card and link version. */
int PcTravelPlayerGfx(int gender, int gfx);              /* the player's field sprite */
const void *PcTravelGraphicsInfo(int gfx, const void *info);  /* the visitor sprites' full anim set */
const void *PcTravelBaseAnims(const void *anims);        /* ...and the walk-step table they use */
int PcTravelBackPic(int gender, int pic);                /* the player's battle back pic */
int PcTravelGameVersion(int version);                    /* VERSION_* on their card and the link */

#endif
