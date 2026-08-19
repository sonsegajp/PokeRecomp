//
// DO NOT MODIFY THIS FILE! It is auto-generated from src/data/wild_encounters.json and Inja template src/data/wild_encounters.json.txt
//


#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_0 20 
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 ENCOUNTER_CHANCE_LAND_MONS_SLOT_0 + 20
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 + 10
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 + 10
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 + 10
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 + 10
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 + 5
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 + 5
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 + 4
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 + 4
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_10 ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 + 1
#define ENCOUNTER_CHANCE_LAND_MONS_SLOT_11 ENCOUNTER_CHANCE_LAND_MONS_SLOT_10 + 1
#define ENCOUNTER_CHANCE_LAND_MONS_TOTAL (ENCOUNTER_CHANCE_LAND_MONS_SLOT_11)
#define ENCOUNTER_CHANCE_WATER_MONS_SLOT_0 60 
#define ENCOUNTER_CHANCE_WATER_MONS_SLOT_1 ENCOUNTER_CHANCE_WATER_MONS_SLOT_0 + 30
#define ENCOUNTER_CHANCE_WATER_MONS_SLOT_2 ENCOUNTER_CHANCE_WATER_MONS_SLOT_1 + 5
#define ENCOUNTER_CHANCE_WATER_MONS_SLOT_3 ENCOUNTER_CHANCE_WATER_MONS_SLOT_2 + 4
#define ENCOUNTER_CHANCE_WATER_MONS_SLOT_4 ENCOUNTER_CHANCE_WATER_MONS_SLOT_3 + 1
#define ENCOUNTER_CHANCE_WATER_MONS_TOTAL (ENCOUNTER_CHANCE_WATER_MONS_SLOT_4)
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_0 60 
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_1 ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_0 + 30
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_2 ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_1 + 5
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_3 ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_2 + 4
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_4 ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_3 + 1
#define ENCOUNTER_CHANCE_ROCK_SMASH_MONS_TOTAL (ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_4)
#define ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_2 60 
#define ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_3 ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_2 + 20
#define ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_4 ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_3 + 20
#define ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_TOTAL (ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_4)
#define ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_SLOT_0 70 
#define ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_SLOT_1 ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_SLOT_0 + 30
#define ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_TOTAL (ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_SLOT_1)
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_5 40 
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_6 ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_5 + 40
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_7 ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_6 + 15
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_8 ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_7 + 4
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_9 ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_8 + 1
#define ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_TOTAL (ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_9)



#ifdef RUBY
const struct WildPokemon PetalburgCity_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
};

const struct WildPokemonInfo PetalburgCity_Ruby_WaterMonsInfo = { 1, PetalburgCity_Ruby_WaterMons };
const struct WildPokemon PetalburgCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo PetalburgCity_Ruby_FishingMonsInfo = { 10, PetalburgCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SlateportCity_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo SlateportCity_Ruby_WaterMonsInfo = { 4, SlateportCity_Ruby_WaterMons };
const struct WildPokemon SlateportCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SlateportCity_Ruby_FishingMonsInfo = { 10, SlateportCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon LilycoveCity_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo LilycoveCity_Ruby_WaterMonsInfo = { 4, LilycoveCity_Ruby_WaterMons };
const struct WildPokemon LilycoveCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_STARYU },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo LilycoveCity_Ruby_FishingMonsInfo = { 10, LilycoveCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon MossdeepCity_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo MossdeepCity_Ruby_WaterMonsInfo = { 4, MossdeepCity_Ruby_WaterMons };
const struct WildPokemon MossdeepCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo MossdeepCity_Ruby_FishingMonsInfo = { 10, MossdeepCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SootopolisCity_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 15, 25, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
};

const struct WildPokemonInfo SootopolisCity_Ruby_WaterMonsInfo = { 1, SootopolisCity_Ruby_WaterMons };
const struct WildPokemon SootopolisCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 10, 15, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 35, 40, SPECIES_GYARADOS },
    { 35, 45, SPECIES_GYARADOS },
    { 5, 45, SPECIES_GYARADOS },
};

const struct WildPokemonInfo SootopolisCity_Ruby_FishingMonsInfo = { 10, SootopolisCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon EverGrandeCity_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo EverGrandeCity_Ruby_WaterMonsInfo = { 4, EverGrandeCity_Ruby_WaterMons };
const struct WildPokemon EverGrandeCity_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_LUVDISC },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_LUVDISC },
    { 30, 35, SPECIES_WAILMER },
    { 30, 35, SPECIES_CORSOLA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo EverGrandeCity_Ruby_FishingMonsInfo = { 10, EverGrandeCity_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon MeteorFalls_1F_1R_Ruby_LandMons[] =
{
    { 16, 16, SPECIES_ZUBAT },
    { 17, 17, SPECIES_ZUBAT },
    { 18, 18, SPECIES_ZUBAT },
    { 15, 15, SPECIES_ZUBAT },
    { 14, 14, SPECIES_ZUBAT },
    { 16, 16, SPECIES_SOLROCK },
    { 18, 18, SPECIES_SOLROCK },
    { 14, 14, SPECIES_SOLROCK },
    { 19, 19, SPECIES_ZUBAT },
    { 20, 20, SPECIES_ZUBAT },
    { 19, 19, SPECIES_ZUBAT },
    { 20, 20, SPECIES_ZUBAT },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Ruby_LandMonsInfo = { 10, MeteorFalls_1F_1R_Ruby_LandMons };
const struct WildPokemon MeteorFalls_1F_1R_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 25, 35, SPECIES_SOLROCK },
    { 15, 25, SPECIES_SOLROCK },
    { 5, 15, SPECIES_SOLROCK },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Ruby_WaterMonsInfo = { 4, MeteorFalls_1F_1R_Ruby_WaterMons };
const struct WildPokemon MeteorFalls_1F_1R_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Ruby_FishingMonsInfo = { 30, MeteorFalls_1F_1R_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon MeteorFalls_1F_2R_Ruby_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_SOLROCK },
    { 33, 33, SPECIES_SOLROCK },
    { 37, 37, SPECIES_SOLROCK },
    { 35, 35, SPECIES_GOLBAT },
    { 39, 39, SPECIES_SOLROCK },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Ruby_LandMonsInfo = { 10, MeteorFalls_1F_2R_Ruby_LandMons };
const struct WildPokemon MeteorFalls_1F_2R_Ruby_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_SOLROCK },
    { 15, 25, SPECIES_SOLROCK },
    { 5, 15, SPECIES_SOLROCK },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Ruby_WaterMonsInfo = { 4, MeteorFalls_1F_2R_Ruby_WaterMons };
const struct WildPokemon MeteorFalls_1F_2R_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Ruby_FishingMonsInfo = { 30, MeteorFalls_1F_2R_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon MeteorFalls_B1F_1R_Ruby_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_SOLROCK },
    { 33, 33, SPECIES_SOLROCK },
    { 37, 37, SPECIES_SOLROCK },
    { 35, 35, SPECIES_GOLBAT },
    { 39, 39, SPECIES_SOLROCK },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Ruby_LandMonsInfo = { 10, MeteorFalls_B1F_1R_Ruby_LandMons };
const struct WildPokemon MeteorFalls_B1F_1R_Ruby_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_SOLROCK },
    { 15, 25, SPECIES_SOLROCK },
    { 5, 15, SPECIES_SOLROCK },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Ruby_WaterMonsInfo = { 4, MeteorFalls_B1F_1R_Ruby_WaterMons };
const struct WildPokemon MeteorFalls_B1F_1R_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Ruby_FishingMonsInfo = { 30, MeteorFalls_B1F_1R_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon MeteorFalls_B1F_2R_Ruby_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 30, 30, SPECIES_BAGON },
    { 35, 35, SPECIES_SOLROCK },
    { 35, 35, SPECIES_BAGON },
    { 37, 37, SPECIES_SOLROCK },
    { 25, 25, SPECIES_BAGON },
    { 39, 39, SPECIES_SOLROCK },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Ruby_LandMonsInfo = { 10, MeteorFalls_B1F_2R_Ruby_LandMons };
const struct WildPokemon MeteorFalls_B1F_2R_Ruby_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_SOLROCK },
    { 15, 25, SPECIES_SOLROCK },
    { 5, 15, SPECIES_SOLROCK },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Ruby_WaterMonsInfo = { 4, MeteorFalls_B1F_2R_Ruby_WaterMons };
