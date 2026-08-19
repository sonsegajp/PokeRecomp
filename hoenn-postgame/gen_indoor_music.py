"""Generate music mappings for indoor Hoenn maps from Ruby's map.json."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Ruby MUS_ -> our Hoenn song IDs
MUSIC_MAP = {
    'MUS_LITTLEROOT': 347, 'MUS_OLDALE': 348, 'MUS_PETALBURG': 349,
    'MUS_RUSTBORO': 350, 'MUS_DEWFORD': 351, 'MUS_SLATEPORT': 352,
    'MUS_CYCLING': 353, 'MUS_VERDANTURF': 354, 'MUS_FALLARBOR': 355,
    'MUS_ROUTE113': 356, 'MUS_FORTREE': 357, 'MUS_LILYCOVE': 358,
    'MUS_ROUTE119': 359, 'MUS_SOOTOPOLIS': 360, 'MUS_EVER_GRANDE': 361,
    'MUS_ROUTE101': 362, 'MUS_ROUTE104': 363, 'MUS_ROUTE110': 364,
    'MUS_ROUTE120': 365, 'MUS_ROUTE122': 366, 'MUS_SURF': 367,
    'MUS_MT_CHIMNEY': 368, 'MUS_GYM': 369, 'MUS_POKE_CENTER': 370,
    'MUS_POKE_MART': 371, 'MUS_VS_WILD': 372, 'MUS_VS_TRAINER': 373,
    'MUS_VS_GYM_LEADER': 374, 'MUS_VS_CHAMPION': 375,
    'MUS_VS_ELITE_FOUR': 376, 'MUS_VS_AQUA_MAGMA': 377,
    # Map Ruby-specific names to our IDs
    'MUS_BIRCH_LAB': 347,  # use littleroot for lab
    'MUS_SCHOOL': 347,
    'MUS_GAME_CORNER': 364,  # reuse route110 for game corner
    'MUS_OCEANIC_MUSEUM': 352,  # reuse slateport
    'MUS_LILYCOVE_MUSEUM': 358,  # reuse lilycove
    'MUS_AQUA_MAGMA_HIDEOUT': 368,  # reuse mt chimney
    'MUS_MT_PYRE': 368,  # reuse mt chimney
    'MUS_MT_PYRE_EXTERIOR': 368,
    'MUS_ABANDONED_SHIP': 367,  # reuse surf
    'MUS_SEALED_CHAMBER': 368,
    'MUS_CAVE_OF_ORIGIN': 360,  # reuse sootopolis
    'MUS_TRICK_HOUSE': 364,
    'MUS_PETALBURG_WOODS': 363,  # reuse route104
    'MUS_SAFARI_ZONE': 363,
    'MUS_B_TOWER': 361,  # reuse ever grande
    'MUS_HALL_OF_FAME_ROOM': 361,
    'MUS_HALL_OF_FAME': 361,
    'MUS_CABLE_CAR': 364,
    'MUS_SAILING': 367,
    'MUS_CONTEST': 358,
    'MUS_CONTEST_LOBBY': 358,
    'MUS_CONTEST_RESULTS': 358,
    'MUS_HELP': 347,
    'MUS_ROULETTE': 364,
    'MUS_FOLLOW_ME': 347,
}

maps_dir = os.path.join(POKERUBY, 'data', 'maps')
updates = {}  # dirname -> music_id

for dirname in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    ruby_music = data.get('music', 'MUS_DUMMY')

    if ruby_music in MUSIC_MAP:
        updates[dirname] = MUSIC_MAP[ruby_music]
    else:
        # Try partial match
        for k, v in MUSIC_MAP.items():
            if ruby_music.endswith(k.replace('MUS_', '')):
                updates[dirname] = v
                break

# Print sed commands to update hoenn_data.c
for dirname, music_id in sorted(updates.items()):
    print(f"{dirname}: {music_id}")
