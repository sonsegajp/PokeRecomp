"""
Full rebuild of hoenn_data.c from all generator outputs and Ruby source data.
Reconstructs: tilesets, layouts, NPCs, warps, events, connections, map headers.
"""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
BASE = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN = os.path.join(BASE, 'hoenn-postgame')
OUT = os.path.join(BASE, 'src', 'platform', 'hoenn_data.c')

# ===== PARSE RUBY DATA =====

def parse_layouts():
    layouts = {}
    path = os.path.join(POKERUBY, 'data', 'layouts', 'layouts.inc')
    with open(path) as f:
        lines = f.readlines()
    i = 0
    while i < len(lines):
        m = re.match(r'^(\w+)_Layout::', lines[i].strip())
        if m:
            name = m.group(1)
            w = h = 0; pri = sec = ""
            for j in range(i+1, min(i+10, len(lines))):
                l = lines[j].strip()
                if '.4byte' in l and 'Border' not in l and 'Blockdata' not in l:
                    val = l.split('.4byte')[1].strip()
                    if 'gTileset_' in val:
                        ts = val.replace('gTileset_', '')
                        if not pri: pri = ts
                        else: sec = ts
                    else:
                        try:
                            v = int(val)
                            if not w: w = v
                            elif not h: h = v
                        except: pass
            if w and h and pri and sec:
                layouts[name] = {'w': w, 'h': h, 'pri': pri, 'sec': sec}
        i += 1
    return layouts

def camel_to_snake(name):
    return re.sub(r'([A-Z])', r'_\1', name).lower().lstrip('_')

def get_tileset_info():
    tilesets = {}
    ts_base = os.path.join(POKERUBY, 'data', 'tilesets')
    for kind in ['primary', 'secondary']:
        kind_dir = os.path.join(ts_base, kind)
        if not os.path.isdir(kind_dir): continue
        for name in sorted(os.listdir(kind_dir)):
            ts_dir = os.path.join(kind_dir, name)
            if not os.path.isdir(ts_dir): continue
            tiles = os.path.join(ts_dir, 'tiles.4bpp')
            meta = os.path.join(ts_dir, 'metatiles.bin')
            tilesets[name] = {
                'kind': kind,
                'tile_count': os.path.getsize(tiles) // 32 if os.path.isfile(tiles) else 0,
                'metatile_count': os.path.getsize(meta) // 16 if os.path.isfile(meta) else 0,
                'has_tiles': os.path.isfile(tiles),
            }
    return tilesets

def resolve_ts(camel, tilesets):
    snake = camel_to_snake(camel)
    if snake in tilesets: return snake
    for k in tilesets:
        if k.lower().replace('_', '') == camel.lower(): return k
    return None

# ===== LOAD DATA =====
layouts = parse_layouts()
tilesets_info = get_tileset_info()

# Our tileset list (CamelCase names used in code)
OUR_TILESETS = [
    ('General', 'primary', 512, 512),
    ('Petalburg', 'secondary', 159, 144),
    ('Rustboro', 'secondary', 498, 350),
    ('Dewford', 'secondary', 343, 144),
    ('Slateport', 'secondary', 504, 364),
    ('Mauville', 'secondary', 450, 434),
    ('Fortree', 'secondary', 493, 267),
    ('Lilycove', 'secondary', 432, 351),
    ('Mossdeep', 'secondary', 499, 363),
    ('Sootopolis', 'secondary', 180, 254),
    ('EverGrande', 'secondary', 320, 168),
    ('Fallarbor', 'secondary', 374, 338),
    ('Lavaridge', 'secondary', 348, 286),
    ('Pacifidlog', 'secondary', 504, 191),
    ('Building', 'primary', 502, 8),
    ('Cave', 'secondary', 420, 414),
    ('GenericBuilding', 'secondary', 509, 512),
    ('Facility', 'secondary', 503, 510),
    ('Ship', 'secondary', 342, 252),
    ('Shop', 'secondary', 502, 280),
    ('Underwater', 'secondary', 500, 236),
    ('TrickHousePuzzle', 'secondary', 255, 149),
    ('EliteFour', 'secondary', 505, 331),
    ('CableClub', 'secondary', 512, 256),
    ('BattleTower', 'secondary', 128, 131),
    ('PokemonCenter', 'secondary', 478, 198),
    ('MeteorFalls', 'secondary', 422, 159),
    ('Lab', 'secondary', 500, 154),
    ('BrendansMaysHouse', 'secondary', 480, 171),
    ('SootopolisGym', 'secondary', 484, 99),
    ('OceanicMuseum', 'secondary', 319, 101),
    ('LilycoveMuseum', 'secondary', 431, 143),
    ('LavaridgeGym', 'secondary', 70, 42),
    ('BikeShop', 'secondary', 512, 248),
    ('SeashoreHouse', 'secondary', 312, 56),
    ('RusturfTunnel', 'secondary', 144, 83),
    ('RustboroGym', 'secondary', 94, 56),
    ('PrettyPetalFlowerShop', 'secondary', 345, 72),
    ('PokemonSchool', 'secondary', 278, 58),
    ('PokemonFanClub', 'secondary', 319, 104),
    ('PokemonDayCare', 'secondary', 355, 68),
    ('PetalburgGym', 'secondary', 384, 140),
    ('MossdeepGym', 'secondary', 122, 80),
    ('MauvilleGym', 'secondary', 160, 117),
    ('MauvilleGameCorner', 'secondary', 469, 96),
    ('InsideOfTruck', 'secondary', 62, 38),
    ('FortreeGym', 'secondary', 84, 53),
    ('DewfordGym', 'secondary', 88, 61),
    ('Contest', 'secondary', 430, 224),
]

