"""Generate missing indoor map headers and lookup entries."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Read existing headers from hoenn_data.c
data_path = r'c:\Users\hyper\projects\pokefirered-native\pokefirered\src\platform\hoenn_data.c'
with open(data_path) as f:
    data_content = f.read()

existing_headers = set()
for m in re.finditer(r'const struct MapHeader gHoennMapHeader_(\w+)\s*=', data_content):
    existing_headers.add(m.group(1))

existing_layouts = set()
for m in re.finditer(r'const struct MapLayout gHoennLayout_(\w+)\s*=', data_content):
    existing_layouts.add(m.group(1))

print(f"Existing: {len(existing_headers)} headers, {len(existing_layouts)} layouts")

# Read ALL map.json files to find missing maps
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
missing_maps = []

for dirname in sorted(os.listdir(maps_dir)):
    if dirname in existing_headers:
        continue
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp):
        continue
    data = json.load(open(jp))
    layout_key = data.get('layout', '').replace('LAYOUT_', '')

    # Convert layout key to the name used in our system
    # LAYOUT_POKEMON_CENTER_1F -> PokemonCenter_1F
    parts = layout_key.split('_')
    layout_name = ''.join(p.capitalize() for p in parts)

    # Try to find matching layout
    found_layout = None
    for el in existing_layouts:
        if el.lower().replace('_', '') == layout_name.lower().replace('_', ''):
            found_layout = el
            break
    # Also try with underscores preserved
    if not found_layout:
        # Convert POKEMON_CENTER_1F -> PokemonCenter_1F preserving underscores between words
        # This is tricky - try direct match with existing
        for el in existing_layouts:
            if el.lower() == layout_key.lower().replace('_', ''):
                found_layout = el
                break

    if not found_layout:
        # Try the layout_key directly
        for el in existing_layouts:
            key_norm = layout_key.lower().replace('_', '')
            el_norm = el.lower().replace('_', '')
            if key_norm == el_norm:
                found_layout = el
                break

    if found_layout:
        missing_maps.append((dirname, found_layout, data))

print(f"Found {len(missing_maps)} missing maps with valid layouts")

# Generate output
outpath = os.path.join(os.path.dirname(__file__), 'missing_maps.txt')
with open(outpath, 'w') as f:
    # Map headers
    f.write("// === MISSING MAP HEADERS (add to hoenn_data.c before #endif) ===\n")
    for dirname, layout, data in missing_maps:
        lid = 0
        # Find layout ID from GetMapLayout switch
        m = re.search(rf'case (\d+): return &gHoennLayout_{re.escape(layout)};', data_content)
        if m:
            lid = int(m.group(1))
        else:
            lid = 350  # fallback

        f.write(f"const struct MapHeader gHoennMapHeader_{dirname} = {{\n")
        f.write(f"    .mapLayout = &gHoennLayout_{layout}, .events = &sNoEvents,\n")
        f.write(f"    .mapScripts = sScript, .connections = NULL,\n")
        f.write(f"    .music = 347, .mapLayoutId = {lid}, .regionMapSectionId = 208,\n")
        f.write(f"    .cave = FALSE, .weather = WEATHER_NONE, .mapType = MAP_TYPE_INDOOR,\n")
        f.write(f"    .bikingAllowed = FALSE, .allowEscaping = 0, .allowRunning = 1, .showMapName = 0,\n")
        f.write(f"    .floorNum = 0, .battleType = 0,\n")
        f.write(f"}};\n")

    # Lookup entries
    f.write("\n// === LOOKUP ENTRIES (add to Hoenn_GetMapHeader) ===\n")
    mg_path = os.path.join(POKERUBY, 'include', 'constants', 'map_groups.h')
    with open(mg_path) as mf:
        for line in mf:
            m = re.match(r'#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
            if m:
                map_id = m.group(1)
                map_num = int(m.group(2))
                ruby_group = int(m.group(3))
                hoenn_group = ruby_group + 43

                # Find matching dirname
                raw = map_id.replace('MAP_', '')
                for dirname, layout, data in missing_maps:
                    if dirname.lower().replace('_', '') == raw.lower().replace('_', ''):
                        f.write(f"    if (group == {hoenn_group} && num == {map_num}) return &gHoennMapHeader_{dirname};\n")
                        break

    # Extern declarations
    f.write("\n// === EXTERN DECLARATIONS (add to hoenn_maps.c) ===\n")
    for dirname, layout, data in missing_maps:
        f.write(f"extern const struct MapHeader gHoennMapHeader_{dirname};\n")

    # Warp events
    f.write("\n// === WARP EVENTS (check warp_data.h for these) ===\n")
    for dirname, layout, data in missing_maps:
        f.write(f"// {dirname}: events=sEvents_{dirname} (if in warp_data.h)\n")

print(f"Generated {outpath}")
