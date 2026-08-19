"""Generate NPC object events for Hoenn maps, mapping Ruby GFX IDs to FireRed."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
POKEFR = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'

# Read FireRed GFX ID values
fr_gfx = {}
with open(os.path.join(POKEFR, 'include', 'constants', 'event_objects.h')) as f:
    for line in f:
        m = re.match(r'#define (OBJ_EVENT_GFX_\w+)\s+(\d+)', line)
        if m:
            fr_gfx[m.group(1)] = int(m.group(2))

# Read Ruby GFX ID values
rb_gfx = {}
with open(os.path.join(POKERUBY, 'include', 'constants', 'event_objects.h')) as f:
    for line in f:
        m = re.match(r'#define (OBJ_EVENT_GFX_\w+)\s+(\d+)', line)
        if m:
            rb_gfx[m.group(1)] = int(m.group(2))

# Map Ruby GFX names to FireRed numeric values
# For shared names, use FireRed's value
# For Ruby-specific names, map to closest FireRed equivalent
GFX_FALLBACK = fr_gfx.get('OBJ_EVENT_GFX_BOY_1', 1)  # generic fallback

RUBY_TO_FR = {}
# Direct matches
for name, rb_val in rb_gfx.items():
    if name in fr_gfx:
        RUBY_TO_FR[name] = fr_gfx[name]

# Manual mappings for Ruby-specific sprites
manual_map = {
    'OBJ_EVENT_GFX_PROF_BIRCH': fr_gfx.get('OBJ_EVENT_GFX_PROF_OAK', GFX_FALLBACK),
    'OBJ_EVENT_GFX_BRENDAN_NORMAL': fr_gfx.get('OBJ_EVENT_GFX_RED_NORMAL', GFX_FALLBACK),
    'OBJ_EVENT_GFX_MAY_NORMAL': fr_gfx.get('OBJ_EVENT_GFX_GREEN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_ARCHIE': fr_gfx.get('OBJ_EVENT_GFX_GIOVANNI', GFX_FALLBACK),
    'OBJ_EVENT_GFX_MAXIE': fr_gfx.get('OBJ_EVENT_GFX_GIOVANNI', GFX_FALLBACK),
    'OBJ_EVENT_GFX_AQUA_MEMBER_M': fr_gfx.get('OBJ_EVENT_GFX_ROCKET_M', GFX_FALLBACK),
    'OBJ_EVENT_GFX_AQUA_MEMBER_F': fr_gfx.get('OBJ_EVENT_GFX_ROCKET_F', GFX_FALLBACK),
    'OBJ_EVENT_GFX_MAGMA_MEMBER_M': fr_gfx.get('OBJ_EVENT_GFX_ROCKET_M', GFX_FALLBACK),
    'OBJ_EVENT_GFX_MAGMA_MEMBER_F': fr_gfx.get('OBJ_EVENT_GFX_ROCKET_F', GFX_FALLBACK),
    'OBJ_EVENT_GFX_WALLY': fr_gfx.get('OBJ_EVENT_GFX_BOY_2', GFX_FALLBACK),
    'OBJ_EVENT_GFX_SCOTT': fr_gfx.get('OBJ_EVENT_GFX_GENTLEMAN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_BRAWLY': fr_gfx.get('OBJ_EVENT_GFX_BLACK_BELT', GFX_FALLBACK),
    'OBJ_EVENT_GFX_ROXANNE': fr_gfx.get('OBJ_EVENT_GFX_LASS', GFX_FALLBACK),
    'OBJ_EVENT_GFX_WATTSON': fr_gfx.get('OBJ_EVENT_GFX_FAT_MAN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_FLANNERY': fr_gfx.get('OBJ_EVENT_GFX_BEAUTY', GFX_FALLBACK),
    'OBJ_EVENT_GFX_NORMAN': fr_gfx.get('OBJ_EVENT_GFX_MAN_1', GFX_FALLBACK),
    'OBJ_EVENT_GFX_WINONA': fr_gfx.get('OBJ_EVENT_GFX_WOMAN_2', GFX_FALLBACK),
    'OBJ_EVENT_GFX_LIZA': fr_gfx.get('OBJ_EVENT_GFX_TWIN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_TATE': fr_gfx.get('OBJ_EVENT_GFX_TWIN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_WALLACE': fr_gfx.get('OBJ_EVENT_GFX_GENTLEMAN', GFX_FALLBACK),
    'OBJ_EVENT_GFX_STEVEN': fr_gfx.get('OBJ_EVENT_GFX_COOLTRAINER_M', GFX_FALLBACK),
    'OBJ_EVENT_GFX_MR_BRINEY': fr_gfx.get('OBJ_EVENT_GFX_OLD_MAN_1', GFX_FALLBACK),
    'OBJ_EVENT_GFX_BERRY_TREE': GFX_FALLBACK,
    'OBJ_EVENT_GFX_BREAKABLE_ROCK': GFX_FALLBACK,
    'OBJ_EVENT_GFX_PUSHABLE_BOULDER': fr_gfx.get('OBJ_EVENT_GFX_BOULDER', GFX_FALLBACK),
    'OBJ_EVENT_GFX_BIRCHS_BAG': fr_gfx.get('OBJ_EVENT_GFX_ITEM_BALL', GFX_FALLBACK),
    'OBJ_EVENT_GFX_CABLE_CAR': 0,  # hide
    'OBJ_EVENT_GFX_SS_TIDAL': 0,  # hide
    'OBJ_EVENT_GFX_BERRY_TREE': 0,  # hide
    'OBJ_EVENT_GFX_BREAKABLE_ROCK': 0,  # hide
}
RUBY_TO_FR.update(manual_map)

# GFX names that should be hidden (vehicles, objects, special effects)
SKIP_GFX = {'OBJ_EVENT_GFX_CABLE_CAR', 'OBJ_EVENT_GFX_SS_TIDAL', 'OBJ_EVENT_GFX_BERRY_TREE',
            'OBJ_EVENT_GFX_BREAKABLE_ROCK', 'OBJ_EVENT_GFX_PUSHABLE_BOULDER',
            'OBJ_EVENT_GFX_BIRCHS_BAG', 'OBJ_EVENT_GFX_TRUCK'}

def get_fr_gfx_id(ruby_name):
    if ruby_name in SKIP_GFX:
        return 0  # hide
    if ruby_name in RUBY_TO_FR:
        return RUBY_TO_FR[ruby_name]
    # Try without common suffixes — use base sprite
    for suffix in ['_NORMAL', '_FIELD_MOVE', '_MACH_BIKE', '_ACRO_BIKE', '_SURFING', '_UNDERWATER', '_RUNNING']:
        base = ruby_name.replace(suffix, '')
        if base in RUBY_TO_FR:
            return RUBY_TO_FR[base]
    # For bike/surfing variants that don't match, hide them
    if any(x in ruby_name for x in ['BIKE', 'SURFING', 'UNDERWATER', 'FIELD_MOVE']):
        return 0  # hide
    return GFX_FALLBACK

# Movement type mapping (most are identical between games)
MOVEMENT_MAP = {
    'MOVEMENT_TYPE_NONE': 0, 'MOVEMENT_TYPE_LOOK_AROUND': 1,
    'MOVEMENT_TYPE_WANDER_AROUND': 2, 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN': 3,
    'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT': 4, 'MOVEMENT_TYPE_FACE_UP': 5,
    'MOVEMENT_TYPE_FACE_DOWN': 6, 'MOVEMENT_TYPE_FACE_LEFT': 7,
    'MOVEMENT_TYPE_FACE_RIGHT': 8,
}

# Read all map NPCs
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
npc_data = {}  # dirname -> list of NPC structs

for dirname in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    npcs = data.get('object_events', [])
    if not npcs: continue

    npc_list = []
    for i, npc in enumerate(npcs):
        gfx = get_fr_gfx_id(npc.get('graphics_id', ''))
        x = npc.get('x', 0)
        y = npc.get('y', 0)
        elev = npc.get('elevation', 3)
        mt = npc.get('movement_type', 'MOVEMENT_TYPE_FACE_DOWN')
        movement = MOVEMENT_MAP.get(mt, 6)  # default FACE_DOWN
        rx = npc.get('movement_range_x', 0)
        ry = npc.get('movement_range_y', 0)
        trainer_type = npc.get('trainer_type', 'TRAINER_TYPE_NONE')
        flag = npc.get('flag', '0')

        # Convert flag - just use 0 for now (always visible)
        flag_val = 0

        if gfx == 0:
            continue  # skip hidden/unmappable NPCs
        npc_list.append({
            'localId': i + 1,
            'gfx': gfx, 'x': x, 'y': y, 'elev': elev,
            'movement': movement, 'rx': rx, 'ry': ry,
            'trainerType': 0, 'flag': flag_val,
        })

    if npc_list:
        npc_data[dirname] = npc_list

print(f"Maps with NPCs: {len(npc_data)}")
print(f"Total NPCs: {sum(len(v) for v in npc_data.values())}")

# Generate output
outpath = os.path.join(os.path.dirname(__file__), 'npc_data.h')
with open(outpath, 'w') as f:
    f.write("// Auto-generated Hoenn NPC object events\n")
    f.write("// Include in hoenn_data.c before the warp/event definitions\n\n")

    for dirname, npcs in sorted(npc_data.items()):
        f.write(f"static const struct ObjectEventTemplate sNPCs_{dirname}[] = {{\n")
        for npc in npcs:
            f.write(f"    {{ .localId = {npc['localId']}, .graphicsId = {npc['gfx']}, .kind = 0, ")
            f.write(f".x = {npc['x']}, .y = {npc['y']}, ")
            f.write(f".objUnion = {{ .normal = {{ .elevation = {npc['elev']}, ")
            f.write(f".movementType = {npc['movement']}, ")
            f.write(f".movementRangeX = {npc['rx']}, .movementRangeY = {npc['ry']}, ")
            f.write(f".trainerType = {npc['trainerType']}, .trainerRange_berryTreeId = 0 }} }}, ")
            f.write(f".script = NULL, .flagId = {npc['flag']} }},\n")
        f.write(f"}};\n")

    f.write("\n// Event struct updates (replace .objectEventCount and .objectEvents):\n")
    for dirname, npcs in sorted(npc_data.items()):
        f.write(f"// {dirname}: .objectEventCount = {len(npcs)}, .objectEvents = sNPCs_{dirname}\n")

print(f"Generated {outpath}")
