// PC port: the other version's wild encounter tables -- LeafGreen's in the
// FireRed build and FireRed's in the LeafGreen build -- compiled from the same
// generated data/wild_encounters.h with the version flipped. Online, both
// versions' exclusives turn up (wild_encounter.c CrossVersionInfo).
#include "global.h"
#ifdef PLATFORM_PC
#include "wild_encounter.h"
#include "constants/maps.h"
#include "constants/species.h"

#ifdef FIRERED
#undef FIRERED
#define LEAFGREEN 1
#else
#undef LEAFGREEN
#define FIRERED 1
#endif

#define gWildMonHeaders gWildMonHeadersOtherVersion
#include "data/wild_encounters.h"
#endif
