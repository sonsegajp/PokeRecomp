"""Generate warp event data for all Hoenn maps from Ruby's map.json files."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Build complete MAP_ID -> (group, num) mapping from Ruby's map_groups.h
MAP_TO_HOENN = {}
mg_path = os.path.join(POKERUBY, 'include', 'constants', 'map_groups.h')
with open(mg_path) as f:
    for line in f:
        m = re.match(r'#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
        if m:
            name = m.group(1)
            mn = int(m.group(2))
            gn = int(m.group(3))
            MAP_TO_HOENN[name] = (gn + 43, mn)

def map_id_to_dirname(map_id):
    parts = map_id.replace('MAP_', '').split('_')
    return ''.join(p.capitalize() for p in parts)

# Process all maps
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
all_warps = {}  # dirname -> [(x, y, elevation, warpId, destMapGroup, destMapNum)]

for d in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, d, 'map.json')
    if not os.path.isfile(jp):
        continue
    data = json.load(open(jp))
    warps = data.get('warp_events', [])
    if not warps:
        continue

    warp_list = []
    for w in warps:
        x = w.get('x', 0)
        y = w.get('y', 0)
        elev = w.get('elevation', 0)
        dest_map = w.get('dest_map', '')
        dest_warp = w.get('dest_warp_id', 0)

        if dest_map not in MAP_TO_HOENN:
            continue

        dg, dm = MAP_TO_HOENN[dest_map]
        warp_list.append((x, y, elev, dest_warp, dg, dm))

    if warp_list:
        all_warps[d] = warp_list

print(f"Found warps for {len(all_warps)} maps")

# Output warp arrays and event structs
outpath = os.path.join(os.path.dirname(__file__), 'warp_data.h')
with open(outpath, 'w') as f:
    f.write("// Auto-generated Hoenn warp data\n\n")

    for dirname, warps in sorted(all_warps.items()):
        f.write(f"static const struct WarpEvent sWarps_{dirname}[] = {{\n")
        for x, y, elev, wid, dg, dm in warps:
            f.write(f"    {{ .x = {x}, .y = {y}, .elevation = {elev}, .warpId = {wid}, .mapGroup = {dg}, .mapNum = {dm} }},\n")
        f.write(f"}};\n")

    f.write("\n// Event structs with warps\n")
    for dirname, warps in sorted(all_warps.items()):
        f.write(f"static const struct MapEvents sEvents_{dirname} = {{\n")
        f.write(f"    .objectEventCount = 0, .warpCount = {len(warps)}, .coordEventCount = 0, .bgEventCount = 0,\n")
        f.write(f"    .objectEvents = NULL, .warps = sWarps_{dirname}, .coordEvents = NULL, .bgEvents = NULL,\n")
        f.write(f"}};\n")

    # Output a mapping of dirname -> events struct name for updating map headers
    f.write("\n// Map header .events field updates:\n")
    for dirname in sorted(all_warps.keys()):
        f.write(f"// {dirname}: &sEvents_{dirname}\n")

print(f"Generated {outpath} with {sum(len(w) for w in all_warps.values())} total warps")