ts_set = {name for name, _, _, _ in OUR_TILESETS}

# Find valid layouts (both tilesets available)
valid_layouts = set()
for lname, li in layouts.items():
    if li['pri'] in ts_set and li['sec'] in ts_set:
        valid_layouts.add(lname)

# Find all maps and their data
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
all_maps = {}
for dirname in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    layout_key = data.get('layout', '').replace('LAYOUT_', '')
    # Find matching layout
    lname = None
    for k in valid_layouts:
        if k.lower().replace('_', '') == layout_key.lower().replace('_', ''):
            lname = k
            break
    if not lname:
        map_name = data.get('name', dirname)
        for k in valid_layouts:
            if k.lower().replace('_', '') == map_name.lower().replace('_', ''):
                lname = k
                break
    if lname:
        all_maps[dirname] = {'json': data, 'layout': lname}

print(f"Valid layouts: {len(valid_layouts)}")
print(f"Maps with valid layouts: {len(all_maps)}")

# ===== READ EXISTING DATA FILES =====

# NPC data with dialogue
npc_path = os.path.join(HOENN, 'npc_data.h')
with open(npc_path) as f:
    npc_content = f.read()

# Warp data
warp_path = os.path.join(HOENN, 'warp_data.h')
with open(warp_path) as f:
    warp_content = f.read()

# Connections
conn_path = os.path.join(HOENN, 'connections.txt')
with open(conn_path) as f:
    conn_content = f.read()