const struct WildPokemon MeteorFalls_B1F_2R_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Ruby_FishingMonsInfo = { 30, MeteorFalls_B1F_2R_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon RusturfTunnel_Ruby_LandMons[] =
{
    { 6, 6, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 6, 6, SPECIES_WHISMUR },
    { 6, 6, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
};

const struct WildPokemonInfo RusturfTunnel_Ruby_LandMonsInfo = { 10, RusturfTunnel_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon GraniteCave_1F_Ruby_LandMons[] =
{
    { 7, 7, SPECIES_ZUBAT },
    { 8, 8, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ZUBAT },
    { 9, 9, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 6, 6, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_GEODUDE },
    { 8, 8, SPECIES_GEODUDE },
    { 6, 6, SPECIES_GEODUDE },
    { 9, 9, SPECIES_GEODUDE },
};

const struct WildPokemonInfo GraniteCave_1F_Ruby_LandMonsInfo = { 10, GraniteCave_1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon GraniteCave_B1F_Ruby_LandMons[] =
{
    { 9, 9, SPECIES_ZUBAT },
    { 10, 10, SPECIES_ARON },
    { 9, 9, SPECIES_ARON },
    { 11, 11, SPECIES_ARON },
    { 10, 10, SPECIES_ZUBAT },
    { 9, 9, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 11, 11, SPECIES_MAKUHITA },
    { 10, 10, SPECIES_MAWILE },
    { 10, 10, SPECIES_MAWILE },
    { 9, 9, SPECIES_MAWILE },
    { 11, 11, SPECIES_MAWILE },
};

const struct WildPokemonInfo GraniteCave_B1F_Ruby_LandMonsInfo = { 10, GraniteCave_B1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon GraniteCave_B2F_Ruby_LandMons[] =
{
    { 10, 10, SPECIES_ZUBAT },
    { 11, 11, SPECIES_ARON },
    { 10, 10, SPECIES_ARON },
    { 11, 11, SPECIES_ZUBAT },
    { 12, 12, SPECIES_ARON },
    { 10, 10, SPECIES_ABRA },
    { 10, 10, SPECIES_MAWILE },
    { 11, 11, SPECIES_MAWILE },
    { 12, 12, SPECIES_MAWILE },
    { 10, 10, SPECIES_MAWILE },
    { 12, 12, SPECIES_MAWILE },
    { 10, 10, SPECIES_MAWILE },
};

const struct WildPokemonInfo GraniteCave_B2F_Ruby_LandMonsInfo = { 10, GraniteCave_B2F_Ruby_LandMons };
const struct WildPokemon GraniteCave_B2F_Ruby_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 10, 20, SPECIES_NOSEPASS },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo GraniteCave_B2F_Ruby_RockSmashMonsInfo = { 20, GraniteCave_B2F_Ruby_RockSmashMons };
#endif

#ifdef RUBY
const struct WildPokemon GraniteCave_StevensRoom_Ruby_LandMons[] =
{
    { 7, 7, SPECIES_ZUBAT },
    { 8, 8, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ZUBAT },
    { 9, 9, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 6, 6, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_ARON },
    { 8, 8, SPECIES_ARON },
    { 7, 7, SPECIES_ARON },
    { 8, 8, SPECIES_ARON },
};

const struct WildPokemonInfo GraniteCave_StevensRoom_Ruby_LandMonsInfo = { 10, GraniteCave_StevensRoom_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon PetalburgWoods_Ruby_LandMons[] =
{
    { 5, 5, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_WURMPLE },
    { 5, 5, SPECIES_SHROOMISH },
    { 6, 6, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_SILCOON },
    { 5, 5, SPECIES_CASCOON },
    { 6, 6, SPECIES_WURMPLE },
    { 6, 6, SPECIES_SHROOMISH },
    { 5, 5, SPECIES_TAILLOW },
    { 5, 5, SPECIES_SLAKOTH },
    { 6, 6, SPECIES_TAILLOW },
    { 6, 6, SPECIES_SLAKOTH },
};

const struct WildPokemonInfo PetalburgWoods_Ruby_LandMonsInfo = { 20, PetalburgWoods_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon JaggedPass_Ruby_LandMons[] =
{
    { 19, 19, SPECIES_NUMEL },
    { 19, 19, SPECIES_NUMEL },
    { 19, 19, SPECIES_MACHOP },
    { 18, 18, SPECIES_NUMEL },
    { 18, 18, SPECIES_SPOINK },
    { 18, 18, SPECIES_MACHOP },
    { 19, 19, SPECIES_SPOINK },
    { 20, 20, SPECIES_MACHOP },
    { 20, 20, SPECIES_NUMEL },
    { 20, 20, SPECIES_SPOINK },
    { 20, 20, SPECIES_NUMEL },
    { 20, 20, SPECIES_SPOINK },
};

const struct WildPokemonInfo JaggedPass_Ruby_LandMonsInfo = { 20, JaggedPass_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon FieryPath_Ruby_LandMons[] =
{
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_KOFFING },
    { 16, 16, SPECIES_NUMEL },
    { 15, 15, SPECIES_MACHOP },
    { 15, 15, SPECIES_TORKOAL },
    { 15, 15, SPECIES_SLUGMA },
    { 16, 16, SPECIES_KOFFING },
    { 16, 16, SPECIES_MACHOP },
    { 14, 14, SPECIES_TORKOAL },
    { 16, 16, SPECIES_TORKOAL },
    { 14, 14, SPECIES_GRIMER },
    { 14, 14, SPECIES_GRIMER },
};

const struct WildPokemonInfo FieryPath_Ruby_LandMonsInfo = { 10, FieryPath_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_1F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_1F_Ruby_LandMonsInfo = { 10, MtPyre_1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_2F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_2F_Ruby_LandMonsInfo = { 10, MtPyre_2F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_3F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_3F_Ruby_LandMonsInfo = { 10, MtPyre_3F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_4F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 27, 27, SPECIES_SHUPPET },
    { 27, 27, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_4F_Ruby_LandMonsInfo = { 10, MtPyre_4F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_5F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 27, 27, SPECIES_SHUPPET },
    { 27, 27, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_5F_Ruby_LandMonsInfo = { 10, MtPyre_5F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_6F_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 23, 23, SPECIES_DUSKULL },
    { 22, 22, SPECIES_DUSKULL },
    { 27, 27, SPECIES_SHUPPET },
    { 27, 27, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_6F_Ruby_LandMonsInfo = { 10, MtPyre_6F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_Exterior_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_DUSKULL },
    { 27, 27, SPECIES_MEDITITE },
    { 28, 28, SPECIES_DUSKULL },
    { 29, 29, SPECIES_MEDITITE },
    { 29, 29, SPECIES_DUSKULL },
    { 27, 27, SPECIES_VULPIX },
    { 29, 29, SPECIES_VULPIX },
    { 25, 25, SPECIES_VULPIX },
    { 27, 27, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
};

const struct WildPokemonInfo MtPyre_Exterior_Ruby_LandMonsInfo = { 10, MtPyre_Exterior_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon MtPyre_Summit_Ruby_LandMons[] =
{
    { 28, 28, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
    { 27, 27, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 30, 30, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 24, 24, SPECIES_DUSKULL },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 30, 30, SPECIES_SHUPPET },
    { 28, 28, SPECIES_CHIMECHO },
    { 28, 28, SPECIES_CHIMECHO },
};

const struct WildPokemonInfo MtPyre_Summit_Ruby_LandMonsInfo = { 10, MtPyre_Summit_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Entrance_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Entrance_Ruby_WaterMonsInfo = { 4, SeafloorCavern_Entrance_Ruby_WaterMons };
const struct WildPokemon SeafloorCavern_Entrance_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Entrance_Ruby_FishingMonsInfo = { 10, SeafloorCavern_Entrance_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room1_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room1_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room1_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room2_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room2_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room2_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room3_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room3_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room3_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room4_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room4_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room4_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room5_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room5_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room5_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room6_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room6_Ruby_LandMons };
const struct WildPokemon SeafloorCavern_Room6_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Ruby_WaterMonsInfo = { 4, SeafloorCavern_Room6_Ruby_WaterMons };
const struct WildPokemon SeafloorCavern_Room6_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Ruby_FishingMonsInfo = { 10, SeafloorCavern_Room6_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room7_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room7_Ruby_LandMons };
const struct WildPokemon SeafloorCavern_Room7_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Ruby_WaterMonsInfo = { 4, SeafloorCavern_Room7_Ruby_WaterMons };
const struct WildPokemon SeafloorCavern_Room7_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Ruby_FishingMonsInfo = { 10, SeafloorCavern_Room7_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SeafloorCavern_Room8_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room8_Ruby_LandMonsInfo = { 4, SeafloorCavern_Room8_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon CaveOfOrigin_Entrance_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_Entrance_Ruby_LandMonsInfo = { 4, CaveOfOrigin_Entrance_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon CaveOfOrigin_1F_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_MAWILE },
    { 32, 32, SPECIES_MAWILE },
    { 34, 34, SPECIES_MAWILE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_1F_Ruby_LandMonsInfo = { 4, CaveOfOrigin_1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon CaveOfOrigin_B1F_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_MAWILE },
    { 32, 32, SPECIES_MAWILE },
    { 34, 34, SPECIES_MAWILE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B1F_Ruby_LandMonsInfo = { 4, CaveOfOrigin_B1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon CaveOfOrigin_B2F_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_MAWILE },
    { 32, 32, SPECIES_MAWILE },
    { 34, 34, SPECIES_MAWILE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B2F_Ruby_LandMonsInfo = { 4, CaveOfOrigin_B2F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon CaveOfOrigin_B3F_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_MAWILE },
    { 32, 32, SPECIES_MAWILE },
    { 34, 34, SPECIES_MAWILE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B3F_Ruby_LandMonsInfo = { 4, CaveOfOrigin_B3F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon VictoryRoad_1F_Ruby_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_HARIYAMA },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_LOUDRED },
    { 36, 36, SPECIES_ZUBAT },
    { 36, 36, SPECIES_MAKUHITA },
    { 38, 38, SPECIES_GOLBAT },
    { 38, 38, SPECIES_HARIYAMA },
    { 36, 36, SPECIES_ARON },
    { 36, 36, SPECIES_WHISMUR },
    { 36, 36, SPECIES_ARON },
    { 36, 36, SPECIES_WHISMUR },
};

const struct WildPokemonInfo VictoryRoad_1F_Ruby_LandMonsInfo = { 10, VictoryRoad_1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon VictoryRoad_B1F_Ruby_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_HARIYAMA },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_MEDICHAM },
    { 38, 38, SPECIES_GOLBAT },
    { 38, 38, SPECIES_HARIYAMA },
    { 42, 42, SPECIES_GOLBAT },
    { 42, 42, SPECIES_HARIYAMA },
    { 42, 42, SPECIES_LAIRON },
    { 38, 38, SPECIES_MEDITITE },
    { 42, 42, SPECIES_LAIRON },
    { 38, 38, SPECIES_MEDITITE },
};

const struct WildPokemonInfo VictoryRoad_B1F_Ruby_LandMonsInfo = { 10, VictoryRoad_B1F_Ruby_LandMons };
const struct WildPokemon VictoryRoad_B1F_Ruby_RockSmashMons[] =
{
    { 30, 40, SPECIES_GRAVELER },
    { 30, 40, SPECIES_GEODUDE },
    { 35, 40, SPECIES_GRAVELER },
    { 35, 40, SPECIES_GRAVELER },
    { 35, 40, SPECIES_GRAVELER },
};

const struct WildPokemonInfo VictoryRoad_B1F_Ruby_RockSmashMonsInfo = { 20, VictoryRoad_B1F_Ruby_RockSmashMons };
#endif

#ifdef RUBY
const struct WildPokemon VictoryRoad_B2F_Ruby_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_MAWILE },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_MEDICHAM },
    { 42, 42, SPECIES_GOLBAT },
    { 42, 42, SPECIES_MAWILE },
    { 44, 44, SPECIES_GOLBAT },
    { 44, 44, SPECIES_MAWILE },
    { 42, 42, SPECIES_LAIRON },
    { 42, 42, SPECIES_MEDICHAM },
    { 44, 44, SPECIES_LAIRON },
    { 44, 44, SPECIES_MEDICHAM },
};

const struct WildPokemonInfo VictoryRoad_B2F_Ruby_LandMonsInfo = { 10, VictoryRoad_B2F_Ruby_LandMons };
const struct WildPokemon VictoryRoad_B2F_Ruby_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 25, 30, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo VictoryRoad_B2F_Ruby_WaterMonsInfo = { 4, VictoryRoad_B2F_Ruby_WaterMons };
const struct WildPokemon VictoryRoad_B2F_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo VictoryRoad_B2F_Ruby_FishingMonsInfo = { 30, VictoryRoad_B2F_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Ruby_LandMonsInfo = { 10, ShoalCave_LowTideEntranceRoom_Ruby_LandMons };
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 35, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Ruby_WaterMonsInfo = { 4, ShoalCave_LowTideEntranceRoom_Ruby_WaterMons };
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Ruby_FishingMonsInfo = { 10, ShoalCave_LowTideEntranceRoom_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon ShoalCave_LowTideInnerRoom_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Ruby_LandMonsInfo = { 10, ShoalCave_LowTideInnerRoom_Ruby_LandMons };
const struct WildPokemon ShoalCave_LowTideInnerRoom_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 35, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Ruby_WaterMonsInfo = { 4, ShoalCave_LowTideInnerRoom_Ruby_WaterMons };
const struct WildPokemon ShoalCave_LowTideInnerRoom_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Ruby_FishingMonsInfo = { 10, ShoalCave_LowTideInnerRoom_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon ShoalCave_LowTideStairsRoom_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideStairsRoom_Ruby_LandMonsInfo = { 10, ShoalCave_LowTideStairsRoom_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon ShoalCave_LowTideLowerRoom_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideLowerRoom_Ruby_LandMonsInfo = { 10, ShoalCave_LowTideLowerRoom_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon ShoalCave_LowTideIceRoom_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 26, 26, SPECIES_SNORUNT },
    { 32, 32, SPECIES_SPHEAL },
    { 30, 30, SPECIES_GOLBAT },
    { 28, 28, SPECIES_SNORUNT },
    { 32, 32, SPECIES_GOLBAT },
    { 30, 30, SPECIES_SNORUNT },
};

const struct WildPokemonInfo ShoalCave_LowTideIceRoom_Ruby_LandMonsInfo = { 10, ShoalCave_LowTideIceRoom_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon NewMauville_Entrance_Ruby_LandMons[] =
{
    { 24, 24, SPECIES_VOLTORB },
    { 24, 24, SPECIES_MAGNEMITE },
    { 25, 25, SPECIES_VOLTORB },
    { 25, 25, SPECIES_MAGNEMITE },
    { 23, 23, SPECIES_VOLTORB },
    { 23, 23, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_VOLTORB },
    { 26, 26, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
};

const struct WildPokemonInfo NewMauville_Entrance_Ruby_LandMonsInfo = { 10, NewMauville_Entrance_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon NewMauville_Inside_Ruby_LandMons[] =
{
    { 24, 24, SPECIES_VOLTORB },
    { 24, 24, SPECIES_MAGNEMITE },
    { 25, 25, SPECIES_VOLTORB },
    { 25, 25, SPECIES_MAGNEMITE },
    { 23, 23, SPECIES_VOLTORB },
    { 23, 23, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_VOLTORB },
    { 26, 26, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_ELECTRODE },
    { 26, 26, SPECIES_MAGNETON },
};

const struct WildPokemonInfo NewMauville_Inside_Ruby_LandMonsInfo = { 10, NewMauville_Inside_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon AbandonedShip_Rooms_B1F_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_Rooms_B1F_Ruby_WaterMonsInfo = { 4, AbandonedShip_Rooms_B1F_Ruby_WaterMons };
const struct WildPokemon AbandonedShip_Rooms_B1F_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_TENTACOOL },
    { 25, 30, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
    { 25, 30, SPECIES_TENTACRUEL },
    { 20, 25, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_Rooms_B1F_Ruby_FishingMonsInfo = { 20, AbandonedShip_Rooms_B1F_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon AbandonedShip_HiddenFloorCorridors_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_HiddenFloorCorridors_Ruby_WaterMonsInfo = { 4, AbandonedShip_HiddenFloorCorridors_Ruby_WaterMons };
const struct WildPokemon AbandonedShip_HiddenFloorCorridors_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_TENTACOOL },
    { 25, 30, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
    { 25, 30, SPECIES_TENTACRUEL },
    { 20, 25, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_HiddenFloorCorridors_Ruby_FishingMonsInfo = { 20, AbandonedShip_HiddenFloorCorridors_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SkyPillar_1F_Ruby_LandMons[] =
{
    { 48, 48, SPECIES_MAWILE },
    { 48, 48, SPECIES_GOLBAT },
    { 50, 50, SPECIES_GOLBAT },
    { 50, 50, SPECIES_MAWILE },
    { 48, 48, SPECIES_CLAYDOL },
    { 48, 48, SPECIES_DUSCLOPS },
    { 50, 50, SPECIES_DUSCLOPS },
    { 49, 49, SPECIES_CLAYDOL },
    { 47, 47, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 47, 47, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
};

const struct WildPokemonInfo SkyPillar_1F_Ruby_LandMonsInfo = { 10, SkyPillar_1F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SkyPillar_3F_Ruby_LandMons[] =
{
    { 51, 51, SPECIES_MAWILE },
    { 51, 51, SPECIES_GOLBAT },
    { 53, 53, SPECIES_GOLBAT },
    { 53, 53, SPECIES_MAWILE },
    { 51, 51, SPECIES_CLAYDOL },
    { 51, 51, SPECIES_DUSCLOPS },
    { 53, 53, SPECIES_DUSCLOPS },
    { 52, 52, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 53, 53, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 53, 53, SPECIES_CLAYDOL },
};

const struct WildPokemonInfo SkyPillar_3F_Ruby_LandMonsInfo = { 10, SkyPillar_3F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon SkyPillar_5F_Ruby_LandMons[] =
{
    { 54, 54, SPECIES_MAWILE },
    { 54, 54, SPECIES_GOLBAT },
    { 56, 56, SPECIES_GOLBAT },
    { 56, 56, SPECIES_MAWILE },
    { 54, 54, SPECIES_CLAYDOL },
    { 54, 54, SPECIES_DUSCLOPS },
    { 56, 56, SPECIES_DUSCLOPS },
    { 55, 55, SPECIES_CLAYDOL },
    { 56, 56, SPECIES_CLAYDOL },
    { 57, 57, SPECIES_ALTARIA },
    { 54, 54, SPECIES_ALTARIA },
    { 60, 60, SPECIES_ALTARIA },
};

const struct WildPokemonInfo SkyPillar_5F_Ruby_LandMonsInfo = { 10, SkyPillar_5F_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon Route101_Ruby_LandMons[] =
{
    { 2, 2, SPECIES_WURMPLE },
    { 2, 2, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_WURMPLE },
    { 3, 3, SPECIES_WURMPLE },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_WURMPLE },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_POOCHYENA },
    { 2, 2, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
};

const struct WildPokemonInfo Route101_Ruby_LandMonsInfo = { 20, Route101_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon Route102_Ruby_LandMons[] =
{
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_WURMPLE },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_WURMPLE },
    { 3, 3, SPECIES_SEEDOT },
    { 4, 4, SPECIES_SEEDOT },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_RALTS },
    { 4, 4, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route102_Ruby_LandMonsInfo = { 20, Route102_Ruby_LandMons };
const struct WildPokemon Route102_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route102_Ruby_WaterMonsInfo = { 4, Route102_Ruby_WaterMons };
const struct WildPokemon Route102_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo Route102_Ruby_FishingMonsInfo = { 30, Route102_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route103_Ruby_LandMons[] =
{
    { 2, 2, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_WINGULL },
    { 3, 3, SPECIES_WINGULL },
    { 2, 2, SPECIES_WINGULL },
    { 4, 4, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route103_Ruby_LandMonsInfo = { 20, Route103_Ruby_LandMons };
const struct WildPokemon Route103_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route103_Ruby_WaterMonsInfo = { 4, Route103_Ruby_WaterMons };
const struct WildPokemon Route103_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route103_Ruby_FishingMonsInfo = { 30, Route103_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route104_Ruby_LandMons[] =
{
    { 4, 4, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_WURMPLE },
    { 5, 5, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_WURMPLE },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_TAILLOW },
    { 5, 5, SPECIES_TAILLOW },
    { 4, 4, SPECIES_WINGULL },
    { 4, 4, SPECIES_WINGULL },
    { 3, 3, SPECIES_WINGULL },
    { 5, 5, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route104_Ruby_LandMonsInfo = { 20, Route104_Ruby_LandMons };
const struct WildPokemon Route104_Ruby_WaterMons[] =
{
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route104_Ruby_WaterMonsInfo = { 4, Route104_Ruby_WaterMons };
const struct WildPokemon Route104_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 20, 25, SPECIES_MAGIKARP },
    { 35, 40, SPECIES_MAGIKARP },
    { 40, 45, SPECIES_MAGIKARP },
};

const struct WildPokemonInfo Route104_Ruby_FishingMonsInfo = { 30, Route104_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route105_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route105_Ruby_WaterMonsInfo = { 4, Route105_Ruby_WaterMons };
const struct WildPokemon Route105_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route105_Ruby_FishingMonsInfo = { 30, Route105_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route106_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route106_Ruby_WaterMonsInfo = { 4, Route106_Ruby_WaterMons };
const struct WildPokemon Route106_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route106_Ruby_FishingMonsInfo = { 30, Route106_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route107_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route107_Ruby_WaterMonsInfo = { 4, Route107_Ruby_WaterMons };
const struct WildPokemon Route107_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route107_Ruby_FishingMonsInfo = { 30, Route107_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route108_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route108_Ruby_WaterMonsInfo = { 4, Route108_Ruby_WaterMons };
const struct WildPokemon Route108_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route108_Ruby_FishingMonsInfo = { 30, Route108_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route109_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route109_Ruby_WaterMonsInfo = { 4, Route109_Ruby_WaterMons };
const struct WildPokemon Route109_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route109_Ruby_FishingMonsInfo = { 30, Route109_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route110_Ruby_LandMons[] =
{
    { 12, 12, SPECIES_ZIGZAGOON },
    { 12, 12, SPECIES_ELECTRIKE },
    { 12, 12, SPECIES_GULPIN },
    { 13, 13, SPECIES_ELECTRIKE },
    { 13, 13, SPECIES_MINUN },
    { 13, 13, SPECIES_ODDISH },
    { 13, 13, SPECIES_MINUN },
    { 13, 13, SPECIES_GULPIN },
    { 12, 12, SPECIES_WINGULL },
    { 12, 12, SPECIES_WINGULL },
    { 12, 12, SPECIES_PLUSLE },
    { 13, 13, SPECIES_PLUSLE },
};

const struct WildPokemonInfo Route110_Ruby_LandMonsInfo = { 20, Route110_Ruby_LandMons };
const struct WildPokemon Route110_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route110_Ruby_WaterMonsInfo = { 4, Route110_Ruby_WaterMons };
const struct WildPokemon Route110_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route110_Ruby_FishingMonsInfo = { 30, Route110_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route111_Ruby_LandMons[] =
{
    { 20, 20, SPECIES_SANDSHREW },
    { 20, 20, SPECIES_TRAPINCH },
    { 21, 21, SPECIES_SANDSHREW },
    { 21, 21, SPECIES_TRAPINCH },
    { 19, 19, SPECIES_CACNEA },
    { 21, 21, SPECIES_CACNEA },
    { 19, 19, SPECIES_SANDSHREW },
    { 19, 19, SPECIES_TRAPINCH },
    { 20, 20, SPECIES_BALTOY },
    { 20, 20, SPECIES_BALTOY },
    { 22, 22, SPECIES_BALTOY },
    { 22, 22, SPECIES_BALTOY },
};

const struct WildPokemonInfo Route111_Ruby_LandMonsInfo = { 10, Route111_Ruby_LandMons };
const struct WildPokemon Route111_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route111_Ruby_WaterMonsInfo = { 4, Route111_Ruby_WaterMons };
const struct WildPokemon Route111_Ruby_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo Route111_Ruby_RockSmashMonsInfo = { 20, Route111_Ruby_RockSmashMons };
const struct WildPokemon Route111_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route111_Ruby_FishingMonsInfo = { 30, Route111_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route112_Ruby_LandMons[] =
{
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_MACHOP },
    { 14, 14, SPECIES_NUMEL },
    { 14, 14, SPECIES_NUMEL },
    { 14, 14, SPECIES_MACHOP },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_MACHOP },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
};

const struct WildPokemonInfo Route112_Ruby_LandMonsInfo = { 20, Route112_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon Route113_Ruby_LandMons[] =
{
    { 15, 15, SPECIES_SPINDA },
    { 15, 15, SPECIES_SPINDA },
    { 15, 15, SPECIES_SANDSHREW },
    { 14, 14, SPECIES_SPINDA },
    { 14, 14, SPECIES_SPINDA },
    { 14, 14, SPECIES_SANDSHREW },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SANDSHREW },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SKARMORY },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SKARMORY },
};

const struct WildPokemonInfo Route113_Ruby_LandMonsInfo = { 20, Route113_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon Route114_Ruby_LandMons[] =
{
    { 16, 16, SPECIES_SWABLU },
    { 16, 16, SPECIES_SEEDOT },
    { 17, 17, SPECIES_SWABLU },
    { 15, 15, SPECIES_SWABLU },
    { 15, 15, SPECIES_SEEDOT },
    { 16, 16, SPECIES_ZANGOOSE },
    { 16, 16, SPECIES_NUZLEAF },
    { 18, 18, SPECIES_NUZLEAF },
    { 17, 17, SPECIES_ZANGOOSE },
    { 15, 15, SPECIES_ZANGOOSE },
    { 17, 17, SPECIES_ZANGOOSE },
    { 15, 15, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route114_Ruby_LandMonsInfo = { 20, Route114_Ruby_LandMons };
const struct WildPokemon Route114_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route114_Ruby_WaterMonsInfo = { 4, Route114_Ruby_WaterMons };
const struct WildPokemon Route114_Ruby_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo Route114_Ruby_RockSmashMonsInfo = { 20, Route114_Ruby_RockSmashMons };
const struct WildPokemon Route114_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route114_Ruby_FishingMonsInfo = { 30, Route114_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route115_Ruby_LandMons[] =
{
    { 23, 23, SPECIES_SWABLU },
    { 23, 23, SPECIES_TAILLOW },
    { 25, 25, SPECIES_SWABLU },
    { 24, 24, SPECIES_TAILLOW },
    { 25, 25, SPECIES_TAILLOW },
    { 25, 25, SPECIES_SWELLOW },
    { 24, 24, SPECIES_JIGGLYPUFF },
    { 25, 25, SPECIES_JIGGLYPUFF },
    { 24, 24, SPECIES_WINGULL },
    { 24, 24, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 25, 25, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route115_Ruby_LandMonsInfo = { 20, Route115_Ruby_LandMons };
const struct WildPokemon Route115_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route115_Ruby_WaterMonsInfo = { 4, Route115_Ruby_WaterMons };
const struct WildPokemon Route115_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route115_Ruby_FishingMonsInfo = { 30, Route115_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route116_Ruby_LandMons[] =
{
    { 6, 6, SPECIES_ZIGZAGOON },
    { 6, 6, SPECIES_WHISMUR },
    { 6, 6, SPECIES_NINCADA },
    { 7, 7, SPECIES_WHISMUR },
    { 7, 7, SPECIES_NINCADA },
    { 6, 6, SPECIES_TAILLOW },
    { 7, 7, SPECIES_TAILLOW },
    { 8, 8, SPECIES_TAILLOW },
    { 7, 7, SPECIES_ZIGZAGOON },
    { 8, 8, SPECIES_ZIGZAGOON },
    { 7, 7, SPECIES_SKITTY },
    { 8, 8, SPECIES_SKITTY },
};

const struct WildPokemonInfo Route116_Ruby_LandMonsInfo = { 20, Route116_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon Route117_Ruby_LandMons[] =
{
    { 13, 13, SPECIES_ZIGZAGOON },
    { 13, 13, SPECIES_ROSELIA },
    { 14, 14, SPECIES_ZIGZAGOON },
    { 14, 14, SPECIES_ROSELIA },
    { 13, 13, SPECIES_MARILL },
    { 13, 13, SPECIES_ODDISH },
    { 13, 13, SPECIES_ILLUMISE },
    { 13, 13, SPECIES_ILLUMISE },
    { 14, 14, SPECIES_ILLUMISE },
    { 14, 14, SPECIES_ILLUMISE },
    { 13, 13, SPECIES_VOLBEAT },
    { 13, 13, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route117_Ruby_LandMonsInfo = { 20, Route117_Ruby_LandMons };
const struct WildPokemon Route117_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route117_Ruby_WaterMonsInfo = { 4, Route117_Ruby_WaterMons };
const struct WildPokemon Route117_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo Route117_Ruby_FishingMonsInfo = { 30, Route117_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route118_Ruby_LandMons[] =
{
    { 24, 24, SPECIES_ZIGZAGOON },
    { 24, 24, SPECIES_ELECTRIKE },
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_ELECTRIKE },
    { 26, 26, SPECIES_LINOONE },
    { 26, 26, SPECIES_MANECTRIC },
    { 25, 25, SPECIES_WINGULL },
    { 25, 25, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route118_Ruby_LandMonsInfo = { 20, Route118_Ruby_LandMons };
const struct WildPokemon Route118_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route118_Ruby_WaterMonsInfo = { 4, Route118_Ruby_WaterMons };
const struct WildPokemon Route118_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_CARVANHA },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_CARVANHA },
    { 20, 25, SPECIES_CARVANHA },
    { 35, 40, SPECIES_CARVANHA },
    { 40, 45, SPECIES_CARVANHA },
};

const struct WildPokemonInfo Route118_Ruby_FishingMonsInfo = { 30, Route118_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route119_Ruby_LandMons[] =
{
    { 25, 25, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_LINOONE },
    { 27, 27, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 24, 24, SPECIES_ODDISH },
    { 25, 25, SPECIES_TROPIUS },
    { 26, 26, SPECIES_TROPIUS },
    { 27, 27, SPECIES_TROPIUS },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route119_Ruby_LandMonsInfo = { 15, Route119_Ruby_LandMons };
const struct WildPokemon Route119_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route119_Ruby_WaterMonsInfo = { 4, Route119_Ruby_WaterMons };
const struct WildPokemon Route119_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_CARVANHA },
    { 25, 30, SPECIES_CARVANHA },
    { 30, 35, SPECIES_CARVANHA },
    { 20, 25, SPECIES_CARVANHA },
    { 35, 40, SPECIES_CARVANHA },
    { 40, 45, SPECIES_CARVANHA },
};

const struct WildPokemonInfo Route119_Ruby_FishingMonsInfo = { 30, Route119_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route120_Ruby_LandMons[] =
{
    { 25, 25, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_LINOONE },
    { 27, 27, SPECIES_LINOONE },
    { 25, 25, SPECIES_ODDISH },
    { 25, 25, SPECIES_MARILL },
    { 26, 26, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 27, 27, SPECIES_MARILL },
    { 25, 25, SPECIES_ABSOL },
    { 27, 27, SPECIES_ABSOL },
    { 25, 25, SPECIES_KECLEON },
    { 25, 25, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route120_Ruby_LandMonsInfo = { 20, Route120_Ruby_LandMons };
const struct WildPokemon Route120_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route120_Ruby_WaterMonsInfo = { 4, Route120_Ruby_WaterMons };
const struct WildPokemon Route120_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route120_Ruby_FishingMonsInfo = { 30, Route120_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route121_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_DUSKULL },
    { 26, 26, SPECIES_LINOONE },
    { 28, 28, SPECIES_DUSKULL },
    { 28, 28, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 28, 28, SPECIES_ODDISH },
    { 28, 28, SPECIES_GLOOM },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route121_Ruby_LandMonsInfo = { 20, Route121_Ruby_LandMons };
const struct WildPokemon Route121_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route121_Ruby_WaterMonsInfo = { 4, Route121_Ruby_WaterMons };
const struct WildPokemon Route121_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route121_Ruby_FishingMonsInfo = { 30, Route121_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route122_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route122_Ruby_WaterMonsInfo = { 4, Route122_Ruby_WaterMons };
const struct WildPokemon Route122_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route122_Ruby_FishingMonsInfo = { 30, Route122_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route123_Ruby_LandMons[] =
{
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_DUSKULL },
    { 26, 26, SPECIES_LINOONE },
    { 28, 28, SPECIES_DUSKULL },
    { 28, 28, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 28, 28, SPECIES_ODDISH },
    { 28, 28, SPECIES_GLOOM },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route123_Ruby_LandMonsInfo = { 20, Route123_Ruby_LandMons };
const struct WildPokemon Route123_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route123_Ruby_WaterMonsInfo = { 4, Route123_Ruby_WaterMons };
const struct WildPokemon Route123_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route123_Ruby_FishingMonsInfo = { 30, Route123_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route124_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route124_Ruby_WaterMonsInfo = { 4, Route124_Ruby_WaterMons };
const struct WildPokemon Route124_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route124_Ruby_FishingMonsInfo = { 30, Route124_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route125_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route125_Ruby_WaterMonsInfo = { 4, Route125_Ruby_WaterMons };
const struct WildPokemon Route125_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route125_Ruby_FishingMonsInfo = { 30, Route125_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route126_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route126_Ruby_WaterMonsInfo = { 4, Route126_Ruby_WaterMons };
const struct WildPokemon Route126_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route126_Ruby_FishingMonsInfo = { 30, Route126_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route127_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route127_Ruby_WaterMonsInfo = { 4, Route127_Ruby_WaterMons };
const struct WildPokemon Route127_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route127_Ruby_FishingMonsInfo = { 30, Route127_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route128_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route128_Ruby_WaterMonsInfo = { 4, Route128_Ruby_WaterMons };
const struct WildPokemon Route128_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_LUVDISC },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_LUVDISC },
    { 30, 35, SPECIES_WAILMER },
    { 30, 35, SPECIES_CORSOLA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route128_Ruby_FishingMonsInfo = { 30, Route128_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route129_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 35, 40, SPECIES_WAILORD },
};

const struct WildPokemonInfo Route129_Ruby_WaterMonsInfo = { 4, Route129_Ruby_WaterMons };
const struct WildPokemon Route129_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route129_Ruby_FishingMonsInfo = { 30, Route129_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route130_Ruby_LandMons[] =
{
    { 30, 30, SPECIES_WYNAUT },
    { 35, 35, SPECIES_WYNAUT },
    { 25, 25, SPECIES_WYNAUT },
    { 40, 40, SPECIES_WYNAUT },
    { 20, 20, SPECIES_WYNAUT },
    { 45, 45, SPECIES_WYNAUT },
    { 15, 15, SPECIES_WYNAUT },
    { 50, 50, SPECIES_WYNAUT },
    { 10, 10, SPECIES_WYNAUT },
    { 5, 5, SPECIES_WYNAUT },
    { 10, 10, SPECIES_WYNAUT },
    { 5, 5, SPECIES_WYNAUT },
};

const struct WildPokemonInfo Route130_Ruby_LandMonsInfo = { 20, Route130_Ruby_LandMons };
const struct WildPokemon Route130_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route130_Ruby_WaterMonsInfo = { 4, Route130_Ruby_WaterMons };
const struct WildPokemon Route130_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route130_Ruby_FishingMonsInfo = { 30, Route130_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route131_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route131_Ruby_WaterMonsInfo = { 4, Route131_Ruby_WaterMons };
const struct WildPokemon Route131_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route131_Ruby_FishingMonsInfo = { 30, Route131_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route132_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route132_Ruby_WaterMonsInfo = { 4, Route132_Ruby_WaterMons };
const struct WildPokemon Route132_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route132_Ruby_FishingMonsInfo = { 30, Route132_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route133_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route133_Ruby_WaterMonsInfo = { 4, Route133_Ruby_WaterMons };
const struct WildPokemon Route133_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route133_Ruby_FishingMonsInfo = { 30, Route133_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Route134_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route134_Ruby_WaterMonsInfo = { 4, Route134_Ruby_WaterMons };
const struct WildPokemon Route134_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route134_Ruby_FishingMonsInfo = { 30, Route134_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SafariZone_Northwest_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_RHYHORN },
    { 27, 27, SPECIES_ODDISH },
    { 29, 29, SPECIES_RHYHORN },
    { 29, 29, SPECIES_ODDISH },
    { 27, 27, SPECIES_DODUO },
    { 29, 29, SPECIES_GLOOM },
    { 31, 31, SPECIES_GLOOM },
    { 29, 29, SPECIES_DODUO },
    { 29, 29, SPECIES_DODRIO },
    { 27, 27, SPECIES_PINSIR },
    { 31, 31, SPECIES_DODRIO },
    { 29, 29, SPECIES_PINSIR },
};

const struct WildPokemonInfo SafariZone_Northwest_Ruby_LandMonsInfo = { 25, SafariZone_Northwest_Ruby_LandMons };
const struct WildPokemon SafariZone_Northwest_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_PSYDUCK },
    { 20, 30, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_GOLDUCK },
    { 25, 40, SPECIES_GOLDUCK },
};

const struct WildPokemonInfo SafariZone_Northwest_Ruby_WaterMonsInfo = { 9, SafariZone_Northwest_Ruby_WaterMons };
const struct WildPokemon SafariZone_Northwest_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 25, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_GOLDEEN },
    { 25, 30, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_SEAKING },
    { 35, 40, SPECIES_SEAKING },
    { 25, 30, SPECIES_SEAKING },
};

const struct WildPokemonInfo SafariZone_Northwest_Ruby_FishingMonsInfo = { 35, SafariZone_Northwest_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SafariZone_Northeast_Ruby_LandMons[] =
{
    { 27, 27, SPECIES_PHANPY },
    { 27, 27, SPECIES_ODDISH },
    { 29, 29, SPECIES_PHANPY },
    { 29, 29, SPECIES_ODDISH },
    { 27, 27, SPECIES_NATU },
    { 29, 29, SPECIES_GLOOM },
    { 31, 31, SPECIES_GLOOM },
    { 29, 29, SPECIES_NATU },
    { 29, 29, SPECIES_XATU },
    { 27, 27, SPECIES_HERACROSS },
    { 31, 31, SPECIES_XATU },
    { 29, 29, SPECIES_HERACROSS },
};

const struct WildPokemonInfo SafariZone_Northeast_Ruby_LandMonsInfo = { 25, SafariZone_Northeast_Ruby_LandMons };
const struct WildPokemon SafariZone_Northeast_Ruby_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 20, 25, SPECIES_GEODUDE },
    { 25, 30, SPECIES_GEODUDE },
};

const struct WildPokemonInfo SafariZone_Northeast_Ruby_RockSmashMonsInfo = { 25, SafariZone_Northeast_Ruby_RockSmashMons };
#endif

#ifdef RUBY
const struct WildPokemon SafariZone_Southwest_Ruby_LandMons[] =
{
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 25, 25, SPECIES_GIRAFARIG },
    { 27, 27, SPECIES_GIRAFARIG },
    { 25, 25, SPECIES_NATU },
    { 25, 25, SPECIES_DODUO },
    { 25, 25, SPECIES_GLOOM },
    { 27, 27, SPECIES_WOBBUFFET },
    { 25, 25, SPECIES_PIKACHU },
    { 27, 27, SPECIES_WOBBUFFET },
    { 27, 27, SPECIES_PIKACHU },
    { 29, 29, SPECIES_WOBBUFFET },
};

const struct WildPokemonInfo SafariZone_Southwest_Ruby_LandMonsInfo = { 25, SafariZone_Southwest_Ruby_LandMons };
const struct WildPokemon SafariZone_Southwest_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_PSYDUCK },
    { 20, 30, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
};

const struct WildPokemonInfo SafariZone_Southwest_Ruby_WaterMonsInfo = { 9, SafariZone_Southwest_Ruby_WaterMons };
const struct WildPokemon SafariZone_Southwest_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 25, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_GOLDEEN },
    { 25, 30, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_SEAKING },
    { 35, 40, SPECIES_SEAKING },
    { 25, 30, SPECIES_SEAKING },
};

const struct WildPokemonInfo SafariZone_Southwest_Ruby_FishingMonsInfo = { 35, SafariZone_Southwest_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon SafariZone_Southeast_Ruby_LandMons[] =
{
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 25, 25, SPECIES_GIRAFARIG },
    { 27, 27, SPECIES_GIRAFARIG },
    { 25, 25, SPECIES_NATU },
    { 25, 25, SPECIES_DODUO },
    { 25, 25, SPECIES_GLOOM },
    { 27, 27, SPECIES_WOBBUFFET },
    { 25, 25, SPECIES_PIKACHU },
    { 27, 27, SPECIES_WOBBUFFET },
    { 27, 27, SPECIES_PIKACHU },
    { 29, 29, SPECIES_WOBBUFFET },
};

const struct WildPokemonInfo SafariZone_Southeast_Ruby_LandMonsInfo = { 25, SafariZone_Southeast_Ruby_LandMons };
#endif

#ifdef RUBY
const struct WildPokemon DewfordTown_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo DewfordTown_Ruby_WaterMonsInfo = { 4, DewfordTown_Ruby_WaterMons };
const struct WildPokemon DewfordTown_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo DewfordTown_Ruby_FishingMonsInfo = { 10, DewfordTown_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon PacifidlogTown_Ruby_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo PacifidlogTown_Ruby_WaterMonsInfo = { 4, PacifidlogTown_Ruby_WaterMons };
const struct WildPokemon PacifidlogTown_Ruby_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo PacifidlogTown_Ruby_FishingMonsInfo = { 10, PacifidlogTown_Ruby_FishingMons };
#endif

#ifdef RUBY
const struct WildPokemon Underwater1_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_CLAMPERL },
    { 20, 30, SPECIES_CHINCHOU },
    { 30, 35, SPECIES_CLAMPERL },
    { 30, 35, SPECIES_RELICANTH },
    { 30, 35, SPECIES_RELICANTH },
};

const struct WildPokemonInfo Underwater1_Ruby_WaterMonsInfo = { 4, Underwater1_Ruby_WaterMons };
#endif

#ifdef RUBY
const struct WildPokemon Underwater2_Ruby_WaterMons[] =
{
    { 20, 30, SPECIES_CLAMPERL },
    { 20, 30, SPECIES_CHINCHOU },
    { 30, 35, SPECIES_CLAMPERL },
    { 30, 35, SPECIES_RELICANTH },
    { 30, 35, SPECIES_RELICANTH },
};

const struct WildPokemonInfo Underwater2_Ruby_WaterMonsInfo = { 4, Underwater2_Ruby_WaterMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon PetalburgCity_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
};

const struct WildPokemonInfo PetalburgCity_Sapphire_WaterMonsInfo = { 1, PetalburgCity_Sapphire_WaterMons };
const struct WildPokemon PetalburgCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo PetalburgCity_Sapphire_FishingMonsInfo = { 10, PetalburgCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SlateportCity_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo SlateportCity_Sapphire_WaterMonsInfo = { 4, SlateportCity_Sapphire_WaterMons };
const struct WildPokemon SlateportCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SlateportCity_Sapphire_FishingMonsInfo = { 10, SlateportCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon LilycoveCity_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo LilycoveCity_Sapphire_WaterMonsInfo = { 4, LilycoveCity_Sapphire_WaterMons };
const struct WildPokemon LilycoveCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_STARYU },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo LilycoveCity_Sapphire_FishingMonsInfo = { 10, LilycoveCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MossdeepCity_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo MossdeepCity_Sapphire_WaterMonsInfo = { 4, MossdeepCity_Sapphire_WaterMons };
const struct WildPokemon MossdeepCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo MossdeepCity_Sapphire_FishingMonsInfo = { 10, MossdeepCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SootopolisCity_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 15, 25, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
};

const struct WildPokemonInfo SootopolisCity_Sapphire_WaterMonsInfo = { 1, SootopolisCity_Sapphire_WaterMons };
const struct WildPokemon SootopolisCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 35, 40, SPECIES_GYARADOS },
    { 35, 45, SPECIES_GYARADOS },
    { 5, 45, SPECIES_GYARADOS },
};

const struct WildPokemonInfo SootopolisCity_Sapphire_FishingMonsInfo = { 10, SootopolisCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon EverGrandeCity_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo EverGrandeCity_Sapphire_WaterMonsInfo = { 4, EverGrandeCity_Sapphire_WaterMons };
const struct WildPokemon EverGrandeCity_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_LUVDISC },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_LUVDISC },
    { 30, 35, SPECIES_WAILMER },
    { 30, 35, SPECIES_CORSOLA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo EverGrandeCity_Sapphire_FishingMonsInfo = { 10, EverGrandeCity_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MeteorFalls_1F_1R_Sapphire_LandMons[] =
{
    { 16, 16, SPECIES_ZUBAT },
    { 17, 17, SPECIES_ZUBAT },
    { 18, 18, SPECIES_ZUBAT },
    { 15, 15, SPECIES_ZUBAT },
    { 14, 14, SPECIES_ZUBAT },
    { 16, 16, SPECIES_LUNATONE },
    { 18, 18, SPECIES_LUNATONE },
    { 14, 14, SPECIES_LUNATONE },
    { 19, 19, SPECIES_ZUBAT },
    { 20, 20, SPECIES_ZUBAT },
    { 19, 19, SPECIES_ZUBAT },
    { 20, 20, SPECIES_ZUBAT },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Sapphire_LandMonsInfo = { 10, MeteorFalls_1F_1R_Sapphire_LandMons };
const struct WildPokemon MeteorFalls_1F_1R_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 25, 35, SPECIES_LUNATONE },
    { 15, 25, SPECIES_LUNATONE },
    { 5, 15, SPECIES_LUNATONE },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Sapphire_WaterMonsInfo = { 4, MeteorFalls_1F_1R_Sapphire_WaterMons };
const struct WildPokemon MeteorFalls_1F_1R_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo MeteorFalls_1F_1R_Sapphire_FishingMonsInfo = { 30, MeteorFalls_1F_1R_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MeteorFalls_1F_2R_Sapphire_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_LUNATONE },
    { 33, 33, SPECIES_LUNATONE },
    { 37, 37, SPECIES_LUNATONE },
    { 35, 35, SPECIES_GOLBAT },
    { 39, 39, SPECIES_LUNATONE },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Sapphire_LandMonsInfo = { 10, MeteorFalls_1F_2R_Sapphire_LandMons };
const struct WildPokemon MeteorFalls_1F_2R_Sapphire_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_LUNATONE },
    { 15, 25, SPECIES_LUNATONE },
    { 5, 15, SPECIES_LUNATONE },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Sapphire_WaterMonsInfo = { 4, MeteorFalls_1F_2R_Sapphire_WaterMons };
const struct WildPokemon MeteorFalls_1F_2R_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_1F_2R_Sapphire_FishingMonsInfo = { 30, MeteorFalls_1F_2R_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MeteorFalls_B1F_1R_Sapphire_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_LUNATONE },
    { 33, 33, SPECIES_LUNATONE },
    { 37, 37, SPECIES_LUNATONE },
    { 35, 35, SPECIES_GOLBAT },
    { 39, 39, SPECIES_LUNATONE },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Sapphire_LandMonsInfo = { 10, MeteorFalls_B1F_1R_Sapphire_LandMons };
const struct WildPokemon MeteorFalls_B1F_1R_Sapphire_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_LUNATONE },
    { 15, 25, SPECIES_LUNATONE },
    { 5, 15, SPECIES_LUNATONE },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Sapphire_WaterMonsInfo = { 4, MeteorFalls_B1F_1R_Sapphire_WaterMons };
const struct WildPokemon MeteorFalls_B1F_1R_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_B1F_1R_Sapphire_FishingMonsInfo = { 30, MeteorFalls_B1F_1R_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MeteorFalls_B1F_2R_Sapphire_LandMons[] =
{
    { 33, 33, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 30, 30, SPECIES_BAGON },
    { 35, 35, SPECIES_LUNATONE },
    { 35, 35, SPECIES_BAGON },
    { 37, 37, SPECIES_LUNATONE },
    { 25, 25, SPECIES_BAGON },
    { 39, 39, SPECIES_LUNATONE },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
    { 38, 38, SPECIES_GOLBAT },
    { 40, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Sapphire_LandMonsInfo = { 10, MeteorFalls_B1F_2R_Sapphire_LandMons };
const struct WildPokemon MeteorFalls_B1F_2R_Sapphire_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 25, 35, SPECIES_LUNATONE },
    { 15, 25, SPECIES_LUNATONE },
    { 5, 15, SPECIES_LUNATONE },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Sapphire_WaterMonsInfo = { 4, MeteorFalls_B1F_2R_Sapphire_WaterMons };
const struct WildPokemon MeteorFalls_B1F_2R_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo MeteorFalls_B1F_2R_Sapphire_FishingMonsInfo = { 30, MeteorFalls_B1F_2R_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon RusturfTunnel_Sapphire_LandMons[] =
{
    { 6, 6, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 6, 6, SPECIES_WHISMUR },
    { 6, 6, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 7, 7, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
    { 5, 5, SPECIES_WHISMUR },
    { 8, 8, SPECIES_WHISMUR },
};

const struct WildPokemonInfo RusturfTunnel_Sapphire_LandMonsInfo = { 10, RusturfTunnel_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon GraniteCave_1F_Sapphire_LandMons[] =
{
    { 7, 7, SPECIES_ZUBAT },
    { 8, 8, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ZUBAT },
    { 9, 9, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 6, 6, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_GEODUDE },
    { 8, 8, SPECIES_GEODUDE },
    { 6, 6, SPECIES_GEODUDE },
    { 9, 9, SPECIES_GEODUDE },
};

const struct WildPokemonInfo GraniteCave_1F_Sapphire_LandMonsInfo = { 10, GraniteCave_1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon GraniteCave_B1F_Sapphire_LandMons[] =
{
    { 9, 9, SPECIES_ZUBAT },
    { 10, 10, SPECIES_ARON },
    { 9, 9, SPECIES_ARON },
    { 11, 11, SPECIES_ARON },
    { 10, 10, SPECIES_ZUBAT },
    { 9, 9, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 11, 11, SPECIES_MAKUHITA },
    { 10, 10, SPECIES_SABLEYE },
    { 10, 10, SPECIES_SABLEYE },
    { 9, 9, SPECIES_SABLEYE },
    { 11, 11, SPECIES_SABLEYE },
};

const struct WildPokemonInfo GraniteCave_B1F_Sapphire_LandMonsInfo = { 10, GraniteCave_B1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon GraniteCave_B2F_Sapphire_LandMons[] =
{
    { 10, 10, SPECIES_ZUBAT },
    { 11, 11, SPECIES_ARON },
    { 10, 10, SPECIES_ARON },
    { 11, 11, SPECIES_ZUBAT },
    { 12, 12, SPECIES_ARON },
    { 10, 10, SPECIES_ABRA },
    { 10, 10, SPECIES_SABLEYE },
    { 11, 11, SPECIES_SABLEYE },
    { 12, 12, SPECIES_SABLEYE },
    { 10, 10, SPECIES_SABLEYE },
    { 12, 12, SPECIES_SABLEYE },
    { 10, 10, SPECIES_SABLEYE },
};

const struct WildPokemonInfo GraniteCave_B2F_Sapphire_LandMonsInfo = { 10, GraniteCave_B2F_Sapphire_LandMons };
const struct WildPokemon GraniteCave_B2F_Sapphire_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 10, 20, SPECIES_NOSEPASS },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo GraniteCave_B2F_Sapphire_RockSmashMonsInfo = { 20, GraniteCave_B2F_Sapphire_RockSmashMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon GraniteCave_StevensRoom_Sapphire_LandMons[] =
{
    { 7, 7, SPECIES_ZUBAT },
    { 8, 8, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ZUBAT },
    { 9, 9, SPECIES_MAKUHITA },
    { 8, 8, SPECIES_ABRA },
    { 10, 10, SPECIES_MAKUHITA },
    { 6, 6, SPECIES_MAKUHITA },
    { 7, 7, SPECIES_ARON },
    { 8, 8, SPECIES_ARON },
    { 7, 7, SPECIES_ARON },
    { 8, 8, SPECIES_ARON },
};

const struct WildPokemonInfo GraniteCave_StevensRoom_Sapphire_LandMonsInfo = { 10, GraniteCave_StevensRoom_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon PetalburgWoods_Sapphire_LandMons[] =
{
    { 5, 5, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_WURMPLE },
    { 5, 5, SPECIES_SHROOMISH },
    { 6, 6, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_SILCOON },
    { 5, 5, SPECIES_CASCOON },
    { 6, 6, SPECIES_WURMPLE },
    { 6, 6, SPECIES_SHROOMISH },
    { 5, 5, SPECIES_TAILLOW },
    { 5, 5, SPECIES_SLAKOTH },
    { 6, 6, SPECIES_TAILLOW },
    { 6, 6, SPECIES_SLAKOTH },
};

const struct WildPokemonInfo PetalburgWoods_Sapphire_LandMonsInfo = { 20, PetalburgWoods_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon JaggedPass_Sapphire_LandMons[] =
{
    { 21, 21, SPECIES_NUMEL },
    { 21, 21, SPECIES_NUMEL },
    { 21, 21, SPECIES_MACHOP },
    { 20, 20, SPECIES_NUMEL },
    { 20, 20, SPECIES_SPOINK },
    { 20, 20, SPECIES_MACHOP },
    { 21, 21, SPECIES_SPOINK },
    { 22, 22, SPECIES_MACHOP },
    { 22, 22, SPECIES_NUMEL },
    { 22, 22, SPECIES_SPOINK },
    { 22, 22, SPECIES_NUMEL },
    { 22, 22, SPECIES_SPOINK },
};

const struct WildPokemonInfo JaggedPass_Sapphire_LandMonsInfo = { 20, JaggedPass_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon FieryPath_Sapphire_LandMons[] =
{
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_GRIMER },
    { 16, 16, SPECIES_NUMEL },
    { 15, 15, SPECIES_MACHOP },
    { 15, 15, SPECIES_TORKOAL },
    { 15, 15, SPECIES_SLUGMA },
    { 16, 16, SPECIES_GRIMER },
    { 16, 16, SPECIES_MACHOP },
    { 14, 14, SPECIES_TORKOAL },
    { 16, 16, SPECIES_TORKOAL },
    { 14, 14, SPECIES_KOFFING },
    { 14, 14, SPECIES_KOFFING },
};

const struct WildPokemonInfo FieryPath_Sapphire_LandMonsInfo = { 10, FieryPath_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_1F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_1F_Sapphire_LandMonsInfo = { 10, MtPyre_1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_2F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_2F_Sapphire_LandMonsInfo = { 10, MtPyre_2F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_3F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
};

const struct WildPokemonInfo MtPyre_3F_Sapphire_LandMonsInfo = { 10, MtPyre_3F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_4F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 27, 27, SPECIES_DUSKULL },
    { 27, 27, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_4F_Sapphire_LandMonsInfo = { 10, MtPyre_4F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_5F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 27, 27, SPECIES_DUSKULL },
    { 27, 27, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_5F_Sapphire_LandMonsInfo = { 10, MtPyre_5F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_6F_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 28, 28, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 23, 23, SPECIES_SHUPPET },
    { 22, 22, SPECIES_SHUPPET },
    { 27, 27, SPECIES_DUSKULL },
    { 27, 27, SPECIES_DUSKULL },
    { 25, 25, SPECIES_DUSKULL },
    { 29, 29, SPECIES_DUSKULL },
};

const struct WildPokemonInfo MtPyre_6F_Sapphire_LandMonsInfo = { 10, MtPyre_6F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_Exterior_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_SHUPPET },
    { 27, 27, SPECIES_MEDITITE },
    { 28, 28, SPECIES_SHUPPET },
    { 29, 29, SPECIES_MEDITITE },
    { 29, 29, SPECIES_SHUPPET },
    { 27, 27, SPECIES_VULPIX },
    { 29, 29, SPECIES_VULPIX },
    { 25, 25, SPECIES_VULPIX },
    { 27, 27, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
};

const struct WildPokemonInfo MtPyre_Exterior_Sapphire_LandMonsInfo = { 10, MtPyre_Exterior_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon MtPyre_Summit_Sapphire_LandMons[] =
{
    { 28, 28, SPECIES_SHUPPET },
    { 29, 29, SPECIES_SHUPPET },
    { 27, 27, SPECIES_SHUPPET },
    { 26, 26, SPECIES_SHUPPET },
    { 30, 30, SPECIES_SHUPPET },
    { 25, 25, SPECIES_SHUPPET },
    { 24, 24, SPECIES_SHUPPET },
    { 28, 28, SPECIES_DUSKULL },
    { 26, 26, SPECIES_DUSKULL },
    { 30, 30, SPECIES_DUSKULL },
    { 28, 28, SPECIES_CHIMECHO },
    { 28, 28, SPECIES_CHIMECHO },
};

const struct WildPokemonInfo MtPyre_Summit_Sapphire_LandMonsInfo = { 10, MtPyre_Summit_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Entrance_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Entrance_Sapphire_WaterMonsInfo = { 4, SeafloorCavern_Entrance_Sapphire_WaterMons };
const struct WildPokemon SeafloorCavern_Entrance_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Entrance_Sapphire_FishingMonsInfo = { 10, SeafloorCavern_Entrance_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room1_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room1_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room1_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room2_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room2_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room2_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room3_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room3_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room3_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room4_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room4_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room4_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room5_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room5_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room5_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room6_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room6_Sapphire_LandMons };
const struct WildPokemon SeafloorCavern_Room6_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Sapphire_WaterMonsInfo = { 4, SeafloorCavern_Room6_Sapphire_WaterMons };
const struct WildPokemon SeafloorCavern_Room6_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Room6_Sapphire_FishingMonsInfo = { 10, SeafloorCavern_Room6_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room7_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room7_Sapphire_LandMons };
const struct WildPokemon SeafloorCavern_Room7_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_ZUBAT },
    { 30, 35, SPECIES_GOLBAT },
    { 30, 35, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Sapphire_WaterMonsInfo = { 4, SeafloorCavern_Room7_Sapphire_WaterMons };
const struct WildPokemon SeafloorCavern_Room7_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo SeafloorCavern_Room7_Sapphire_FishingMonsInfo = { 10, SeafloorCavern_Room7_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SeafloorCavern_Room8_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo SeafloorCavern_Room8_Sapphire_LandMonsInfo = { 4, SeafloorCavern_Room8_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon CaveOfOrigin_Entrance_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 33, 33, SPECIES_ZUBAT },
    { 28, 28, SPECIES_ZUBAT },
    { 29, 29, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 35, 35, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_Entrance_Sapphire_LandMonsInfo = { 4, CaveOfOrigin_Entrance_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon CaveOfOrigin_1F_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SABLEYE },
    { 32, 32, SPECIES_SABLEYE },
    { 34, 34, SPECIES_SABLEYE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_1F_Sapphire_LandMonsInfo = { 4, CaveOfOrigin_1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon CaveOfOrigin_B1F_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SABLEYE },
    { 32, 32, SPECIES_SABLEYE },
    { 34, 34, SPECIES_SABLEYE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B1F_Sapphire_LandMonsInfo = { 4, CaveOfOrigin_B1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon CaveOfOrigin_B2F_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SABLEYE },
    { 32, 32, SPECIES_SABLEYE },
    { 34, 34, SPECIES_SABLEYE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B2F_Sapphire_LandMonsInfo = { 4, CaveOfOrigin_B2F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon CaveOfOrigin_B3F_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_ZUBAT },
    { 31, 31, SPECIES_ZUBAT },
    { 32, 32, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SABLEYE },
    { 32, 32, SPECIES_SABLEYE },
    { 34, 34, SPECIES_SABLEYE },
    { 33, 33, SPECIES_ZUBAT },
    { 34, 34, SPECIES_ZUBAT },
    { 34, 34, SPECIES_GOLBAT },
    { 35, 35, SPECIES_GOLBAT },
    { 33, 33, SPECIES_GOLBAT },
    { 36, 36, SPECIES_GOLBAT },
};

const struct WildPokemonInfo CaveOfOrigin_B3F_Sapphire_LandMonsInfo = { 4, CaveOfOrigin_B3F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon VictoryRoad_1F_Sapphire_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_HARIYAMA },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_LOUDRED },
    { 36, 36, SPECIES_ZUBAT },
    { 36, 36, SPECIES_MAKUHITA },
    { 38, 38, SPECIES_GOLBAT },
    { 38, 38, SPECIES_HARIYAMA },
    { 36, 36, SPECIES_ARON },
    { 36, 36, SPECIES_WHISMUR },
    { 36, 36, SPECIES_ARON },
    { 36, 36, SPECIES_WHISMUR },
};

const struct WildPokemonInfo VictoryRoad_1F_Sapphire_LandMonsInfo = { 10, VictoryRoad_1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon VictoryRoad_B1F_Sapphire_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_HARIYAMA },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_MEDICHAM },
    { 38, 38, SPECIES_GOLBAT },
    { 38, 38, SPECIES_HARIYAMA },
    { 42, 42, SPECIES_GOLBAT },
    { 42, 42, SPECIES_HARIYAMA },
    { 42, 42, SPECIES_LAIRON },
    { 38, 38, SPECIES_MEDITITE },
    { 42, 42, SPECIES_LAIRON },
    { 38, 38, SPECIES_MEDITITE },
};

const struct WildPokemonInfo VictoryRoad_B1F_Sapphire_LandMonsInfo = { 10, VictoryRoad_B1F_Sapphire_LandMons };
const struct WildPokemon VictoryRoad_B1F_Sapphire_RockSmashMons[] =
{
    { 30, 40, SPECIES_GRAVELER },
    { 30, 40, SPECIES_GEODUDE },
    { 35, 40, SPECIES_GRAVELER },
    { 35, 40, SPECIES_GRAVELER },
    { 35, 40, SPECIES_GRAVELER },
};

const struct WildPokemonInfo VictoryRoad_B1F_Sapphire_RockSmashMonsInfo = { 20, VictoryRoad_B1F_Sapphire_RockSmashMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon VictoryRoad_B2F_Sapphire_LandMons[] =
{
    { 40, 40, SPECIES_GOLBAT },
    { 40, 40, SPECIES_SABLEYE },
    { 40, 40, SPECIES_LAIRON },
    { 40, 40, SPECIES_MEDICHAM },
    { 42, 42, SPECIES_GOLBAT },
    { 42, 42, SPECIES_SABLEYE },
    { 44, 44, SPECIES_GOLBAT },
    { 44, 44, SPECIES_SABLEYE },
    { 42, 42, SPECIES_LAIRON },
    { 42, 42, SPECIES_MEDICHAM },
    { 44, 44, SPECIES_LAIRON },
    { 44, 44, SPECIES_MEDICHAM },
};

const struct WildPokemonInfo VictoryRoad_B2F_Sapphire_LandMonsInfo = { 10, VictoryRoad_B2F_Sapphire_LandMons };
const struct WildPokemon VictoryRoad_B2F_Sapphire_WaterMons[] =
{
    { 30, 35, SPECIES_GOLBAT },
    { 25, 30, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
    { 35, 40, SPECIES_GOLBAT },
};

const struct WildPokemonInfo VictoryRoad_B2F_Sapphire_WaterMonsInfo = { 4, VictoryRoad_B2F_Sapphire_WaterMons };
const struct WildPokemon VictoryRoad_B2F_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 30, 35, SPECIES_WHISCASH },
    { 35, 40, SPECIES_WHISCASH },
    { 40, 45, SPECIES_WHISCASH },
};

const struct WildPokemonInfo VictoryRoad_B2F_Sapphire_FishingMonsInfo = { 30, VictoryRoad_B2F_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Sapphire_LandMonsInfo = { 10, ShoalCave_LowTideEntranceRoom_Sapphire_LandMons };
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 35, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Sapphire_WaterMonsInfo = { 4, ShoalCave_LowTideEntranceRoom_Sapphire_WaterMons };
const struct WildPokemon ShoalCave_LowTideEntranceRoom_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo ShoalCave_LowTideEntranceRoom_Sapphire_FishingMonsInfo = { 10, ShoalCave_LowTideEntranceRoom_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon ShoalCave_LowTideInnerRoom_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Sapphire_LandMonsInfo = { 10, ShoalCave_LowTideInnerRoom_Sapphire_LandMons };
const struct WildPokemon ShoalCave_LowTideInnerRoom_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_ZUBAT },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 30, SPECIES_SPHEAL },
    { 25, 35, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Sapphire_WaterMonsInfo = { 4, ShoalCave_LowTideInnerRoom_Sapphire_WaterMons };
const struct WildPokemon ShoalCave_LowTideInnerRoom_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo ShoalCave_LowTideInnerRoom_Sapphire_FishingMonsInfo = { 10, ShoalCave_LowTideInnerRoom_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon ShoalCave_LowTideStairsRoom_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideStairsRoom_Sapphire_LandMonsInfo = { 10, ShoalCave_LowTideStairsRoom_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon ShoalCave_LowTideLowerRoom_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 32, 32, SPECIES_ZUBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
    { 32, 32, SPECIES_GOLBAT },
    { 32, 32, SPECIES_SPHEAL },
};

const struct WildPokemonInfo ShoalCave_LowTideLowerRoom_Sapphire_LandMonsInfo = { 10, ShoalCave_LowTideLowerRoom_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon ShoalCave_LowTideIceRoom_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZUBAT },
    { 26, 26, SPECIES_SPHEAL },
    { 28, 28, SPECIES_ZUBAT },
    { 28, 28, SPECIES_SPHEAL },
    { 30, 30, SPECIES_ZUBAT },
    { 30, 30, SPECIES_SPHEAL },
    { 26, 26, SPECIES_SNORUNT },
    { 32, 32, SPECIES_SPHEAL },
    { 30, 30, SPECIES_GOLBAT },
    { 28, 28, SPECIES_SNORUNT },
    { 32, 32, SPECIES_GOLBAT },
    { 30, 30, SPECIES_SNORUNT },
};

const struct WildPokemonInfo ShoalCave_LowTideIceRoom_Sapphire_LandMonsInfo = { 10, ShoalCave_LowTideIceRoom_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon NewMauville_Entrance_Sapphire_LandMons[] =
{
    { 24, 24, SPECIES_VOLTORB },
    { 24, 24, SPECIES_MAGNEMITE },
    { 25, 25, SPECIES_VOLTORB },
    { 25, 25, SPECIES_MAGNEMITE },
    { 23, 23, SPECIES_VOLTORB },
    { 23, 23, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_VOLTORB },
    { 26, 26, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
};

const struct WildPokemonInfo NewMauville_Entrance_Sapphire_LandMonsInfo = { 10, NewMauville_Entrance_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon NewMauville_Inside_Sapphire_LandMons[] =
{
    { 24, 24, SPECIES_VOLTORB },
    { 24, 24, SPECIES_MAGNEMITE },
    { 25, 25, SPECIES_VOLTORB },
    { 25, 25, SPECIES_MAGNEMITE },
    { 23, 23, SPECIES_VOLTORB },
    { 23, 23, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_VOLTORB },
    { 26, 26, SPECIES_MAGNEMITE },
    { 22, 22, SPECIES_VOLTORB },
    { 22, 22, SPECIES_MAGNEMITE },
    { 26, 26, SPECIES_ELECTRODE },
    { 26, 26, SPECIES_MAGNETON },
};

const struct WildPokemonInfo NewMauville_Inside_Sapphire_LandMonsInfo = { 10, NewMauville_Inside_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon AbandonedShip_Rooms_B1F_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_Rooms_B1F_Sapphire_WaterMonsInfo = { 4, AbandonedShip_Rooms_B1F_Sapphire_WaterMons };
const struct WildPokemon AbandonedShip_Rooms_B1F_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_TENTACOOL },
    { 25, 30, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
    { 25, 30, SPECIES_TENTACRUEL },
    { 20, 25, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_Rooms_B1F_Sapphire_FishingMonsInfo = { 20, AbandonedShip_Rooms_B1F_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon AbandonedShip_HiddenFloorCorridors_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 5, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_HiddenFloorCorridors_Sapphire_WaterMonsInfo = { 4, AbandonedShip_HiddenFloorCorridors_Sapphire_WaterMons };
const struct WildPokemon AbandonedShip_HiddenFloorCorridors_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_TENTACOOL },
    { 25, 30, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACOOL },
    { 30, 35, SPECIES_TENTACRUEL },
    { 25, 30, SPECIES_TENTACRUEL },
    { 20, 25, SPECIES_TENTACRUEL },
};

const struct WildPokemonInfo AbandonedShip_HiddenFloorCorridors_Sapphire_FishingMonsInfo = { 20, AbandonedShip_HiddenFloorCorridors_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SkyPillar_1F_Sapphire_LandMons[] =
{
    { 48, 48, SPECIES_SABLEYE },
    { 48, 48, SPECIES_GOLBAT },
    { 50, 50, SPECIES_GOLBAT },
    { 50, 50, SPECIES_SABLEYE },
    { 48, 48, SPECIES_CLAYDOL },
    { 48, 48, SPECIES_BANETTE },
    { 50, 50, SPECIES_BANETTE },
    { 49, 49, SPECIES_CLAYDOL },
    { 47, 47, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 47, 47, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
};

const struct WildPokemonInfo SkyPillar_1F_Sapphire_LandMonsInfo = { 10, SkyPillar_1F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SkyPillar_3F_Sapphire_LandMons[] =
{
    { 51, 51, SPECIES_SABLEYE },
    { 51, 51, SPECIES_GOLBAT },
    { 53, 53, SPECIES_GOLBAT },
    { 53, 53, SPECIES_SABLEYE },
    { 51, 51, SPECIES_CLAYDOL },
    { 51, 51, SPECIES_BANETTE },
    { 53, 53, SPECIES_BANETTE },
    { 52, 52, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 53, 53, SPECIES_CLAYDOL },
    { 50, 50, SPECIES_CLAYDOL },
    { 53, 53, SPECIES_CLAYDOL },
};

const struct WildPokemonInfo SkyPillar_3F_Sapphire_LandMonsInfo = { 10, SkyPillar_3F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SkyPillar_5F_Sapphire_LandMons[] =
{
    { 54, 54, SPECIES_SABLEYE },
    { 54, 54, SPECIES_GOLBAT },
    { 56, 56, SPECIES_GOLBAT },
    { 56, 56, SPECIES_SABLEYE },
    { 54, 54, SPECIES_CLAYDOL },
    { 54, 54, SPECIES_BANETTE },
    { 56, 56, SPECIES_BANETTE },
    { 55, 55, SPECIES_CLAYDOL },
    { 56, 56, SPECIES_CLAYDOL },
    { 57, 57, SPECIES_ALTARIA },
    { 54, 54, SPECIES_ALTARIA },
    { 60, 60, SPECIES_ALTARIA },
};

const struct WildPokemonInfo SkyPillar_5F_Sapphire_LandMonsInfo = { 10, SkyPillar_5F_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route101_Sapphire_LandMons[] =
{
    { 2, 2, SPECIES_WURMPLE },
    { 2, 2, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_WURMPLE },
    { 3, 3, SPECIES_WURMPLE },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_WURMPLE },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_POOCHYENA },
    { 2, 2, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
};

const struct WildPokemonInfo Route101_Sapphire_LandMonsInfo = { 20, Route101_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route102_Sapphire_LandMons[] =
{
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_WURMPLE },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_WURMPLE },
    { 3, 3, SPECIES_LOTAD },
    { 4, 4, SPECIES_LOTAD },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_RALTS },
    { 4, 4, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route102_Sapphire_LandMonsInfo = { 20, Route102_Sapphire_LandMons };
const struct WildPokemon Route102_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route102_Sapphire_WaterMonsInfo = { 4, Route102_Sapphire_WaterMons };
const struct WildPokemon Route102_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo Route102_Sapphire_FishingMonsInfo = { 30, Route102_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route103_Sapphire_LandMons[] =
{
    { 2, 2, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 3, 3, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 2, 2, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_POOCHYENA },
    { 4, 4, SPECIES_POOCHYENA },
    { 3, 3, SPECIES_WINGULL },
    { 3, 3, SPECIES_WINGULL },
    { 2, 2, SPECIES_WINGULL },
    { 4, 4, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route103_Sapphire_LandMonsInfo = { 20, Route103_Sapphire_LandMons };
const struct WildPokemon Route103_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route103_Sapphire_WaterMonsInfo = { 4, Route103_Sapphire_WaterMons };
const struct WildPokemon Route103_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route103_Sapphire_FishingMonsInfo = { 30, Route103_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route104_Sapphire_LandMons[] =
{
    { 4, 4, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_WURMPLE },
    { 5, 5, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_WURMPLE },
    { 4, 4, SPECIES_ZIGZAGOON },
    { 5, 5, SPECIES_ZIGZAGOON },
    { 4, 4, SPECIES_TAILLOW },
    { 5, 5, SPECIES_TAILLOW },
    { 4, 4, SPECIES_WINGULL },
    { 4, 4, SPECIES_WINGULL },
    { 3, 3, SPECIES_WINGULL },
    { 5, 5, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route104_Sapphire_LandMonsInfo = { 20, Route104_Sapphire_LandMons };
const struct WildPokemon Route104_Sapphire_WaterMons[] =
{
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route104_Sapphire_WaterMonsInfo = { 4, Route104_Sapphire_WaterMons };
const struct WildPokemon Route104_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_MAGIKARP },
    { 25, 30, SPECIES_MAGIKARP },
    { 30, 35, SPECIES_MAGIKARP },
    { 20, 25, SPECIES_MAGIKARP },
    { 35, 40, SPECIES_MAGIKARP },
    { 40, 45, SPECIES_MAGIKARP },
};

const struct WildPokemonInfo Route104_Sapphire_FishingMonsInfo = { 30, Route104_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route105_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route105_Sapphire_WaterMonsInfo = { 4, Route105_Sapphire_WaterMons };
const struct WildPokemon Route105_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route105_Sapphire_FishingMonsInfo = { 30, Route105_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route106_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route106_Sapphire_WaterMonsInfo = { 4, Route106_Sapphire_WaterMons };
const struct WildPokemon Route106_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route106_Sapphire_FishingMonsInfo = { 30, Route106_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route107_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route107_Sapphire_WaterMonsInfo = { 4, Route107_Sapphire_WaterMons };
const struct WildPokemon Route107_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route107_Sapphire_FishingMonsInfo = { 30, Route107_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route108_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route108_Sapphire_WaterMonsInfo = { 4, Route108_Sapphire_WaterMons };
const struct WildPokemon Route108_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route108_Sapphire_FishingMonsInfo = { 30, Route108_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route109_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route109_Sapphire_WaterMonsInfo = { 4, Route109_Sapphire_WaterMons };
const struct WildPokemon Route109_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route109_Sapphire_FishingMonsInfo = { 30, Route109_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route110_Sapphire_LandMons[] =
{
    { 12, 12, SPECIES_ZIGZAGOON },
    { 12, 12, SPECIES_ELECTRIKE },
    { 12, 12, SPECIES_GULPIN },
    { 13, 13, SPECIES_ELECTRIKE },
    { 13, 13, SPECIES_PLUSLE },
    { 13, 13, SPECIES_ODDISH },
    { 13, 13, SPECIES_PLUSLE },
    { 13, 13, SPECIES_GULPIN },
    { 12, 12, SPECIES_WINGULL },
    { 12, 12, SPECIES_WINGULL },
    { 12, 12, SPECIES_MINUN },
    { 13, 13, SPECIES_MINUN },
};

const struct WildPokemonInfo Route110_Sapphire_LandMonsInfo = { 20, Route110_Sapphire_LandMons };
const struct WildPokemon Route110_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route110_Sapphire_WaterMonsInfo = { 4, Route110_Sapphire_WaterMons };
const struct WildPokemon Route110_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route110_Sapphire_FishingMonsInfo = { 30, Route110_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route111_Sapphire_LandMons[] =
{
    { 20, 20, SPECIES_SANDSHREW },
    { 20, 20, SPECIES_TRAPINCH },
    { 21, 21, SPECIES_SANDSHREW },
    { 21, 21, SPECIES_TRAPINCH },
    { 19, 19, SPECIES_CACNEA },
    { 21, 21, SPECIES_CACNEA },
    { 19, 19, SPECIES_SANDSHREW },
    { 19, 19, SPECIES_TRAPINCH },
    { 20, 20, SPECIES_BALTOY },
    { 20, 20, SPECIES_BALTOY },
    { 22, 22, SPECIES_BALTOY },
    { 22, 22, SPECIES_BALTOY },
};

const struct WildPokemonInfo Route111_Sapphire_LandMonsInfo = { 10, Route111_Sapphire_LandMons };
const struct WildPokemon Route111_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route111_Sapphire_WaterMonsInfo = { 4, Route111_Sapphire_WaterMons };
const struct WildPokemon Route111_Sapphire_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo Route111_Sapphire_RockSmashMonsInfo = { 20, Route111_Sapphire_RockSmashMons };
const struct WildPokemon Route111_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route111_Sapphire_FishingMonsInfo = { 30, Route111_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route112_Sapphire_LandMons[] =
{
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_NUMEL },
    { 15, 15, SPECIES_MACHOP },
    { 14, 14, SPECIES_NUMEL },
    { 14, 14, SPECIES_NUMEL },
    { 14, 14, SPECIES_MACHOP },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_MACHOP },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
    { 16, 16, SPECIES_NUMEL },
};

const struct WildPokemonInfo Route112_Sapphire_LandMonsInfo = { 20, Route112_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route113_Sapphire_LandMons[] =
{
    { 15, 15, SPECIES_SPINDA },
    { 15, 15, SPECIES_SPINDA },
    { 15, 15, SPECIES_SANDSHREW },
    { 14, 14, SPECIES_SPINDA },
    { 14, 14, SPECIES_SPINDA },
    { 14, 14, SPECIES_SANDSHREW },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SANDSHREW },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SKARMORY },
    { 16, 16, SPECIES_SPINDA },
    { 16, 16, SPECIES_SKARMORY },
};

const struct WildPokemonInfo Route113_Sapphire_LandMonsInfo = { 20, Route113_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route114_Sapphire_LandMons[] =
{
    { 16, 16, SPECIES_SWABLU },
    { 16, 16, SPECIES_LOTAD },
    { 17, 17, SPECIES_SWABLU },
    { 15, 15, SPECIES_SWABLU },
    { 15, 15, SPECIES_LOTAD },
    { 16, 16, SPECIES_SEVIPER },
    { 16, 16, SPECIES_LOMBRE },
    { 18, 18, SPECIES_LOMBRE },
    { 17, 17, SPECIES_SEVIPER },
    { 15, 15, SPECIES_SEVIPER },
    { 17, 17, SPECIES_SEVIPER },
    { 15, 15, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route114_Sapphire_LandMonsInfo = { 20, Route114_Sapphire_LandMons };
const struct WildPokemon Route114_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route114_Sapphire_WaterMonsInfo = { 4, Route114_Sapphire_WaterMons };
const struct WildPokemon Route114_Sapphire_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
};

const struct WildPokemonInfo Route114_Sapphire_RockSmashMonsInfo = { 20, Route114_Sapphire_RockSmashMons };
const struct WildPokemon Route114_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route114_Sapphire_FishingMonsInfo = { 30, Route114_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route115_Sapphire_LandMons[] =
{
    { 23, 23, SPECIES_SWABLU },
    { 23, 23, SPECIES_TAILLOW },
    { 25, 25, SPECIES_SWABLU },
    { 24, 24, SPECIES_TAILLOW },
    { 25, 25, SPECIES_TAILLOW },
    { 25, 25, SPECIES_SWELLOW },
    { 24, 24, SPECIES_JIGGLYPUFF },
    { 25, 25, SPECIES_JIGGLYPUFF },
    { 24, 24, SPECIES_WINGULL },
    { 24, 24, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 25, 25, SPECIES_WINGULL },
};

const struct WildPokemonInfo Route115_Sapphire_LandMonsInfo = { 20, Route115_Sapphire_LandMons };
const struct WildPokemon Route115_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route115_Sapphire_WaterMonsInfo = { 4, Route115_Sapphire_WaterMons };
const struct WildPokemon Route115_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route115_Sapphire_FishingMonsInfo = { 30, Route115_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route116_Sapphire_LandMons[] =
{
    { 6, 6, SPECIES_ZIGZAGOON },
    { 6, 6, SPECIES_WHISMUR },
    { 6, 6, SPECIES_NINCADA },
    { 7, 7, SPECIES_WHISMUR },
    { 7, 7, SPECIES_NINCADA },
    { 6, 6, SPECIES_TAILLOW },
    { 7, 7, SPECIES_TAILLOW },
    { 8, 8, SPECIES_TAILLOW },
    { 7, 7, SPECIES_ZIGZAGOON },
    { 8, 8, SPECIES_ZIGZAGOON },
    { 7, 7, SPECIES_SKITTY },
    { 8, 8, SPECIES_SKITTY },
};

const struct WildPokemonInfo Route116_Sapphire_LandMonsInfo = { 20, Route116_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route117_Sapphire_LandMons[] =
{
    { 13, 13, SPECIES_ZIGZAGOON },
    { 13, 13, SPECIES_ROSELIA },
    { 14, 14, SPECIES_ZIGZAGOON },
    { 14, 14, SPECIES_ROSELIA },
    { 13, 13, SPECIES_MARILL },
    { 13, 13, SPECIES_ODDISH },
    { 13, 13, SPECIES_VOLBEAT },
    { 13, 13, SPECIES_VOLBEAT },
    { 14, 14, SPECIES_VOLBEAT },
    { 14, 14, SPECIES_VOLBEAT },
    { 13, 13, SPECIES_ILLUMISE },
    { 13, 13, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route117_Sapphire_LandMonsInfo = { 20, Route117_Sapphire_LandMons };
const struct WildPokemon Route117_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route117_Sapphire_WaterMonsInfo = { 4, Route117_Sapphire_WaterMons };
const struct WildPokemon Route117_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_CORPHISH },
    { 25, 30, SPECIES_CORPHISH },
    { 30, 35, SPECIES_CORPHISH },
    { 20, 25, SPECIES_CORPHISH },
    { 35, 40, SPECIES_CORPHISH },
    { 40, 45, SPECIES_CORPHISH },
};

const struct WildPokemonInfo Route117_Sapphire_FishingMonsInfo = { 30, Route117_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route118_Sapphire_LandMons[] =
{
    { 24, 24, SPECIES_ZIGZAGOON },
    { 24, 24, SPECIES_ELECTRIKE },
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_ELECTRIKE },
    { 26, 26, SPECIES_LINOONE },
    { 26, 26, SPECIES_MANECTRIC },
    { 25, 25, SPECIES_WINGULL },
    { 25, 25, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route118_Sapphire_LandMonsInfo = { 20, Route118_Sapphire_LandMons };
const struct WildPokemon Route118_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route118_Sapphire_WaterMonsInfo = { 4, Route118_Sapphire_WaterMons };
const struct WildPokemon Route118_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_CARVANHA },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_CARVANHA },
    { 20, 25, SPECIES_CARVANHA },
    { 35, 40, SPECIES_CARVANHA },
    { 40, 45, SPECIES_CARVANHA },
};

const struct WildPokemonInfo Route118_Sapphire_FishingMonsInfo = { 30, Route118_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route119_Sapphire_LandMons[] =
{
    { 25, 25, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_LINOONE },
    { 27, 27, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 24, 24, SPECIES_ODDISH },
    { 25, 25, SPECIES_TROPIUS },
    { 26, 26, SPECIES_TROPIUS },
    { 27, 27, SPECIES_TROPIUS },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route119_Sapphire_LandMonsInfo = { 15, Route119_Sapphire_LandMons };
const struct WildPokemon Route119_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route119_Sapphire_WaterMonsInfo = { 4, Route119_Sapphire_WaterMons };
const struct WildPokemon Route119_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_CARVANHA },
    { 25, 30, SPECIES_CARVANHA },
    { 30, 35, SPECIES_CARVANHA },
    { 20, 25, SPECIES_CARVANHA },
    { 35, 40, SPECIES_CARVANHA },
    { 40, 45, SPECIES_CARVANHA },
};

const struct WildPokemonInfo Route119_Sapphire_FishingMonsInfo = { 30, Route119_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route120_Sapphire_LandMons[] =
{
    { 25, 25, SPECIES_ZIGZAGOON },
    { 25, 25, SPECIES_LINOONE },
    { 27, 27, SPECIES_LINOONE },
    { 25, 25, SPECIES_ODDISH },
    { 25, 25, SPECIES_MARILL },
    { 26, 26, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 27, 27, SPECIES_MARILL },
    { 25, 25, SPECIES_ABSOL },
    { 27, 27, SPECIES_ABSOL },
    { 25, 25, SPECIES_KECLEON },
    { 25, 25, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route120_Sapphire_LandMonsInfo = { 20, Route120_Sapphire_LandMons };
const struct WildPokemon Route120_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_MARILL },
    { 10, 20, SPECIES_MARILL },
    { 30, 35, SPECIES_MARILL },
    { 5, 10, SPECIES_MARILL },
    { 20, 30, SPECIES_SURSKIT },
};

const struct WildPokemonInfo Route120_Sapphire_WaterMonsInfo = { 4, Route120_Sapphire_WaterMons };
const struct WildPokemon Route120_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_BARBOACH },
    { 25, 30, SPECIES_BARBOACH },
    { 30, 35, SPECIES_BARBOACH },
    { 20, 25, SPECIES_BARBOACH },
    { 35, 40, SPECIES_BARBOACH },
    { 40, 45, SPECIES_BARBOACH },
};

const struct WildPokemonInfo Route120_Sapphire_FishingMonsInfo = { 30, Route120_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route121_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_SHUPPET },
    { 26, 26, SPECIES_LINOONE },
    { 28, 28, SPECIES_SHUPPET },
    { 28, 28, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 28, 28, SPECIES_ODDISH },
    { 28, 28, SPECIES_GLOOM },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route121_Sapphire_LandMonsInfo = { 20, Route121_Sapphire_LandMons };
const struct WildPokemon Route121_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route121_Sapphire_WaterMonsInfo = { 4, Route121_Sapphire_WaterMons };
const struct WildPokemon Route121_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route121_Sapphire_FishingMonsInfo = { 30, Route121_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route122_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route122_Sapphire_WaterMonsInfo = { 4, Route122_Sapphire_WaterMons };
const struct WildPokemon Route122_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route122_Sapphire_FishingMonsInfo = { 30, Route122_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route123_Sapphire_LandMons[] =
{
    { 26, 26, SPECIES_ZIGZAGOON },
    { 26, 26, SPECIES_SHUPPET },
    { 26, 26, SPECIES_LINOONE },
    { 28, 28, SPECIES_SHUPPET },
    { 28, 28, SPECIES_LINOONE },
    { 26, 26, SPECIES_ODDISH },
    { 28, 28, SPECIES_ODDISH },
    { 28, 28, SPECIES_GLOOM },
    { 26, 26, SPECIES_WINGULL },
    { 27, 27, SPECIES_WINGULL },
    { 28, 28, SPECIES_WINGULL },
    { 25, 25, SPECIES_KECLEON },
};

const struct WildPokemonInfo Route123_Sapphire_LandMonsInfo = { 20, Route123_Sapphire_LandMons };
const struct WildPokemon Route123_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route123_Sapphire_WaterMonsInfo = { 4, Route123_Sapphire_WaterMons };
const struct WildPokemon Route123_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route123_Sapphire_FishingMonsInfo = { 30, Route123_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route124_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route124_Sapphire_WaterMonsInfo = { 4, Route124_Sapphire_WaterMons };
const struct WildPokemon Route124_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route124_Sapphire_FishingMonsInfo = { 30, Route124_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route125_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route125_Sapphire_WaterMonsInfo = { 4, Route125_Sapphire_WaterMons };
const struct WildPokemon Route125_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route125_Sapphire_FishingMonsInfo = { 30, Route125_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route126_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route126_Sapphire_WaterMonsInfo = { 4, Route126_Sapphire_WaterMons };
const struct WildPokemon Route126_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route126_Sapphire_FishingMonsInfo = { 30, Route126_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route127_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route127_Sapphire_WaterMonsInfo = { 4, Route127_Sapphire_WaterMons };
const struct WildPokemon Route127_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route127_Sapphire_FishingMonsInfo = { 30, Route127_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route128_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route128_Sapphire_WaterMonsInfo = { 4, Route128_Sapphire_WaterMons };
const struct WildPokemon Route128_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_LUVDISC },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_LUVDISC },
    { 30, 35, SPECIES_WAILMER },
    { 30, 35, SPECIES_CORSOLA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route128_Sapphire_FishingMonsInfo = { 30, Route128_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route129_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_WAILORD },
};

const struct WildPokemonInfo Route129_Sapphire_WaterMonsInfo = { 4, Route129_Sapphire_WaterMons };
const struct WildPokemon Route129_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route129_Sapphire_FishingMonsInfo = { 30, Route129_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route130_Sapphire_LandMons[] =
{
    { 30, 30, SPECIES_WYNAUT },
    { 35, 35, SPECIES_WYNAUT },
    { 25, 25, SPECIES_WYNAUT },
    { 40, 40, SPECIES_WYNAUT },
    { 20, 20, SPECIES_WYNAUT },
    { 45, 45, SPECIES_WYNAUT },
    { 15, 15, SPECIES_WYNAUT },
    { 50, 50, SPECIES_WYNAUT },
    { 10, 10, SPECIES_WYNAUT },
    { 5, 5, SPECIES_WYNAUT },
    { 10, 10, SPECIES_WYNAUT },
    { 5, 5, SPECIES_WYNAUT },
};

const struct WildPokemonInfo Route130_Sapphire_LandMonsInfo = { 20, Route130_Sapphire_LandMons };
const struct WildPokemon Route130_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route130_Sapphire_WaterMonsInfo = { 4, Route130_Sapphire_WaterMons };
const struct WildPokemon Route130_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route130_Sapphire_FishingMonsInfo = { 30, Route130_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route131_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route131_Sapphire_WaterMonsInfo = { 4, Route131_Sapphire_WaterMons };
const struct WildPokemon Route131_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route131_Sapphire_FishingMonsInfo = { 30, Route131_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route132_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route132_Sapphire_WaterMonsInfo = { 4, Route132_Sapphire_WaterMons };
const struct WildPokemon Route132_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route132_Sapphire_FishingMonsInfo = { 30, Route132_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route133_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route133_Sapphire_WaterMonsInfo = { 4, Route133_Sapphire_WaterMons };
const struct WildPokemon Route133_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route133_Sapphire_FishingMonsInfo = { 30, Route133_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Route134_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo Route134_Sapphire_WaterMonsInfo = { 4, Route134_Sapphire_WaterMons };
const struct WildPokemon Route134_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_HORSEA },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo Route134_Sapphire_FishingMonsInfo = { 30, Route134_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SafariZone_Northwest_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_RHYHORN },
    { 27, 27, SPECIES_ODDISH },
    { 29, 29, SPECIES_RHYHORN },
    { 29, 29, SPECIES_ODDISH },
    { 27, 27, SPECIES_DODUO },
    { 29, 29, SPECIES_GLOOM },
    { 31, 31, SPECIES_GLOOM },
    { 29, 29, SPECIES_DODUO },
    { 29, 29, SPECIES_DODRIO },
    { 27, 27, SPECIES_PINSIR },
    { 31, 31, SPECIES_DODRIO },
    { 29, 29, SPECIES_PINSIR },
};

const struct WildPokemonInfo SafariZone_Northwest_Sapphire_LandMonsInfo = { 25, SafariZone_Northwest_Sapphire_LandMons };
const struct WildPokemon SafariZone_Northwest_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_PSYDUCK },
    { 20, 30, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_GOLDUCK },
    { 25, 40, SPECIES_GOLDUCK },
};

const struct WildPokemonInfo SafariZone_Northwest_Sapphire_WaterMonsInfo = { 9, SafariZone_Northwest_Sapphire_WaterMons };
const struct WildPokemon SafariZone_Northwest_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 25, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_GOLDEEN },
    { 25, 30, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_SEAKING },
    { 35, 40, SPECIES_SEAKING },
    { 25, 30, SPECIES_SEAKING },
};

const struct WildPokemonInfo SafariZone_Northwest_Sapphire_FishingMonsInfo = { 35, SafariZone_Northwest_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SafariZone_Northeast_Sapphire_LandMons[] =
{
    { 27, 27, SPECIES_PHANPY },
    { 27, 27, SPECIES_ODDISH },
    { 29, 29, SPECIES_PHANPY },
    { 29, 29, SPECIES_ODDISH },
    { 27, 27, SPECIES_NATU },
    { 29, 29, SPECIES_GLOOM },
    { 31, 31, SPECIES_GLOOM },
    { 29, 29, SPECIES_NATU },
    { 29, 29, SPECIES_XATU },
    { 27, 27, SPECIES_HERACROSS },
    { 31, 31, SPECIES_XATU },
    { 29, 29, SPECIES_HERACROSS },
};

const struct WildPokemonInfo SafariZone_Northeast_Sapphire_LandMonsInfo = { 25, SafariZone_Northeast_Sapphire_LandMons };
const struct WildPokemon SafariZone_Northeast_Sapphire_RockSmashMons[] =
{
    { 10, 15, SPECIES_GEODUDE },
    { 5, 10, SPECIES_GEODUDE },
    { 15, 20, SPECIES_GEODUDE },
    { 20, 25, SPECIES_GEODUDE },
    { 25, 30, SPECIES_GEODUDE },
};

const struct WildPokemonInfo SafariZone_Northeast_Sapphire_RockSmashMonsInfo = { 25, SafariZone_Northeast_Sapphire_RockSmashMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SafariZone_Southwest_Sapphire_LandMons[] =
{
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 25, 25, SPECIES_GIRAFARIG },
    { 27, 27, SPECIES_GIRAFARIG },
    { 25, 25, SPECIES_NATU },
    { 27, 27, SPECIES_DODUO },
    { 25, 25, SPECIES_GLOOM },
    { 27, 27, SPECIES_WOBBUFFET },
    { 25, 25, SPECIES_PIKACHU },
    { 27, 27, SPECIES_WOBBUFFET },
    { 27, 27, SPECIES_PIKACHU },
    { 29, 29, SPECIES_WOBBUFFET },
};

const struct WildPokemonInfo SafariZone_Southwest_Sapphire_LandMonsInfo = { 25, SafariZone_Southwest_Sapphire_LandMons };
const struct WildPokemon SafariZone_Southwest_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_PSYDUCK },
    { 20, 30, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
    { 30, 35, SPECIES_PSYDUCK },
};

const struct WildPokemonInfo SafariZone_Southwest_Sapphire_WaterMonsInfo = { 9, SafariZone_Southwest_Sapphire_WaterMons };
const struct WildPokemon SafariZone_Southwest_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 25, SPECIES_GOLDEEN },
    { 10, 30, SPECIES_GOLDEEN },
    { 25, 30, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_GOLDEEN },
    { 30, 35, SPECIES_SEAKING },
    { 35, 40, SPECIES_SEAKING },
    { 25, 30, SPECIES_SEAKING },
};

const struct WildPokemonInfo SafariZone_Southwest_Sapphire_FishingMonsInfo = { 35, SafariZone_Southwest_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon SafariZone_Southeast_Sapphire_LandMons[] =
{
    { 25, 25, SPECIES_ODDISH },
    { 27, 27, SPECIES_ODDISH },
    { 25, 25, SPECIES_GIRAFARIG },
    { 27, 27, SPECIES_GIRAFARIG },
    { 25, 25, SPECIES_NATU },
    { 25, 25, SPECIES_DODUO },
    { 25, 25, SPECIES_GLOOM },
    { 27, 27, SPECIES_WOBBUFFET },
    { 25, 25, SPECIES_PIKACHU },
    { 27, 27, SPECIES_WOBBUFFET },
    { 27, 27, SPECIES_PIKACHU },
    { 29, 29, SPECIES_WOBBUFFET },
};

const struct WildPokemonInfo SafariZone_Southeast_Sapphire_LandMonsInfo = { 25, SafariZone_Southeast_Sapphire_LandMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon DewfordTown_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo DewfordTown_Sapphire_WaterMonsInfo = { 4, DewfordTown_Sapphire_WaterMons };
const struct WildPokemon DewfordTown_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_WAILMER },
    { 20, 25, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo DewfordTown_Sapphire_FishingMonsInfo = { 10, DewfordTown_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon PacifidlogTown_Sapphire_WaterMons[] =
{
    { 5, 35, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WINGULL },
    { 15, 25, SPECIES_WINGULL },
    { 25, 30, SPECIES_PELIPPER },
    { 25, 30, SPECIES_PELIPPER },
};

const struct WildPokemonInfo PacifidlogTown_Sapphire_WaterMonsInfo = { 4, PacifidlogTown_Sapphire_WaterMons };
const struct WildPokemon PacifidlogTown_Sapphire_FishingMons[] =
{
    { 5, 10, SPECIES_MAGIKARP },
    { 5, 10, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_MAGIKARP },
    { 10, 30, SPECIES_TENTACOOL },
    { 10, 30, SPECIES_WAILMER },
    { 30, 35, SPECIES_SHARPEDO },
    { 30, 35, SPECIES_WAILMER },
    { 25, 30, SPECIES_WAILMER },
    { 35, 40, SPECIES_WAILMER },
    { 40, 45, SPECIES_WAILMER },
};

const struct WildPokemonInfo PacifidlogTown_Sapphire_FishingMonsInfo = { 10, PacifidlogTown_Sapphire_FishingMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Underwater1_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_CLAMPERL },
    { 20, 30, SPECIES_CHINCHOU },
    { 30, 35, SPECIES_CLAMPERL },
    { 30, 35, SPECIES_RELICANTH },
    { 30, 35, SPECIES_RELICANTH },
};

const struct WildPokemonInfo Underwater1_Sapphire_WaterMonsInfo = { 4, Underwater1_Sapphire_WaterMons };
#endif

#ifdef SAPPHIRE
const struct WildPokemon Underwater2_Sapphire_WaterMons[] =
{
    { 20, 30, SPECIES_CLAMPERL },
    { 20, 30, SPECIES_CHINCHOU },
    { 30, 35, SPECIES_CLAMPERL },
    { 30, 35, SPECIES_RELICANTH },
    { 30, 35, SPECIES_RELICANTH },
};

const struct WildPokemonInfo Underwater2_Sapphire_WaterMonsInfo = { 4, Underwater2_Sapphire_WaterMons };
#endif


const struct WildPokemonHeader gWildMonHeaders[] =
{
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_PETALBURG_CITY),
        .mapNum = MAP_NUM(MAP_PETALBURG_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &PetalburgCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &PetalburgCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY),
        .mapNum = MAP_NUM(MAP_SLATEPORT_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SlateportCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SlateportCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY),
        .mapNum = MAP_NUM(MAP_LILYCOVE_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &LilycoveCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &LilycoveCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MOSSDEEP_CITY),
        .mapNum = MAP_NUM(MAP_MOSSDEEP_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &MossdeepCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MossdeepCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SOOTOPOLIS_CITY),
        .mapNum = MAP_NUM(MAP_SOOTOPOLIS_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SootopolisCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SootopolisCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_EVER_GRANDE_CITY),
        .mapNum = MAP_NUM(MAP_EVER_GRANDE_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &EverGrandeCity_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &EverGrandeCity_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_1F_1R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_1F_1R),
        .landMonsInfo = &MeteorFalls_1F_1R_Ruby_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_1F_1R_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_1F_1R_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_1F_2R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_1F_2R),
        .landMonsInfo = &MeteorFalls_1F_2R_Ruby_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_1F_2R_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_1F_2R_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_B1F_1R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_B1F_1R),
        .landMonsInfo = &MeteorFalls_B1F_1R_Ruby_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_B1F_1R_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_B1F_1R_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_B1F_2R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_B1F_2R),
        .landMonsInfo = &MeteorFalls_B1F_2R_Ruby_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_B1F_2R_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_B1F_2R_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_RUSTURF_TUNNEL),
        .mapNum = MAP_NUM(MAP_RUSTURF_TUNNEL),
        .landMonsInfo = &RusturfTunnel_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_1F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_1F),
        .landMonsInfo = &GraniteCave_1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_B1F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_B1F),
        .landMonsInfo = &GraniteCave_B1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_B2F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_B2F),
        .landMonsInfo = &GraniteCave_B2F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &GraniteCave_B2F_Ruby_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_STEVENS_ROOM),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_STEVENS_ROOM),
        .landMonsInfo = &GraniteCave_StevensRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
        .mapNum = MAP_NUM(MAP_PETALBURG_WOODS),
        .landMonsInfo = &PetalburgWoods_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_JAGGED_PASS),
        .mapNum = MAP_NUM(MAP_JAGGED_PASS),
        .landMonsInfo = &JaggedPass_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_FIERY_PATH),
        .mapNum = MAP_NUM(MAP_FIERY_PATH),
        .landMonsInfo = &FieryPath_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_1F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_1F),
        .landMonsInfo = &MtPyre_1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_2F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_2F),
        .landMonsInfo = &MtPyre_2F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_3F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_3F),
        .landMonsInfo = &MtPyre_3F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_4F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_4F),
        .landMonsInfo = &MtPyre_4F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_5F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_5F),
        .landMonsInfo = &MtPyre_5F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_6F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_6F),
        .landMonsInfo = &MtPyre_6F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_EXTERIOR),
        .mapNum = MAP_NUM(MAP_MT_PYRE_EXTERIOR),
        .landMonsInfo = &MtPyre_Exterior_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_SUMMIT),
        .mapNum = MAP_NUM(MAP_MT_PYRE_SUMMIT),
        .landMonsInfo = &MtPyre_Summit_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ENTRANCE),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ENTRANCE),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SeafloorCavern_Entrance_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Entrance_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM1),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM1),
        .landMonsInfo = &SeafloorCavern_Room1_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM2),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM2),
        .landMonsInfo = &SeafloorCavern_Room2_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM3),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM3),
        .landMonsInfo = &SeafloorCavern_Room3_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM4),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM4),
        .landMonsInfo = &SeafloorCavern_Room4_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM5),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM5),
        .landMonsInfo = &SeafloorCavern_Room5_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM6),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM6),
        .landMonsInfo = &SeafloorCavern_Room6_Ruby_LandMonsInfo,
        .waterMonsInfo = &SeafloorCavern_Room6_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Room6_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM7),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM7),
        .landMonsInfo = &SeafloorCavern_Room7_Ruby_LandMonsInfo,
        .waterMonsInfo = &SeafloorCavern_Room7_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Room7_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM8),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM8),
        .landMonsInfo = &SeafloorCavern_Room8_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_ENTRANCE),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_ENTRANCE),
        .landMonsInfo = &CaveOfOrigin_Entrance_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_1F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_1F),
        .landMonsInfo = &CaveOfOrigin_1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B1F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B1F),
        .landMonsInfo = &CaveOfOrigin_B1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B2F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B2F),
        .landMonsInfo = &CaveOfOrigin_B2F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B3F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B3F),
        .landMonsInfo = &CaveOfOrigin_B3F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_1F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_1F),
        .landMonsInfo = &VictoryRoad_1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_B1F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_B1F),
        .landMonsInfo = &VictoryRoad_B1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &VictoryRoad_B1F_Ruby_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_B2F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_B2F),
        .landMonsInfo = &VictoryRoad_B2F_Ruby_LandMonsInfo,
        .waterMonsInfo = &VictoryRoad_B2F_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &VictoryRoad_B2F_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM),
        .landMonsInfo = &ShoalCave_LowTideEntranceRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = &ShoalCave_LowTideEntranceRoom_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &ShoalCave_LowTideEntranceRoom_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM),
        .landMonsInfo = &ShoalCave_LowTideInnerRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = &ShoalCave_LowTideInnerRoom_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &ShoalCave_LowTideInnerRoom_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM),
        .landMonsInfo = &ShoalCave_LowTideStairsRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM),
        .landMonsInfo = &ShoalCave_LowTideLowerRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
        .landMonsInfo = &ShoalCave_LowTideIceRoom_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_NEW_MAUVILLE_ENTRANCE),
        .mapNum = MAP_NUM(MAP_NEW_MAUVILLE_ENTRANCE),
        .landMonsInfo = &NewMauville_Entrance_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_NEW_MAUVILLE_INSIDE),
        .mapNum = MAP_NUM(MAP_NEW_MAUVILLE_INSIDE),
        .landMonsInfo = &NewMauville_Inside_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ABANDONED_SHIP_ROOMS_B1F),
        .mapNum = MAP_NUM(MAP_ABANDONED_SHIP_ROOMS_B1F),
        .landMonsInfo = NULL,
        .waterMonsInfo = &AbandonedShip_Rooms_B1F_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &AbandonedShip_Rooms_B1F_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS),
        .mapNum = MAP_NUM(MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS),
        .landMonsInfo = NULL,
        .waterMonsInfo = &AbandonedShip_HiddenFloorCorridors_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &AbandonedShip_HiddenFloorCorridors_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_1F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_1F),
        .landMonsInfo = &SkyPillar_1F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_3F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_3F),
        .landMonsInfo = &SkyPillar_3F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_5F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_5F),
        .landMonsInfo = &SkyPillar_5F_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE101),
        .mapNum = MAP_NUM(MAP_ROUTE101),
        .landMonsInfo = &Route101_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
        .mapNum = MAP_NUM(MAP_ROUTE102),
        .landMonsInfo = &Route102_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route102_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route102_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
        .mapNum = MAP_NUM(MAP_ROUTE103),
        .landMonsInfo = &Route103_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route103_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route103_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE104),
        .mapNum = MAP_NUM(MAP_ROUTE104),
        .landMonsInfo = &Route104_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route104_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route104_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE105),
        .mapNum = MAP_NUM(MAP_ROUTE105),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route105_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route105_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE106),
        .mapNum = MAP_NUM(MAP_ROUTE106),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route106_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route106_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE107),
        .mapNum = MAP_NUM(MAP_ROUTE107),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route107_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route107_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE108),
        .mapNum = MAP_NUM(MAP_ROUTE108),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route108_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route108_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE109),
        .mapNum = MAP_NUM(MAP_ROUTE109),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route109_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route109_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE110),
        .mapNum = MAP_NUM(MAP_ROUTE110),
        .landMonsInfo = &Route110_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route110_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route110_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE111),
        .mapNum = MAP_NUM(MAP_ROUTE111),
        .landMonsInfo = &Route111_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route111_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = &Route111_Ruby_RockSmashMonsInfo,
        .fishingMonsInfo = &Route111_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE112),
        .mapNum = MAP_NUM(MAP_ROUTE112),
        .landMonsInfo = &Route112_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE113),
        .mapNum = MAP_NUM(MAP_ROUTE113),
        .landMonsInfo = &Route113_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE114),
        .mapNum = MAP_NUM(MAP_ROUTE114),
        .landMonsInfo = &Route114_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route114_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = &Route114_Ruby_RockSmashMonsInfo,
        .fishingMonsInfo = &Route114_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE115),
        .mapNum = MAP_NUM(MAP_ROUTE115),
        .landMonsInfo = &Route115_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route115_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route115_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE116),
        .mapNum = MAP_NUM(MAP_ROUTE116),
        .landMonsInfo = &Route116_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE117),
        .mapNum = MAP_NUM(MAP_ROUTE117),
        .landMonsInfo = &Route117_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route117_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route117_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE118),
        .mapNum = MAP_NUM(MAP_ROUTE118),
        .landMonsInfo = &Route118_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route118_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route118_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE119),
        .mapNum = MAP_NUM(MAP_ROUTE119),
        .landMonsInfo = &Route119_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route119_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route119_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE120),
        .mapNum = MAP_NUM(MAP_ROUTE120),
        .landMonsInfo = &Route120_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route120_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route120_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE121),
        .mapNum = MAP_NUM(MAP_ROUTE121),
        .landMonsInfo = &Route121_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route121_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route121_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE122),
        .mapNum = MAP_NUM(MAP_ROUTE122),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route122_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route122_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE123),
        .mapNum = MAP_NUM(MAP_ROUTE123),
        .landMonsInfo = &Route123_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route123_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route123_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE124),
        .mapNum = MAP_NUM(MAP_ROUTE124),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route124_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route124_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE125),
        .mapNum = MAP_NUM(MAP_ROUTE125),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route125_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route125_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE126),
        .mapNum = MAP_NUM(MAP_ROUTE126),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route126_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route126_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE127),
        .mapNum = MAP_NUM(MAP_ROUTE127),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route127_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route127_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE128),
        .mapNum = MAP_NUM(MAP_ROUTE128),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route128_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route128_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE129),
        .mapNum = MAP_NUM(MAP_ROUTE129),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route129_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route129_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE130),
        .mapNum = MAP_NUM(MAP_ROUTE130),
        .landMonsInfo = &Route130_Ruby_LandMonsInfo,
        .waterMonsInfo = &Route130_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route130_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE131),
        .mapNum = MAP_NUM(MAP_ROUTE131),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route131_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route131_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE132),
        .mapNum = MAP_NUM(MAP_ROUTE132),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route132_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route132_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE133),
        .mapNum = MAP_NUM(MAP_ROUTE133),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route133_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route133_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE134),
        .mapNum = MAP_NUM(MAP_ROUTE134),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route134_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route134_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_NORTHWEST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_NORTHWEST),
        .landMonsInfo = &SafariZone_Northwest_Ruby_LandMonsInfo,
        .waterMonsInfo = &SafariZone_Northwest_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SafariZone_Northwest_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_NORTHEAST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_NORTHEAST),
        .landMonsInfo = &SafariZone_Northeast_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &SafariZone_Northeast_Ruby_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_SOUTHWEST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_SOUTHWEST),
        .landMonsInfo = &SafariZone_Southwest_Ruby_LandMonsInfo,
        .waterMonsInfo = &SafariZone_Southwest_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SafariZone_Southwest_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_SOUTHEAST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_SOUTHEAST),
        .landMonsInfo = &SafariZone_Southeast_Ruby_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_DEWFORD_TOWN),
        .mapNum = MAP_NUM(MAP_DEWFORD_TOWN),
        .landMonsInfo = NULL,
        .waterMonsInfo = &DewfordTown_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &DewfordTown_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_PACIFIDLOG_TOWN),
        .mapNum = MAP_NUM(MAP_PACIFIDLOG_TOWN),
        .landMonsInfo = NULL,
        .waterMonsInfo = &PacifidlogTown_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &PacifidlogTown_Ruby_FishingMonsInfo,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_UNDERWATER1),
        .mapNum = MAP_NUM(MAP_UNDERWATER1),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Underwater1_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef RUBY
    {
        .mapGroup = MAP_GROUP(MAP_UNDERWATER2),
        .mapNum = MAP_NUM(MAP_UNDERWATER2),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Underwater2_Ruby_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_PETALBURG_CITY),
        .mapNum = MAP_NUM(MAP_PETALBURG_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &PetalburgCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &PetalburgCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY),
        .mapNum = MAP_NUM(MAP_SLATEPORT_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SlateportCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SlateportCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY),
        .mapNum = MAP_NUM(MAP_LILYCOVE_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &LilycoveCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &LilycoveCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MOSSDEEP_CITY),
        .mapNum = MAP_NUM(MAP_MOSSDEEP_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &MossdeepCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MossdeepCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SOOTOPOLIS_CITY),
        .mapNum = MAP_NUM(MAP_SOOTOPOLIS_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SootopolisCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SootopolisCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_EVER_GRANDE_CITY),
        .mapNum = MAP_NUM(MAP_EVER_GRANDE_CITY),
        .landMonsInfo = NULL,
        .waterMonsInfo = &EverGrandeCity_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &EverGrandeCity_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_1F_1R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_1F_1R),
        .landMonsInfo = &MeteorFalls_1F_1R_Sapphire_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_1F_1R_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_1F_1R_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_1F_2R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_1F_2R),
        .landMonsInfo = &MeteorFalls_1F_2R_Sapphire_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_1F_2R_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_1F_2R_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_B1F_1R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_B1F_1R),
        .landMonsInfo = &MeteorFalls_B1F_1R_Sapphire_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_B1F_1R_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_B1F_1R_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_METEOR_FALLS_B1F_2R),
        .mapNum = MAP_NUM(MAP_METEOR_FALLS_B1F_2R),
        .landMonsInfo = &MeteorFalls_B1F_2R_Sapphire_LandMonsInfo,
        .waterMonsInfo = &MeteorFalls_B1F_2R_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &MeteorFalls_B1F_2R_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_RUSTURF_TUNNEL),
        .mapNum = MAP_NUM(MAP_RUSTURF_TUNNEL),
        .landMonsInfo = &RusturfTunnel_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_1F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_1F),
        .landMonsInfo = &GraniteCave_1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_B1F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_B1F),
        .landMonsInfo = &GraniteCave_B1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_B2F),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_B2F),
        .landMonsInfo = &GraniteCave_B2F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &GraniteCave_B2F_Sapphire_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_GRANITE_CAVE_STEVENS_ROOM),
        .mapNum = MAP_NUM(MAP_GRANITE_CAVE_STEVENS_ROOM),
        .landMonsInfo = &GraniteCave_StevensRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
        .mapNum = MAP_NUM(MAP_PETALBURG_WOODS),
        .landMonsInfo = &PetalburgWoods_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_JAGGED_PASS),
        .mapNum = MAP_NUM(MAP_JAGGED_PASS),
        .landMonsInfo = &JaggedPass_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_FIERY_PATH),
        .mapNum = MAP_NUM(MAP_FIERY_PATH),
        .landMonsInfo = &FieryPath_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_1F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_1F),
        .landMonsInfo = &MtPyre_1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_2F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_2F),
        .landMonsInfo = &MtPyre_2F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_3F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_3F),
        .landMonsInfo = &MtPyre_3F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_4F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_4F),
        .landMonsInfo = &MtPyre_4F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_5F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_5F),
        .landMonsInfo = &MtPyre_5F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_6F),
        .mapNum = MAP_NUM(MAP_MT_PYRE_6F),
        .landMonsInfo = &MtPyre_6F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_EXTERIOR),
        .mapNum = MAP_NUM(MAP_MT_PYRE_EXTERIOR),
        .landMonsInfo = &MtPyre_Exterior_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_MT_PYRE_SUMMIT),
        .mapNum = MAP_NUM(MAP_MT_PYRE_SUMMIT),
        .landMonsInfo = &MtPyre_Summit_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ENTRANCE),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ENTRANCE),
        .landMonsInfo = NULL,
        .waterMonsInfo = &SeafloorCavern_Entrance_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Entrance_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM1),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM1),
        .landMonsInfo = &SeafloorCavern_Room1_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM2),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM2),
        .landMonsInfo = &SeafloorCavern_Room2_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM3),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM3),
        .landMonsInfo = &SeafloorCavern_Room3_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM4),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM4),
        .landMonsInfo = &SeafloorCavern_Room4_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM5),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM5),
        .landMonsInfo = &SeafloorCavern_Room5_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM6),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM6),
        .landMonsInfo = &SeafloorCavern_Room6_Sapphire_LandMonsInfo,
        .waterMonsInfo = &SeafloorCavern_Room6_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Room6_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM7),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM7),
        .landMonsInfo = &SeafloorCavern_Room7_Sapphire_LandMonsInfo,
        .waterMonsInfo = &SeafloorCavern_Room7_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SeafloorCavern_Room7_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM8),
        .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM8),
        .landMonsInfo = &SeafloorCavern_Room8_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_ENTRANCE),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_ENTRANCE),
        .landMonsInfo = &CaveOfOrigin_Entrance_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_1F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_1F),
        .landMonsInfo = &CaveOfOrigin_1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B1F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B1F),
        .landMonsInfo = &CaveOfOrigin_B1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B2F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B2F),
        .landMonsInfo = &CaveOfOrigin_B2F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_CAVE_OF_ORIGIN_B3F),
        .mapNum = MAP_NUM(MAP_CAVE_OF_ORIGIN_B3F),
        .landMonsInfo = &CaveOfOrigin_B3F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_1F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_1F),
        .landMonsInfo = &VictoryRoad_1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_B1F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_B1F),
        .landMonsInfo = &VictoryRoad_B1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &VictoryRoad_B1F_Sapphire_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_B2F),
        .mapNum = MAP_NUM(MAP_VICTORY_ROAD_B2F),
        .landMonsInfo = &VictoryRoad_B2F_Sapphire_LandMonsInfo,
        .waterMonsInfo = &VictoryRoad_B2F_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &VictoryRoad_B2F_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM),
        .landMonsInfo = &ShoalCave_LowTideEntranceRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = &ShoalCave_LowTideEntranceRoom_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &ShoalCave_LowTideEntranceRoom_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM),
        .landMonsInfo = &ShoalCave_LowTideInnerRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = &ShoalCave_LowTideInnerRoom_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &ShoalCave_LowTideInnerRoom_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM),
        .landMonsInfo = &ShoalCave_LowTideStairsRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM),
        .landMonsInfo = &ShoalCave_LowTideLowerRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
        .mapNum = MAP_NUM(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM),
        .landMonsInfo = &ShoalCave_LowTideIceRoom_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_NEW_MAUVILLE_ENTRANCE),
        .mapNum = MAP_NUM(MAP_NEW_MAUVILLE_ENTRANCE),
        .landMonsInfo = &NewMauville_Entrance_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_NEW_MAUVILLE_INSIDE),
        .mapNum = MAP_NUM(MAP_NEW_MAUVILLE_INSIDE),
        .landMonsInfo = &NewMauville_Inside_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ABANDONED_SHIP_ROOMS_B1F),
        .mapNum = MAP_NUM(MAP_ABANDONED_SHIP_ROOMS_B1F),
        .landMonsInfo = NULL,
        .waterMonsInfo = &AbandonedShip_Rooms_B1F_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &AbandonedShip_Rooms_B1F_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS),
        .mapNum = MAP_NUM(MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS),
        .landMonsInfo = NULL,
        .waterMonsInfo = &AbandonedShip_HiddenFloorCorridors_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &AbandonedShip_HiddenFloorCorridors_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_1F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_1F),
        .landMonsInfo = &SkyPillar_1F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_3F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_3F),
        .landMonsInfo = &SkyPillar_3F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SKY_PILLAR_5F),
        .mapNum = MAP_NUM(MAP_SKY_PILLAR_5F),
        .landMonsInfo = &SkyPillar_5F_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE101),
        .mapNum = MAP_NUM(MAP_ROUTE101),
        .landMonsInfo = &Route101_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
        .mapNum = MAP_NUM(MAP_ROUTE102),
        .landMonsInfo = &Route102_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route102_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route102_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
        .mapNum = MAP_NUM(MAP_ROUTE103),
        .landMonsInfo = &Route103_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route103_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route103_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE104),
        .mapNum = MAP_NUM(MAP_ROUTE104),
        .landMonsInfo = &Route104_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route104_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route104_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE105),
        .mapNum = MAP_NUM(MAP_ROUTE105),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route105_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route105_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE106),
        .mapNum = MAP_NUM(MAP_ROUTE106),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route106_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route106_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE107),
        .mapNum = MAP_NUM(MAP_ROUTE107),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route107_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route107_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE108),
        .mapNum = MAP_NUM(MAP_ROUTE108),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route108_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route108_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE109),
        .mapNum = MAP_NUM(MAP_ROUTE109),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route109_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route109_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE110),
        .mapNum = MAP_NUM(MAP_ROUTE110),
        .landMonsInfo = &Route110_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route110_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route110_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE111),
        .mapNum = MAP_NUM(MAP_ROUTE111),
        .landMonsInfo = &Route111_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route111_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = &Route111_Sapphire_RockSmashMonsInfo,
        .fishingMonsInfo = &Route111_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE112),
        .mapNum = MAP_NUM(MAP_ROUTE112),
        .landMonsInfo = &Route112_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE113),
        .mapNum = MAP_NUM(MAP_ROUTE113),
        .landMonsInfo = &Route113_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE114),
        .mapNum = MAP_NUM(MAP_ROUTE114),
        .landMonsInfo = &Route114_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route114_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = &Route114_Sapphire_RockSmashMonsInfo,
        .fishingMonsInfo = &Route114_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE115),
        .mapNum = MAP_NUM(MAP_ROUTE115),
        .landMonsInfo = &Route115_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route115_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route115_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE116),
        .mapNum = MAP_NUM(MAP_ROUTE116),
        .landMonsInfo = &Route116_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE117),
        .mapNum = MAP_NUM(MAP_ROUTE117),
        .landMonsInfo = &Route117_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route117_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route117_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE118),
        .mapNum = MAP_NUM(MAP_ROUTE118),
        .landMonsInfo = &Route118_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route118_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route118_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE119),
        .mapNum = MAP_NUM(MAP_ROUTE119),
        .landMonsInfo = &Route119_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route119_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route119_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE120),
        .mapNum = MAP_NUM(MAP_ROUTE120),
        .landMonsInfo = &Route120_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route120_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route120_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE121),
        .mapNum = MAP_NUM(MAP_ROUTE121),
        .landMonsInfo = &Route121_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route121_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route121_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE122),
        .mapNum = MAP_NUM(MAP_ROUTE122),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route122_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route122_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE123),
        .mapNum = MAP_NUM(MAP_ROUTE123),
        .landMonsInfo = &Route123_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route123_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route123_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE124),
        .mapNum = MAP_NUM(MAP_ROUTE124),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route124_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route124_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE125),
        .mapNum = MAP_NUM(MAP_ROUTE125),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route125_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route125_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE126),
        .mapNum = MAP_NUM(MAP_ROUTE126),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route126_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route126_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE127),
        .mapNum = MAP_NUM(MAP_ROUTE127),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route127_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route127_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE128),
        .mapNum = MAP_NUM(MAP_ROUTE128),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route128_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route128_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE129),
        .mapNum = MAP_NUM(MAP_ROUTE129),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route129_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route129_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE130),
        .mapNum = MAP_NUM(MAP_ROUTE130),
        .landMonsInfo = &Route130_Sapphire_LandMonsInfo,
        .waterMonsInfo = &Route130_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route130_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE131),
        .mapNum = MAP_NUM(MAP_ROUTE131),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route131_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route131_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE132),
        .mapNum = MAP_NUM(MAP_ROUTE132),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route132_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route132_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE133),
        .mapNum = MAP_NUM(MAP_ROUTE133),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route133_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route133_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE134),
        .mapNum = MAP_NUM(MAP_ROUTE134),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Route134_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &Route134_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_NORTHWEST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_NORTHWEST),
        .landMonsInfo = &SafariZone_Northwest_Sapphire_LandMonsInfo,
        .waterMonsInfo = &SafariZone_Northwest_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SafariZone_Northwest_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_NORTHEAST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_NORTHEAST),
        .landMonsInfo = &SafariZone_Northeast_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = &SafariZone_Northeast_Sapphire_RockSmashMonsInfo,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_SOUTHWEST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_SOUTHWEST),
        .landMonsInfo = &SafariZone_Southwest_Sapphire_LandMonsInfo,
        .waterMonsInfo = &SafariZone_Southwest_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &SafariZone_Southwest_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_SAFARI_ZONE_SOUTHEAST),
        .mapNum = MAP_NUM(MAP_SAFARI_ZONE_SOUTHEAST),
        .landMonsInfo = &SafariZone_Southeast_Sapphire_LandMonsInfo,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_DEWFORD_TOWN),
        .mapNum = MAP_NUM(MAP_DEWFORD_TOWN),
        .landMonsInfo = NULL,
        .waterMonsInfo = &DewfordTown_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &DewfordTown_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_PACIFIDLOG_TOWN),
        .mapNum = MAP_NUM(MAP_PACIFIDLOG_TOWN),
        .landMonsInfo = NULL,
        .waterMonsInfo = &PacifidlogTown_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = &PacifidlogTown_Sapphire_FishingMonsInfo,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_UNDERWATER1),
        .mapNum = MAP_NUM(MAP_UNDERWATER1),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Underwater1_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
#ifdef SAPPHIRE
    {
        .mapGroup = MAP_GROUP(MAP_UNDERWATER2),
        .mapNum = MAP_NUM(MAP_UNDERWATER2),
        .landMonsInfo = NULL,
        .waterMonsInfo = &Underwater2_Sapphire_WaterMonsInfo,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
#endif
    {
        .mapGroup = MAP_GROUP(MAP_UNDEFINED),
        .mapNum = MAP_NUM(MAP_UNDEFINED),
        .landMonsInfo = NULL,
        .waterMonsInfo = NULL,
        .rockSmashMonsInfo = NULL,
        .fishingMonsInfo = NULL,
    },
};
