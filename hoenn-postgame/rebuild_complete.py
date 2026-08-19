"""
Complete rebuild of hoenn_data.c from Ruby source data.
Produces the entire file: tilesets, layouts, NPCs, warps, events, connections, map headers.
"""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
BASE = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN = os.path.join(BASE, 'hoenn-postgame')
OUT = os.path.join(BASE, 'src', 'platform', 'hoenn_data.c')

# ===== TILESET DATA =====
TILESETS = [
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
TS_SET = {n for n,_,_,_ in TILESETS}

# ===== PARSE LAYOUTS =====
def parse_layouts():
    layouts = {}
    path = os.path.join(POKERUBY, 'data', 'layouts', 'layouts.inc')
    with open(path) as f:
        lines = f.readlines()
    i = 0
    while i < len(lines):
        m = re.match(r'^(\w+)_Layout::', lines[i].strip())
        if m:
            name = m.group(1); w = h = 0; pri = sec = ""
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

layouts = parse_layouts()
valid_layouts = {k for k,v in layouts.items() if v['pri'] in TS_SET and v['sec'] in TS_SET}

# ===== PARSE ALL MAPS =====
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
all_maps = {}
for dirname in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    layout_key = data.get('layout', '').replace('LAYOUT_', '')
    lname = None
    for k in valid_layouts:
        if k.lower().replace('_', '') == layout_key.lower().replace('_', ''):
            lname = k; break
    if not lname:
        mn = data.get('name', dirname)
        for k in valid_layouts:
            if k.lower().replace('_', '') == mn.lower().replace('_', ''):
                lname = k; break
    if lname:
        all_maps[dirname] = {'json': data, 'layout': lname}

print(f"Tilesets: {len(TILESETS)}, Layouts: {len(valid_layouts)}, Maps: {len(all_maps)}")

# ===== MUSIC/WEATHER/MAPSEC MAPS =====
MUSIC = {
    'MUS_LITTLEROOT':347,'MUS_OLDALE':348,'MUS_PETALBURG':349,'MUS_RUSTBORO':350,
    'MUS_DEWFORD':351,'MUS_SLATEPORT':352,'MUS_CYCLING':353,'MUS_VERDANTURF':354,
    'MUS_FALLARBOR':355,'MUS_ROUTE113':356,'MUS_FORTREE':357,'MUS_LILYCOVE':358,
    'MUS_ROUTE119':359,'MUS_SOOTOPOLIS':360,'MUS_EVER_GRANDE':361,'MUS_ROUTE101':362,
    'MUS_ROUTE104':363,'MUS_ROUTE110':364,'MUS_ROUTE120':365,'MUS_ROUTE122':366,
    'MUS_SURF':367,'MUS_MT_CHIMNEY':368,'MUS_GYM':369,'MUS_POKE_CENTER':370,
    'MUS_POKE_MART':371,'MUS_BIRCH_LAB':347,'MUS_SCHOOL':347,'MUS_GAME_CORNER':364,
    'MUS_OCEANIC_MUSEUM':352,'MUS_LILYCOVE_MUSEUM':358,'MUS_AQUA_MAGMA_HIDEOUT':368,
    'MUS_MT_PYRE':368,'MUS_MT_PYRE_EXTERIOR':368,'MUS_ABANDONED_SHIP':367,
    'MUS_SEALED_CHAMBER':368,'MUS_CAVE_OF_ORIGIN':360,'MUS_TRICK_HOUSE':364,
    'MUS_PETALBURG_WOODS':363,'MUS_SAFARI_ZONE':363,'MUS_B_TOWER':361,
    'MUS_HALL_OF_FAME_ROOM':361,'MUS_HALL_OF_FAME':361,'MUS_CABLE_CAR':364,
    'MUS_SAILING':367,'MUS_CONTEST':358,'MUS_CONTEST_LOBBY':358,'MUS_ROULETTE':364,
}
WEATHER = {
    'WEATHER_NONE':'WEATHER_NONE','WEATHER_CLOUDS':'WEATHER_SUNNY_CLOUDS',
    'WEATHER_SUNNY':'WEATHER_SUNNY','WEATHER_RAIN_LIGHT':'WEATHER_RAIN',
    'WEATHER_SNOW':'WEATHER_SNOW','WEATHER_RAIN_MED':'WEATHER_RAIN_THUNDERSTORM',
    'WEATHER_FOG_1':'WEATHER_FOG_HORIZONTAL','WEATHER_ASH':'WEATHER_VOLCANIC_ASH',
    'WEATHER_SANDSTORM':'WEATHER_SANDSTORM','WEATHER_FOG_2':'WEATHER_FOG_DIAGONAL',
    'WEATHER_SHADE':'WEATHER_SHADE','WEATHER_DROUGHT':'WEATHER_DROUGHT',
    'WEATHER_RAIN_HEAVY':'WEATHER_DOWNPOUR','WEATHER_BUBBLES':'WEATHER_UNDERWATER_BUBBLES',
    'WEATHER_UNDERWATER':'WEATHER_UNDERWATER','WEATHER_FOG_3':'WEATHER_FOG_DIAGONAL',
}
MAPTYPE = {
    'MAP_TYPE_TOWN':'MAP_TYPE_TOWN','MAP_TYPE_CITY':'MAP_TYPE_CITY',
    'MAP_TYPE_ROUTE':'MAP_TYPE_ROUTE','MAP_TYPE_UNDERGROUND':'MAP_TYPE_UNDERGROUND',
    'MAP_TYPE_UNDERWATER':'MAP_TYPE_UNDERWATER','MAP_TYPE_6':'MAP_TYPE_OCEAN_ROUTE',
    'MAP_TYPE_OCEAN_ROUTE':'MAP_TYPE_OCEAN_ROUTE','MAP_TYPE_INDOOR':'MAP_TYPE_INDOOR',
    'MAP_TYPE_SECRET_BASE':'MAP_TYPE_SECRET_BASE',
}
MAPSEC = {}
for city, val in [('LITTLEROOT_TOWN',208),('OLDALE_TOWN',209),('PETALBURG_CITY',210),
    ('RUSTBORO_CITY',211),('DEWFORD_TOWN',212),('SLATEPORT_CITY',213),('MAUVILLE_CITY',214),
    ('VERDANTURF_TOWN',215),('FALLARBOR_TOWN',216),('LAVARIDGE_TOWN',217),('FORTREE_CITY',218),
    ('LILYCOVE_CITY',219),('MOSSDEEP_CITY',220),('SOOTOPOLIS_CITY',221),('EVER_GRANDE_CITY',222),
    ('PACIFIDLOG_TOWN',223)]:
    MAPSEC[f'MAPSEC_{city}'] = val
for i in range(101,132):
    MAPSEC[f'MAPSEC_ROUTE_{i}'] = min(224+i-101,254)
for i in range(132,135):
    MAPSEC[f'MAPSEC_ROUTE_{i}'] = 255
# Indoor/dungeon MAPSECs -> map to parent outdoor area
MAPSEC['MAPSEC_UNDERWATER_124'] = 247  # Route 124
MAPSEC['MAPSEC_UNDERWATER_125'] = 248
MAPSEC['MAPSEC_UNDERWATER_126'] = 249
MAPSEC['MAPSEC_UNDERWATER_127'] = 250
MAPSEC['MAPSEC_UNDERWATER_SOOTOPOLIS'] = 221
MAPSEC['MAPSEC_UNDERWATER_128'] = 251
MAPSEC['MAPSEC_UNDERWATER_SEALED_CHAMBER'] = 251
MAPSEC['MAPSEC_GRANITE_CAVE'] = 229     # Route 106
MAPSEC['MAPSEC_MT_CHIMNEY'] = 235       # Route 112
MAPSEC['MAPSEC_SAFARI_ZONE'] = 239      # Route 116 area
MAPSEC['MAPSEC_BATTLE_TOWER'] = 251     # Route 128 area
MAPSEC['MAPSEC_PETALBURG_WOODS'] = 227  # Route 104
MAPSEC['MAPSEC_RUSTURF_TUNNEL'] = 239   # Route 116
MAPSEC['MAPSEC_ABANDONED_SHIP'] = 232   # Route 109
MAPSEC['MAPSEC_NEW_MAUVILLE'] = 233     # Route 110
MAPSEC['MAPSEC_METEOR_FALLS'] = 237     # Route 114
MAPSEC['MAPSEC_METEOR_FALLS_2'] = 237
MAPSEC['MAPSEC_MT_PYRE'] = 245          # Route 122
MAPSEC['MAPSEC_EVIL_TEAM_HIDEOUT'] = 219  # Lilycove
MAPSEC['MAPSEC_SHOAL_CAVE'] = 248       # Route 125
MAPSEC['MAPSEC_SEAFLOOR_CAVERN'] = 251  # Route 128
MAPSEC['MAPSEC_VICTORY_ROAD'] = 222     # Ever Grande
MAPSEC['MAPSEC_MIRAGE_ISLAND'] = 253    # Route 130
MAPSEC['MAPSEC_CAVE_OF_ORIGIN'] = 221   # Sootopolis
MAPSEC['MAPSEC_SOUTHERN_ISLAND'] = 253
MAPSEC['MAPSEC_FIERY_PATH'] = 235       # Route 112
MAPSEC['MAPSEC_FIERY_PATH_2'] = 235
MAPSEC['MAPSEC_JAGGED_PASS'] = 235
MAPSEC['MAPSEC_JAGGED_PASS_2'] = 235
MAPSEC['MAPSEC_SEALED_CHAMBER'] = 251
MAPSEC['MAPSEC_SCORCHED_SLAB'] = 243    # Route 120
MAPSEC['MAPSEC_ISLAND_CAVE'] = 228      # Route 105
MAPSEC['MAPSEC_DESERT_RUINS'] = 234     # Route 111
MAPSEC['MAPSEC_ANCIENT_TOMB'] = 243     # Route 120
MAPSEC['MAPSEC_INSIDE_OF_TRUCK'] = 208  # Littleroot
MAPSEC['MAPSEC_SKY_PILLAR'] = 253       # Route 131
MAPSEC['MAPSEC_SECRET_BASE'] = 208
MAPSEC['MAPSEC_DYNAMIC'] = 208
MAPSEC['MAPSEC_NONE'] = 208

# ===== READ EXISTING DATA =====
npc_path = os.path.join(HOENN, 'npc_data.h')
warp_path = os.path.join(HOENN, 'warp_data.h')
conn_path = os.path.join(HOENN, 'connections.txt')

with open(npc_path) as f: npc_content = f.read()
with open(warp_path) as f: warp_content = f.read()
with open(conn_path) as f: conn_content = f.read()

# Parse NPC event updates
npc_updates = {}
for m in re.finditer(r'// (\w+): \.objectEventCount = (\d+), \.objectEvents = (sNPCs_\w+)', npc_content):
    npc_updates[m.group(1)] = (int(m.group(2)), m.group(3))

# Parse warp event structs
warp_events = {}
for m in re.finditer(r'static const struct MapEvents (sEvents_\w+) = \{([^}]+)\}', warp_content):
    name = m.group(1).replace('sEvents_', '')
    warp_events[name] = m.group(1)

# Parse connection data
conn_structs = {}
for m in re.finditer(r'static const struct MapConnections (s\w+ConnData) = \{ (\d+), (s\w+Conns) \}', conn_content):
    # Extract the map name from the variable name
    var = m.group(1)  # e.g. sLittlerootTownConnData
    conn_structs[var] = True

print(f"NPC updates: {len(npc_updates)}, Warp events: {len(warp_events)}")

# ===== GENERATE THE FILE =====
with open(OUT, 'w') as f:
    f.write("#ifdef PORTABLE\n")
    f.write('#include "global.h"\n#include "global.fieldmap.h"\n')
    f.write('#include "constants/hoenn.h"\n#include "constants/hoenn_map_groups.h"\n')
    f.write('#include "constants/weather.h"\n#include "constants/map_types.h"\n')
    f.write('#include "constants/maps.h"\n#include "constants/event_bg.h"\n')
    f.write('#include "fieldmap.h"\n#include <stdio.h>\n#include <string.h>\n\n')

    # Constants
    f.write("#define RUBY_TILES_PRIMARY 512\n#define RUBY_METATILES_PRIMARY 512\n")
    f.write("#define RUBY_PALS_PRIMARY 6\n")
    f.write("#define PAL_SHIFT (NUM_PALS_IN_PRIMARY - RUBY_PALS_PRIMARY)\n")
    f.write("#define TILES_PER_METATILE 8\n\n")

    # Metatile count defines
    for name, kind, tc, mc in TILESETS:
        f.write(f"#define {name.upper()}_METATILE_COUNT {mc}\n")
    f.write("\n")

    # Tileset externs
    for name, kind, tc, mc in TILESETS:
        f.write(f"extern const u16 gHoennTilesetTiles_{name}[];\n")
        f.write(f"extern const u16 gHoennTilesetPalettes_{name}[][16];\n")
        f.write(f"extern const u16 gHoennMetatiles_{name}[];\n")
        f.write(f"extern const u16 gHoennRawMetatileAttrs_{name}[];\n")
    f.write("\n")

    # Layout externs
    emitted_layouts = set()
    for dirname, minfo in sorted(all_maps.items()):
        lname = minfo['layout']
        if lname not in emitted_layouts:
            emitted_layouts.add(lname)
            f.write(f"extern const u16 gHoennLayout_{lname}_Border[];\n")
            f.write(f"extern const u16 gHoennLayout_{lname}_Map[];\n")
    f.write("\n")

    # Static arrays - attrs, metatiles, palettes
    for name, kind, tc, mc in TILESETS:
        if kind == 'primary':
            f.write(f"static u32 sAttrs_{name}[NUM_METATILES_IN_PRIMARY];\n")
        else:
            f.write(f"static u32 sAttrs_{name}[NUM_METATILES_TOTAL - NUM_METATILES_IN_PRIMARY];\n")
        f.write(f"static u16 sMeta_{name}[{mc} * TILES_PER_METATILE];\n")
        if kind == 'secondary':
            f.write(f"static u16 sPals_{name}[16][16];\n")
    f.write("\n")

    # Map block arrays
    for lname in sorted(emitted_layouts):
        li = layouts[lname]
        f.write(f"static u16 sMap_{lname}[{li['w']} * {li['h']}];\n")
    f.write("\nstatic bool8 sInited = FALSE;\n\n")

    # ConvertRubyAttr
    f.write("static u32 ConvertRubyAttr(u16 raw) {\n")
    f.write("    u32 behavior = raw & 0xFF;\n    u32 layerType = (raw >> 12) & 3;\n")
    f.write("    u32 encounterType = 0;\n")
    f.write("    if (behavior == 0x02) encounterType = 1;\n")
    f.write("    else if (behavior == 0x03) encounterType = 1;\n")
    f.write("    else if (behavior == 0x06) encounterType = 1;\n")
    f.write("    else if (behavior == 0x08) encounterType = 1;\n")
    f.write("    else if (behavior == 0x0B) encounterType = 1;\n")
    f.write("    else if (behavior == 0x24) encounterType = 1;\n")
    f.write("    else if (behavior == 0x10) encounterType = 2;\n")
    f.write("    else if (behavior == 0x11) encounterType = 2;\n")
    f.write("    else if (behavior == 0x12) encounterType = 2;\n")
    f.write("    else if (behavior == 0x15) encounterType = 2;\n")
    f.write("    return behavior | (encounterType << 24) | (layerType << 29);\n}\n\n")

    # ShiftTile
    f.write("static u16 ShiftTile(u16 e) {\n")
    f.write("    u16 t = e & 0x3FF, fl = e & 0x0C00, p = (e >> 12) & 0xF;\n")
    f.write("    if (p >= RUBY_PALS_PRIMARY) p += PAL_SHIFT;\n")
    f.write("    return t | fl | (p << 12);\n}\n\n")

    # CopyBlocks
    f.write("static void CopyBlocks(const u16 *s, u16 *d, int n) {\n")
    f.write("    for (int i = 0; i < n; i++) d[i] = s[i];\n}\n\n")

    # Tileset struct declarations
    for name, kind, tc, mc in TILESETS:
        f.write(f"struct Tileset gHoennTileset_{name};\n")
    f.write("\n")

    # IsHoennTileset
    f.write("int Hoenn_IsHoennTileset(const struct Tileset *ts) {\n")
    for name, _, _, _ in TILESETS:
        f.write(f"    if (ts == &gHoennTileset_{name}) return 1;\n")
    f.write("    return 0;\n}\n\n")

    # GetTileCount
    f.write("int Hoenn_GetTileCount(const struct Tileset *ts) {\n")
    for name, _, tc, _ in TILESETS:
        f.write(f"    if (ts == &gHoennTileset_{name}) return {tc};\n")
    f.write("    return 512;\n}\n\n")

    # Hoenn_InitTilesets
    f.write("void Hoenn_InitTilesets(void) {\n    if (sInited) return;\n\n")

    # Convert attrs + metatiles
    for name, kind, tc, mc in TILESETS:
        f.write(f"    for (int i = 0; i < {mc}; i++) {{\n")
        f.write(f"        u16 r = gHoennRawMetatileAttrs_{name}[i];\n")
        f.write(f"        sAttrs_{name}[i] = ConvertRubyAttr(r);\n    }}\n")
        f.write(f"    for (int i = 0; i < {mc} * TILES_PER_METATILE; i++)\n")
        f.write(f"        sMeta_{name}[i] = ShiftTile(gHoennMetatiles_{name}[i]);\n")
        if kind == 'secondary':
            f.write(f"    memset(sPals_{name}, 0, sizeof(sPals_{name}));\n")
            f.write(f"    for (int i = 0; i < 6; i++)\n")
            f.write(f"        memcpy(sPals_{name}[i + NUM_PALS_IN_PRIMARY],\n")
            f.write(f"               gHoennTilesetPalettes_{name}[i + RUBY_PALS_PRIMARY], 32);\n")
        f.write("\n")

    # CopyBlocks for all layouts
    for lname in sorted(emitted_layouts):
        li = layouts[lname]
        f.write(f"    CopyBlocks(gHoennLayout_{lname}_Map, sMap_{lname}, {li['w']}*{li['h']});\n")

    f.write("\n")

    # Tileset struct setup
    for name, kind, tc, mc in TILESETS:
        is_sec = "TRUE" if kind == 'secondary' else "FALSE"
        pal_src = f"sPals_{name}" if kind == 'secondary' else f"gHoennTilesetPalettes_{name}"
        f.write(f"    gHoennTileset_{name}.isCompressed = FALSE;\n")
        f.write(f"    gHoennTileset_{name}.isSecondary = {is_sec};\n")
        f.write(f"    gHoennTileset_{name}.tiles = (const u32 *)gHoennTilesetTiles_{name};\n")
        f.write(f"    gHoennTileset_{name}.palettes = {pal_src};\n")
        f.write(f"    gHoennTileset_{name}.metatiles = sMeta_{name};\n")
        f.write(f"    gHoennTileset_{name}.callback = NULL;\n")
        f.write(f"    gHoennTileset_{name}.metatileAttributes = sAttrs_{name};\n\n")

    f.write("    sInited = TRUE;\n    printf(\"[HOENN] Data converted\\n\"); fflush(stdout);\n}\n\n")

    # Layout structs
    layout_id = 300
    layout_ids = {}
    for lname in sorted(emitted_layouts):
        li = layouts[lname]
        layout_ids[lname] = layout_id
        f.write(f"const struct MapLayout gHoennLayout_{lname} = {{\n")
        f.write(f"    .width = {li['w']}, .height = {li['h']}, .border = gHoennLayout_{lname}_Border,\n")
        f.write(f"    .map = sMap_{lname}, .primaryTileset = &gHoennTileset_{li['pri']},\n")
        f.write(f"    .secondaryTileset = &gHoennTileset_{li['sec']}, .borderWidth = 2, .borderHeight = 2,\n}};\n")
        layout_id += 1
    f.write("\n")

    # Empty script and events
    f.write("static const u8 sScript[] = { 0 };\n")
    f.write("static const struct MapEvents sNoEvents = {\n")
    f.write("    .objectEventCount = 0, .warpCount = 0, .coordEventCount = 0, .bgEventCount = 0,\n")
    f.write("    .objectEvents = NULL, .warps = NULL, .coordEvents = NULL, .bgEvents = NULL,\n};\n\n")

    # NPC data (from npc_data.h - just the arrays and externs)
    f.write("// ===== NPC DATA =====\n")
    for line in npc_content.split('\n'):
        if line.startswith('extern ') or line.startswith('static const struct ObjectEventTemplate') or \
           (line.startswith('    {') and '.localId' in line) or line.startswith('};'):
            f.write(line + '\n')
        elif line.strip() == '':
            f.write('\n')
    f.write("\n")

    # Warp data (from warp_data.h - only warp arrays, NOT event structs)
    f.write("// ===== WARP DATA =====\n")
    skip_events = False
    for line in warp_content.split('\n'):
        if line.startswith('//') and 'Auto-generated' in line:
            continue
        if '// Event structs with warps' in line:
            skip_events = True
            continue
        if '// Map header' in line:
            skip_events = True
            continue
        if skip_events:
            continue
        f.write(line + '\n')
    f.write("\n")

    # Generate combined event structs (warps + NPCs)
    f.write("// ===== EVENT STRUCTS (warps + NPCs combined) =====\n")
    # Parse warp arrays to know which maps have warps
    warp_arrays = {}
    warp_counts = {}
    for m in re.finditer(r'static const struct WarpEvent (sWarps_(\w+))\[\] = \{', warp_content):
        warp_arrays[m.group(2)] = m.group(1)
    for m in re.finditer(r'static const struct MapEvents sEvents_(\w+) = \{\s*\.objectEventCount = \d+, \.warpCount = (\d+)', warp_content):
        warp_counts[m.group(1)] = int(m.group(2))

    # Generate event structs for all maps
    all_event_maps = set(warp_arrays.keys()) | set(npc_updates.keys())
    for dirname in sorted(all_event_maps):
        warp_ref = "NULL"
        warp_count = 0
        npc_ref = "NULL"
        npc_count = 0
        if dirname in warp_arrays:
            warp_ref = warp_arrays[dirname]
            warp_count = warp_counts.get(dirname, 0)
        if dirname in npc_updates:
            npc_count, npc_arr = npc_updates[dirname]
            npc_ref = npc_arr

        f.write(f"static const struct MapEvents sEvents_{dirname} = {{\n")
        f.write(f"    .objectEventCount = {npc_count}, .warpCount = {warp_count}, .coordEventCount = 0, .bgEventCount = 0,\n")
        f.write(f"    .objectEvents = {npc_ref}, .warps = {warp_ref}, .coordEvents = NULL, .bgEvents = NULL,\n}};\n")
    f.write("\n")

    # Connection data (from connections.txt)
    f.write("// ===== CONNECTIONS =====\n")
    for line in conn_content.split('\n'):
        if line.startswith('//') and not line.startswith('// '):
            continue
        f.write(line + '\n')
    f.write("\n")

    # GetMapLayout
    f.write("const struct MapLayout *Hoenn_GetMapLayout(u16 id) {\n    switch (id) {\n")
    for lname, lid in sorted(layout_ids.items(), key=lambda x: x[1]):
        f.write(f"    case {lid}: return &gHoennLayout_{lname};\n")
    f.write("    default: return NULL;\n    }\n}\n\n")

    # Map headers
    f.write("// ===== MAP HEADERS =====\n")
    for dirname, minfo in sorted(all_maps.items()):
        data = minfo['json']
        lname = minfo['layout']
        if lname not in layout_ids: continue
        lid = layout_ids[lname]
        music = MUSIC.get(data.get('music', ''), 347)
        weather = WEATHER.get(data.get('weather', 'WEATHER_NONE'), 'WEATHER_NONE')
        maptype = MAPTYPE.get(data.get('map_type', 'MAP_TYPE_INDOOR'), 'MAP_TYPE_INDOOR')
        mapsec = MAPSEC.get(data.get('region_map_section', ''), 208)
        show_name = 1 if data.get('show_map_name', False) else 0

        # Events reference
        events_ref = '&sNoEvents'
        if dirname in warp_events:
            events_ref = f'&{warp_events[dirname]}'

        # NPC update
        if dirname in npc_updates:
            count, arr = npc_updates[dirname]
            # The events struct from warp_data.h already exists, but we need to update NPC fields
            # For simplicity, we'll handle this by creating new event structs that combine warps + NPCs
            pass  # handled below

        # Connection reference
        conn_ref = 'NULL'
        conn_var = f's{dirname}ConnData'
        if conn_var in conn_content:
            conn_ref = f'&{conn_var}'

        f.write(f"const struct MapHeader gHoennMapHeader_{dirname} = {{\n")
        f.write(f"    .mapLayout = &gHoennLayout_{lname}, .events = {events_ref},\n")
        f.write(f"    .mapScripts = sScript, .connections = {conn_ref},\n")
        f.write(f"    .music = {music}, .mapLayoutId = {lid}, .regionMapSectionId = {mapsec},\n")
        f.write(f"    .cave = FALSE, .weather = {weather}, .mapType = {maptype},\n")
        f.write(f"    .bikingAllowed = TRUE, .allowEscaping = 0, .allowRunning = 1, .showMapName = {show_name},\n")
        f.write(f"    .floorNum = 0, .battleType = 0,\n}};\n")
    f.write("\n")

    f.write("#endif // PORTABLE\n")

print(f"\nGenerated {OUT}")
print(f"  {len(TILESETS)} tilesets")
print(f"  {len(emitted_layouts)} layouts")
print(f"  {len(all_maps)} map headers")

# Check file size
sz = os.path.getsize(OUT)
print(f"  File size: {sz:,} bytes")
