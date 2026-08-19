"""Generate C code for all remaining Hoenn indoor/cave/dungeon maps."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Already-added maps (from outdoor phase)
EXISTING = {
    'LittlerootTown', 'OldaleTown', 'PetalburgCity', 'Route101', 'Route102', 'Route103',
    'Route104', 'Route105', 'Route106', 'Route107', 'Route108', 'Route109',
    'Route110', 'Route111', 'Route112', 'Route113', 'Route114', 'Route115',
    'Route116', 'Route117', 'Route118', 'Route119', 'Route120', 'Route121',
    'Route122', 'Route123', 'Route124', 'Route125', 'Route126', 'Route127',
    'Route128', 'Route129', 'Route130', 'Route131', 'Route132', 'Route133', 'Route134',
    'SlateportCity', 'MauvilleCity', 'RustboroCity', 'FortreeCity',
    'LilycoveCity', 'MossdeepCity', 'SootopolisCity', 'EverGrandeCity',
    'DewfordTown', 'LavaridgeTown', 'FallarborTown', 'VerdanturfTown', 'PacifidlogTown',
}

# Tilesets we have (all 49)
VALID_TILESETS = {
    'General', 'Petalburg', 'Rustboro', 'Dewford', 'Slateport', 'Mauville',
    'Fortree', 'Lilycove', 'Mossdeep', 'Sootopolis', 'EverGrande', 'Fallarbor',
    'Lavaridge', 'Pacifidlog', 'Building', 'Cave', 'GenericBuilding', 'Facility',
    'Ship', 'Shop', 'Underwater', 'TrickHousePuzzle', 'EliteFour', 'CableClub',
    'BattleTower', 'PokemonCenter', 'MeteorFalls', 'Lab', 'BrendansMaysHouse',
    'SootopolisGym', 'OceanicMuseum', 'LilycoveMuseum', 'LavaridgeGym', 'BikeShop',
    'SeashoreHouse', 'RusturfTunnel', 'RustboroGym', 'PrettyPetalFlowerShop',
    'PokemonSchool', 'PokemonFanClub', 'PokemonDayCare', 'PetalburgGym', 'MossdeepGym',
    'MauvilleGym', 'MauvilleGameCorner', 'InsideOfTruck', 'FortreeGym', 'DewfordGym',
    'Contest',
}

# Parse layouts to get dimensions and tilesets
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

layouts = parse_layouts()

# Find all maps not yet added
new_maps = []
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
for d in sorted(os.listdir(maps_dir)):
    if d in EXISTING:
        continue
    jp = os.path.join(maps_dir, d, 'map.json')
    if not os.path.isfile(jp):
        continue
    data = json.load(open(jp))
    map_name = data.get('name', d)

    # Find layout
    lname = None
    for k in layouts:
        if k.lower().replace('_','') == map_name.lower().replace('_',''):
            lname = k
            break
    if not lname:
        continue

    li = layouts[lname]
    if li['pri'] not in VALID_TILESETS or li['sec'] not in VALID_TILESETS:
        continue

    new_maps.append((d, lname, li['w'], li['h'], li['pri'], li['sec']))

print(f"Found {len(new_maps)} new maps to add")

# Output the code
# Layout externs
print("\n// === LAYOUT EXTERNS ===")
emitted = set()
for d, lname, w, h, pri, sec in new_maps:
    if lname not in emitted:
        emitted.add(lname)
        print(f"extern const u16 gHoennLayout_{lname}_Border[];")
        print(f"extern const u16 gHoennLayout_{lname}_Map[];")

# sMap arrays
print("\n// === MAP ARRAYS ===")
emitted = set()
for d, lname, w, h, pri, sec in new_maps:
    if lname not in emitted:
        emitted.add(lname)
        print(f"static u16 sMap_{lname}[{w} * {h}];")

# CopyBlocks calls
print("\n// === COPYBLOCKS CALLS ===")
emitted = set()
for d, lname, w, h, pri, sec in new_maps:
    if lname not in emitted:
        emitted.add(lname)
        print(f"    CopyBlocks(gHoennLayout_{lname}_Map, sMap_{lname}, {w}*{h});")

# Layout structs
print("\n// === LAYOUT STRUCTS ===")
emitted = set()
lid = 350  # start after outdoor layouts
layout_ids = {}
for d, lname, w, h, pri, sec in new_maps:
    if lname not in emitted:
        emitted.add(lname)
        layout_ids[lname] = lid
        print(f"const struct MapLayout gHoennLayout_{lname} = {{")
        print(f"    .width = {w}, .height = {h}, .border = gHoennLayout_{lname}_Border,")
        print(f"    .map = sMap_{lname}, .primaryTileset = &gHoennTileset_{pri},")
        print(f"    .secondaryTileset = &gHoennTileset_{sec}, .borderWidth = 2, .borderHeight = 2,")
        print(f"}};")
        lid += 1

# GetMapLayout cases
print("\n// === GETMAPLAYOUT CASES ===")
for lname, lid in sorted(layout_ids.items(), key=lambda x: x[1]):
    print(f"    case {lid}: return &gHoennLayout_{lname};")

# Map headers
print("\n// === MAP HEADERS ===")
for d, lname, w, h, pri, sec in new_maps:
    if lname not in layout_ids:
        continue
    lid = layout_ids[lname]
    print(f"const struct MapHeader gHoennMapHeader_{d} = {{")
    print(f"    .mapLayout = &gHoennLayout_{lname}, .events = &sNoEvents,")
    print(f"    .mapScripts = sScript, .connections = NULL,")
    print(f"    .music = 347, .mapLayoutId = {lid}, .regionMapSectionId = 208,")
    print(f"    .cave = FALSE, .weather = WEATHER_NONE, .mapType = MAP_TYPE_INDOOR,")
    print(f"    .bikingAllowed = FALSE, .allowEscaping = 0, .allowRunning = 1, .showMapName = 0,")
    print(f"    .floorNum = 0, .battleType = 0,")
    print(f"}};")

# Group entries (need Ruby's map_groups.h to get proper group/index)
print(f"\n// Total: {len(new_maps)} new maps, {len(layout_ids)} unique layouts")
print(f"// Layout IDs: 350-{lid-1}")