# ===== MUSIC MAPPING =====
MUSIC_MAP = {
    'MUS_LITTLEROOT': 347, 'MUS_OLDALE': 348, 'MUS_PETALBURG': 349,
    'MUS_RUSTBORO': 350, 'MUS_DEWFORD': 351, 'MUS_SLATEPORT': 352,
    'MUS_CYCLING': 353, 'MUS_VERDANTURF': 354, 'MUS_FALLARBOR': 355,
    'MUS_ROUTE113': 356, 'MUS_FORTREE': 357, 'MUS_LILYCOVE': 358,
    'MUS_ROUTE119': 359, 'MUS_SOOTOPOLIS': 360, 'MUS_EVER_GRANDE': 361,
    'MUS_ROUTE101': 362, 'MUS_ROUTE104': 363, 'MUS_ROUTE110': 364,
    'MUS_ROUTE120': 365, 'MUS_ROUTE122': 366, 'MUS_SURF': 367,
    'MUS_MT_CHIMNEY': 368, 'MUS_GYM': 369, 'MUS_POKE_CENTER': 370,
    'MUS_POKE_MART': 371, 'MUS_BIRCH_LAB': 347, 'MUS_SCHOOL': 347,
    'MUS_GAME_CORNER': 364, 'MUS_OCEANIC_MUSEUM': 352, 'MUS_LILYCOVE_MUSEUM': 358,
    'MUS_AQUA_MAGMA_HIDEOUT': 368, 'MUS_MT_PYRE': 368, 'MUS_MT_PYRE_EXTERIOR': 368,
    'MUS_ABANDONED_SHIP': 367, 'MUS_SEALED_CHAMBER': 368, 'MUS_CAVE_OF_ORIGIN': 360,
    'MUS_TRICK_HOUSE': 364, 'MUS_PETALBURG_WOODS': 363, 'MUS_SAFARI_ZONE': 363,
    'MUS_B_TOWER': 361, 'MUS_HALL_OF_FAME_ROOM': 361, 'MUS_HALL_OF_FAME': 361,
    'MUS_CABLE_CAR': 364, 'MUS_SAILING': 367, 'MUS_CONTEST': 358,
    'MUS_CONTEST_LOBBY': 358, 'MUS_ROULETTE': 364, 'MUS_HELP': 347,
    'MUS_FOLLOW_ME': 347, 'MUS_CONTEST_RESULTS': 358,
}
WEATHER_MAP = {
    'WEATHER_NONE': 'WEATHER_NONE', 'WEATHER_CLOUDS': 'WEATHER_SUNNY_CLOUDS',
    'WEATHER_SUNNY': 'WEATHER_SUNNY', 'WEATHER_RAIN_LIGHT': 'WEATHER_RAIN',
    'WEATHER_SNOW': 'WEATHER_SNOW', 'WEATHER_RAIN_MED': 'WEATHER_RAIN_THUNDERSTORM',
    'WEATHER_FOG_1': 'WEATHER_FOG_HORIZONTAL', 'WEATHER_ASH': 'WEATHER_VOLCANIC_ASH',
    'WEATHER_SANDSTORM': 'WEATHER_SANDSTORM', 'WEATHER_FOG_2': 'WEATHER_FOG_DIAGONAL',
    'WEATHER_FOG_3': 'WEATHER_FOG_DIAGONAL', 'WEATHER_SHADE': 'WEATHER_SHADE',
    'WEATHER_DROUGHT': 'WEATHER_DROUGHT', 'WEATHER_RAIN_HEAVY': 'WEATHER_DOWNPOUR',
    'WEATHER_BUBBLES': 'WEATHER_UNDERWATER_BUBBLES', 'WEATHER_UNDERWATER': 'WEATHER_UNDERWATER',
}
MAP_TYPE_MAP = {
    'MAP_TYPE_TOWN': 'MAP_TYPE_TOWN', 'MAP_TYPE_CITY': 'MAP_TYPE_CITY',
    'MAP_TYPE_ROUTE': 'MAP_TYPE_ROUTE', 'MAP_TYPE_UNDERGROUND': 'MAP_TYPE_UNDERGROUND',
    'MAP_TYPE_UNDERWATER': 'MAP_TYPE_UNDERWATER', 'MAP_TYPE_6': 'MAP_TYPE_OCEAN_ROUTE',
    'MAP_TYPE_OCEAN_ROUTE': 'MAP_TYPE_OCEAN_ROUTE', 'MAP_TYPE_INDOOR': 'MAP_TYPE_INDOOR',
    'MAP_TYPE_SECRET_BASE': 'MAP_TYPE_SECRET_BASE',
}

# MAPSEC mapping
MAPSEC = {
    'MAPSEC_LITTLEROOT_TOWN': 208, 'MAPSEC_OLDALE_TOWN': 209, 'MAPSEC_PETALBURG_CITY': 210,
    'MAPSEC_RUSTBORO_CITY': 211, 'MAPSEC_DEWFORD_TOWN': 212, 'MAPSEC_SLATEPORT_CITY': 213,
    'MAPSEC_MAUVILLE_CITY': 214, 'MAPSEC_VERDANTURF_TOWN': 215, 'MAPSEC_FALLARBOR_TOWN': 216,
    'MAPSEC_LAVARIDGE_TOWN': 217, 'MAPSEC_FORTREE_CITY': 218, 'MAPSEC_LILYCOVE_CITY': 219,
    'MAPSEC_MOSSDEEP_CITY': 220, 'MAPSEC_SOOTOPOLIS_CITY': 221, 'MAPSEC_EVER_GRANDE_CITY': 222,
    'MAPSEC_PACIFIDLOG_TOWN': 223,
}
# Routes
for i in range(101, 132):
    MAPSEC[f'MAPSEC_ROUTE_{i}'] = min(224 + (i - 101), 254)
MAPSEC['MAPSEC_ROUTE_132'] = 255
MAPSEC['MAPSEC_ROUTE_133'] = 255
MAPSEC['MAPSEC_ROUTE_134'] = 255

def get_mapsec(ruby_mapsec):
    if ruby_mapsec in MAPSEC:
        return MAPSEC[ruby_mapsec]
    return 208  # fallback to Littleroot

def get_music(ruby_music):
    return MUSIC_MAP.get(ruby_music, 347)

print(f"\nReconstruction plan:")
print(f"  {len(OUR_TILESETS)} tilesets")
print(f"  {len(valid_layouts)} valid layouts")
print(f"  {len(all_maps)} maps")
print(f"\nRun the full generator to produce hoenn_data.c")
print("This script validates all data is available.")
print("The actual reconstruction needs to be done carefully.")
