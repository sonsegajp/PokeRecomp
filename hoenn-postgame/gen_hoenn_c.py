"""
Generate hoenn_data.c and hoenn_maps.c from pokeruby map.json files.
Produces: tileset structs, layout structs, map headers, connections, lookup tables.
"""
import os, re, json, glob

POKERUBY = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'pokeruby'))
OUTDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'src', 'platform'))
HOENN_GROUP_BASE = 43

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

def parse_layouts():
    layouts = {}
    path = os.path.join(POKERUBY, 'data', 'layouts', 'layouts.inc')
    with open(path, 'r') as f:
        lines = f.readlines()
    i = 0
    while i < len(lines):
        m = re.match(r'^(\w+)_Layout::', lines[i].strip())
        if m:
            name = m.group(1)
            width = height = 0
            primary_ts = secondary_ts = ""
            for j in range(i+1, min(i+10, len(lines))):
                l = lines[j].strip()
                if '.4byte' in l and 'Border' not in l and 'Blockdata' not in l:
                    val = l.split('.4byte')[1].strip()
                    if 'gTileset_' in val:
                        ts = val.replace('gTileset_', '')
                        if not primary_ts: primary_ts = ts
                        else: secondary_ts = ts
                    else:
                        try:
                            v = int(val)
                            if not width: width = v
                            elif not height: height = v
                        except: pass
            if width and height and primary_ts and secondary_ts:
                layouts[name] = {'width': width, 'height': height, 'primary': primary_ts, 'secondary': secondary_ts}
        i += 1
    return layouts

def resolve_tileset_name(camel, tilesets):
    snake = camel_to_snake(camel)
    if snake in tilesets: return snake
    for k in tilesets:
        if k.lower().replace('_', '') == camel.lower(): return k
    return None

def parse_map_groups():
    """Parse Ruby's map_groups.h to get group/map ordering"""
    groups = {}  # group_num -> [(map_num, map_id_name)]
    path = os.path.join(POKERUBY, 'include', 'constants', 'map_groups.h')
    with open(path, 'r') as f:
        for line in f:
            m = re.match(r'#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
            if m:
                name = m.group(1)
                map_num = int(m.group(2))
                group_num = int(m.group(3))
                if group_num not in groups:
                    groups[group_num] = []
                groups[group_num].append((map_num, name))
    for g in groups:
        groups[g].sort()
    return groups

def parse_all_map_jsons():
    """Parse all map.json files"""
    maps = {}
    maps_dir = os.path.join(POKERUBY, 'data', 'maps')
    for d in sorted(os.listdir(maps_dir)):
        json_path = os.path.join(maps_dir, d, 'map.json')
        if os.path.isfile(json_path):
            with open(json_path, 'r') as f:
                data = json.load(f)
            maps[d] = data
    return maps

# Direction string to CONNECTION_ constant
DIR_MAP = {'up': 'CONNECTION_NORTH', 'down': 'CONNECTION_SOUTH',
           'left': 'CONNECTION_WEST', 'right': 'CONNECTION_EAST',
           'dive': 'CONNECTION_DIVE', 'emerge': 'CONNECTION_EMERGE'}

# Ruby weather -> FireRed weather mapping
WEATHER_MAP = {
    'WEATHER_NONE': 'WEATHER_NONE',
    'WEATHER_CLOUDS': 'WEATHER_SUNNY_CLOUDS',
    'WEATHER_SUNNY': 'WEATHER_SUNNY',
    'WEATHER_RAIN_LIGHT': 'WEATHER_RAIN',
    'WEATHER_SNOW': 'WEATHER_SNOW',
    'WEATHER_RAIN_MED': 'WEATHER_RAIN_THUNDERSTORM',
    'WEATHER_FOG_1': 'WEATHER_FOG_HORIZONTAL',
    'WEATHER_ASH': 'WEATHER_VOLCANIC_ASH',
    'WEATHER_SANDSTORM': 'WEATHER_SANDSTORM',
    'WEATHER_FOG_2': 'WEATHER_FOG_DIAGONAL',
    'WEATHER_FOG_3': 'WEATHER_FOG_DIAGONAL',
    'WEATHER_SHADE': 'WEATHER_SHADE',
    'WEATHER_DROUGHT': 'WEATHER_DROUGHT',
    'WEATHER_RAIN_HEAVY': 'WEATHER_DOWNPOUR',
    'WEATHER_BUBBLES': 'WEATHER_UNDERWATER_BUBBLES',
    'WEATHER_UNDERWATER': 'WEATHER_UNDERWATER',
    'WEATHER_ROUTE119_CYCLE': 'WEATHER_ROUTE119_CYCLE',
    'WEATHER_ROUTE123_CYCLE': 'WEATHER_ROUTE123_CYCLE',
}

# Ruby map type -> FireRed map type mapping
MAP_TYPE_MAP = {
    'MAP_TYPE_TOWN': 'MAP_TYPE_TOWN',
    'MAP_TYPE_CITY': 'MAP_TYPE_CITY',
    'MAP_TYPE_ROUTE': 'MAP_TYPE_ROUTE',
    'MAP_TYPE_UNDERGROUND': 'MAP_TYPE_UNDERGROUND',
    'MAP_TYPE_UNDERWATER': 'MAP_TYPE_UNDERWATER',
    'MAP_TYPE_6': 'MAP_TYPE_OCEAN_ROUTE',
    'MAP_TYPE_OCEAN_ROUTE': 'MAP_TYPE_OCEAN_ROUTE',
    'MAP_TYPE_INDOOR': 'MAP_TYPE_INDOOR',
    'MAP_TYPE_SECRET_BASE': 'MAP_TYPE_SECRET_BASE',
}

# MAPSEC mapping (Ruby MAPSEC name -> Hoenn MAPSEC ID starting at 210)
MAPSEC_MAP = {}
MAPSEC_NAMES = {}
NEXT_MAPSEC = [210]

def get_hoenn_mapsec(ruby_mapsec_name):
    if ruby_mapsec_name not in MAPSEC_MAP:
        MAPSEC_MAP[ruby_mapsec_name] = NEXT_MAPSEC[0]
        # Convert MAPSEC_LITTLEROOT_TOWN -> "LITTLEROOT TOWN"
        nice = ruby_mapsec_name.replace('MAPSEC_', '').replace('_', ' ')
        MAPSEC_NAMES[NEXT_MAPSEC[0]] = nice
        NEXT_MAPSEC[0] += 1
    return MAPSEC_MAP[ruby_mapsec_name]

def map_id_to_dirname(map_id):
    """MAP_LITTLEROOT_TOWN -> LittlerootTown"""
    parts = map_id.replace('MAP_', '').split('_')
    return ''.join(p.capitalize() for p in parts)

def main():
    tilesets = get_tileset_info()
    layouts = parse_layouts()
    groups = parse_map_groups()
    all_maps = parse_all_map_jsons()

    print(f"Parsed {len(all_maps)} map JSONs, {len(groups)} groups, {len(layouts)} layouts, {len(tilesets)} tilesets")

    # Build map_id -> (hoenn_group, hoenn_map_num) mapping
    map_id_to_hoenn = {}
    for ruby_group, maps_in_group in sorted(groups.items()):
        hoenn_group = ruby_group + HOENN_GROUP_BASE
        for map_num, map_id in maps_in_group:
            map_id_to_hoenn[map_id] = (hoenn_group, map_num)

    # Resolve all tileset names (skip those without tile data)
    # Only include tilesets that have tile data
    ts_resolve = {}
    used_ts = set()
    for name, info in layouts.items():
        for ts_camel in [info['primary'], info['secondary']]:
            if ts_camel not in ts_resolve:
                resolved = resolve_tileset_name(ts_camel, tilesets)
                if resolved and tilesets[resolved]['has_tiles']:
                    ts_resolve[ts_camel] = resolved
                    used_ts.add(ts_camel)
    # Limit to only General + Petalburg for initial testing
    used_ts = {t for t in used_ts if t in ('General', 'Petalburg')}
    ts_resolve = {k: v for k, v in ts_resolve.items() if k in used_ts}

    # =========================================================================
    # Generate hoenn_data.c
    # =========================================================================
    data_path = os.path.join(OUTDIR, 'hoenn_data.c')
    with open(data_path, 'w') as f:
        f.write("// Auto-generated by hoenn-postgame/gen_hoenn_c.py\n")
        f.write("#ifdef PORTABLE\n")
        f.write('#include "global.h"\n#include "global.fieldmap.h"\n')
        f.write('#include "constants/hoenn.h"\n#include "constants/hoenn_map_groups.h"\n')
        f.write('#include "constants/weather.h"\n#include "constants/map_types.h"\n')
        f.write('#include "constants/maps.h"\n#include "constants/event_bg.h"\n')
        f.write('#include "fieldmap.h"\n#include <stdio.h>\n#include <string.h>\n\n')

        # Constants
        f.write("#define RUBY_TILES_PRIMARY 512\n")
        f.write("#define RUBY_PALS_PRIMARY 6\n")
        f.write("#define TILE_SHIFT (NUM_TILES_IN_PRIMARY - RUBY_TILES_PRIMARY)\n")
        f.write("#define PAL_SHIFT (NUM_PALS_IN_PRIMARY - RUBY_PALS_PRIMARY)\n")
        f.write("#define METATILE_SHIFT (NUM_METATILES_IN_PRIMARY - RUBY_TILES_PRIMARY)\n")
        f.write("#define TILES_PER_METATILE 8\n\n")

        # Extern declarations for binary data
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            f.write(f"extern const u16 gHoennTilesetTiles_{ts_camel}[];\n")
            f.write(f"extern const u16 gHoennTilesetPalettes_{ts_camel}[][16];\n")
            f.write(f"extern const u16 gHoennMetatiles_{ts_camel}[];\n")
            f.write(f"extern const u16 gHoennRawMetatileAttrs_{ts_camel}[];\n")
        f.write("\n")

        # Layout externs
        emitted_layouts = set()
        for dirname, mdata in sorted(all_maps.items()):
            layout_name = mdata.get('name', dirname)
            lkey = mdata.get('layout', '').replace('LAYOUT_', '')
            # Find layout key
            if layout_name in layouts:
                lname = layout_name
            else:
                lname = None
                for k in layouts:
                    if k.lower().replace('_', '') == layout_name.lower().replace('_', ''):
                        lname = k
                        break
            if lname and lname not in emitted_layouts:
                emitted_layouts.add(lname)
                f.write(f"extern const u16 gHoennLayout_{lname}_Border[];\n")
                f.write(f"extern const u16 gHoennLayout_{lname}_Map[];\n")
        f.write("\n")

        # Tileset structs
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            info = tilesets[ts_resolve[ts_camel]]
            f.write(f"struct Tileset gHoennTileset_{ts_camel};\n")
        f.write("\n")

        # Converted data arrays
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            info = tilesets[ts_resolve[ts_camel]]
            mc = info['metatile_count']
            if info['kind'] == 'primary':
                f.write(f"static u32 sAttrs_{ts_camel}[NUM_METATILES_IN_PRIMARY];\n")
                f.write(f"static u16 sMetatiles_{ts_camel}[{mc} * TILES_PER_METATILE];\n")
            else:
                f.write(f"static u32 sAttrs_{ts_camel}[{mc}];\n")
                f.write(f"static u16 sMetatiles_{ts_camel}[{mc} * TILES_PER_METATILE];\n")
                f.write(f"static u16 sPalettes_{ts_camel}[16][16];\n")
        f.write("\n")

        # Map block converted arrays (only for layouts with valid tilesets)
        valid_layouts = set()
        for lname, li in layouts.items():
            if li['primary'] in ts_resolve and li['secondary'] in ts_resolve:
                valid_layouts.add(lname)

        for lname in sorted(valid_layouts):
            w = layouts[lname]['width']
            h = layouts[lname]['height']
            f.write(f"static u16 sMap_{lname}[{w} * {h}];\n")
        f.write("\n")

        # ShiftTileEntry function
        f.write("static u16 ShiftTileEntry(u16 entry) {\n")
        f.write("    u16 t = entry & 0x3FF, fl = entry & 0x0C00, p = (entry >> 12) & 0xF;\n")
        f.write("    if (t >= RUBY_TILES_PRIMARY) t += TILE_SHIFT;\n")
        f.write("    if (p >= RUBY_PALS_PRIMARY) p += PAL_SHIFT;\n")
        f.write("    return t | fl | (p << 12);\n}\n\n")

        # ShiftMapBlocks function
        f.write("static void ShiftMapBlocks(const u16 *src, u16 *dst, int count) {\n")
        f.write("    for (int i = 0; i < count; i++) {\n")
        f.write("        u16 b = src[i], mt = b & 0x3FF, rest = b & 0xFC00;\n")
        f.write("        if (mt >= RUBY_TILES_PRIMARY) mt += METATILE_SHIFT;\n")
        f.write("        dst[i] = mt | rest;\n    }\n}\n\n")

        # IsHoennTileset
        f.write("int Hoenn_IsHoennTileset(const struct Tileset *ts) {\n")
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            f.write(f"    if (ts == &gHoennTileset_{ts_camel}) return 1;\n")
        f.write("    return 0;\n}\n\n")

        # GetTileCount
        f.write("int Hoenn_GetTileCount(const struct Tileset *ts) {\n")
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            tc = tilesets[ts_resolve[ts_camel]]['tile_count']
            f.write(f"    if (ts == &gHoennTileset_{ts_camel}) return {tc};\n")
        f.write("    return 512;\n}\n\n")

        # Init function
        f.write("static bool8 sInited = FALSE;\n")
        f.write("void Hoenn_InitTilesets(void) {\n")
        f.write("    if (sInited) return;\n\n")

        # Convert each tileset
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            info = tilesets[ts_resolve[ts_camel]]
            mc = info['metatile_count']

            # Attributes
            f.write(f"    for (int i = 0; i < {mc}; i++) {{\n")
            f.write(f"        u16 r = gHoennRawMetatileAttrs_{ts_camel}[i];\n")
            f.write(f"        sAttrs_{ts_camel}[i] = (r & 0xFF) | (((r >> 12) & 3) << 29);\n    }}\n")

            # Metatiles
            f.write(f"    for (int i = 0; i < {mc} * TILES_PER_METATILE; i++)\n")
            f.write(f"        sMetatiles_{ts_camel}[i] = ShiftTileEntry(gHoennMetatiles_{ts_camel}[i]);\n")

            # Palette shift for secondary
            if info['kind'] == 'secondary':
                f.write(f"    memset(sPalettes_{ts_camel}, 0, sizeof(sPalettes_{ts_camel}));\n")
                f.write(f"    for (int i = 0; i < 6; i++)\n")
                f.write(f"        memcpy(sPalettes_{ts_camel}[i + NUM_PALS_IN_PRIMARY],\n")
                f.write(f"               gHoennTilesetPalettes_{ts_camel}[i + RUBY_PALS_PRIMARY], 32);\n")
            f.write("\n")

        # Set tileset struct fields
        for ts_camel in sorted(used_ts):
            if ts_camel not in ts_resolve: continue
            info = tilesets[ts_resolve[ts_camel]]
            is_sec = "TRUE" if info['kind'] == 'secondary' else "FALSE"
            pal_src = f"sPalettes_{ts_camel}" if info['kind'] == 'secondary' else f"gHoennTilesetPalettes_{ts_camel}"
            f.write(f"    gHoennTileset_{ts_camel}.isCompressed = FALSE;\n")
            f.write(f"    gHoennTileset_{ts_camel}.isSecondary = {is_sec};\n")
            f.write(f"    gHoennTileset_{ts_camel}.tiles = (const u32 *)gHoennTilesetTiles_{ts_camel};\n")
            f.write(f"    gHoennTileset_{ts_camel}.palettes = {pal_src};\n")
            f.write(f"    gHoennTileset_{ts_camel}.metatiles = sMetatiles_{ts_camel};\n")
            f.write(f"    gHoennTileset_{ts_camel}.callback = NULL;\n")
            f.write(f"    gHoennTileset_{ts_camel}.metatileAttributes = sAttrs_{ts_camel};\n")

        # Convert map blocks (only valid layouts)
        f.write("\n")
        emitted_shifts = set()
        for lname in sorted(valid_layouts):
            if lname in emitted_shifts:
                continue
            emitted_shifts.add(lname)
            w = layouts[lname]['width']
            h = layouts[lname]['height']
            f.write(f"    ShiftMapBlocks(gHoennLayout_{lname}_Map, sMap_{lname}, {w}*{h});\n")

        f.write('\n    sInited = TRUE;\n')
        f.write('    printf("[HOENN] All tileset/layout data converted\\n"); fflush(stdout);\n')
        f.write("}\n\n")

        # Layout structs
        emitted_layouts2 = set()
        layout_id = 300
        layout_id_map = {}
        for dirname, mdata in sorted(all_maps.items()):
            layout_name = mdata.get('name', dirname)
            lname = None
            for k in layouts:
                if k.lower().replace('_', '') == layout_name.lower().replace('_', ''):
                    lname = k
                    break
            if not lname or lname in emitted_layouts2:
                continue
            emitted_layouts2.add(lname)
            li = layouts[lname]
            pri_ts = li['primary']
            sec_ts = li['secondary']
            if pri_ts not in ts_resolve or sec_ts not in ts_resolve:
                continue
            layout_id_map[lname] = layout_id
            f.write(f"const struct MapLayout gHoennLayout_{lname} = {{\n")
            f.write(f"    .width = {li['width']}, .height = {li['height']},\n")
            f.write(f"    .border = gHoennLayout_{lname}_Border,\n")
            f.write(f"    .map = sMap_{lname},\n")
            f.write(f"    .primaryTileset = &gHoennTileset_{pri_ts},\n")
            f.write(f"    .secondaryTileset = &gHoennTileset_{sec_ts},\n")
            f.write(f"    .borderWidth = 2, .borderHeight = 2,\n}};\n")
            layout_id += 1
        f.write("\n")

        # Empty script/events
        f.write("static const u8 sHoennEmptyScript[] = { 0 };\n")
        f.write("static const struct MapEvents sEmptyEvents = {\n")
        f.write("    .objectEventCount = 0, .warpCount = 0, .coordEventCount = 0, .bgEventCount = 0,\n")
        f.write("    .objectEvents = NULL, .warps = NULL, .coordEvents = NULL, .bgEvents = NULL,\n};\n\n")

        # Connections for each map
        for dirname, mdata in sorted(all_maps.items()):
            conns = mdata.get('connections', [])
            if not conns: continue
            map_id = mdata.get('id', '')
            f.write(f"static const struct MapConnection sConns_{dirname}[] = {{\n")
            for conn in conns:
                d = DIR_MAP.get(conn['direction'], 'CONNECTION_NORTH')
                offset = conn.get('offset', 0)
                dest_id = conn['map']
                if dest_id in map_id_to_hoenn:
                    dg, dm = map_id_to_hoenn[dest_id]
                else:
                    dg, dm = 43, 0
                f.write(f"    {{ .direction = {d}, .offset = {offset}, .mapGroup = {dg}, .mapNum = {dm} }},\n")
            f.write(f"}};\nstatic const struct MapConnections sConnData_{dirname} = {{ .count = {len(conns)}, .connections = sConns_{dirname} }};\n\n")

        # Map headers
        for dirname, mdata in sorted(all_maps.items()):
            layout_name = mdata.get('name', dirname)
            lname = None
            for k in layouts:
                if k.lower().replace('_', '') == layout_name.lower().replace('_', ''):
                    lname = k
                    break
            if not lname or lname not in layout_id_map:
                continue

            mapsec = get_hoenn_mapsec(mdata.get('region_map_section', 'MAPSEC_NONE'))
            raw_weather = mdata.get('weather', 'WEATHER_SUNNY')
            weather = WEATHER_MAP.get(raw_weather, 'WEATHER_SUNNY')
            raw_maptype = mdata.get('map_type', 'MAP_TYPE_ROUTE')
            maptype = MAP_TYPE_MAP.get(raw_maptype, 'MAP_TYPE_ROUTE')
            show_name = 1 if mdata.get('show_map_name', False) else 0
            conns = mdata.get('connections', [])
            conn_ref = "NULL"  # temporarily disable connections to debug crash

            f.write(f"const struct MapHeader gHoennHeader_{dirname} = {{\n")
            f.write(f"    .mapLayout = &gHoennLayout_{lname}, .events = &sEmptyEvents,\n")
            f.write(f"    .mapScripts = sHoennEmptyScript, .connections = {conn_ref},\n")
            f.write(f"    .music = 141, .mapLayoutId = {layout_id_map[lname]},\n")
            f.write(f"    .regionMapSectionId = {mapsec},\n")
            f.write(f"    .cave = FALSE, .weather = {weather}, .mapType = {maptype},\n")
            f.write(f"    .bikingAllowed = TRUE, .allowEscaping = 0, .allowRunning = 1, .showMapName = {show_name},\n")
            f.write(f"    .floorNum = 0, .battleType = 0,\n}};\n")
        f.write("\n")

        # Layout lookup
        f.write("const struct MapLayout *Hoenn_GetMapLayout(u16 layoutId) {\n")
        f.write("    switch (layoutId) {\n")
        for lname, lid in sorted(layout_id_map.items(), key=lambda x: x[1]):
            f.write(f"    case {lid}: return &gHoennLayout_{lname};\n")
        f.write(f"    default: return NULL;\n    }}\n}}\n\n")

        f.write("#endif // PORTABLE\n")

    print(f"Generated {data_path}")
    print(f"  {len(emitted_layouts2)} layouts, {len(all_maps)} map headers, {len(MAPSEC_MAP)} MAPSECs")

    # =========================================================================
    # Generate hoenn_maps.c (lookup table)
    # =========================================================================
    maps_path = os.path.join(OUTDIR, 'hoenn_maps.c')
    with open(maps_path, 'w') as f:
        f.write("// Auto-generated by hoenn-postgame/gen_hoenn_c.py\n")
        f.write("#ifdef PORTABLE\n")
        f.write('#include "global.h"\n#include "global.fieldmap.h"\n')
        f.write('#include "constants/hoenn.h"\n#include "constants/hoenn_map_groups.h"\n')
        f.write('#include <stdio.h>\n\n')

        # Extern declarations (only for maps with valid layouts)
        valid_map_dirnames = set()
        for dirname, mdata in all_maps.items():
            layout_name = mdata.get('name', dirname)
            for k in layouts:
                if k.lower().replace('_', '') == layout_name.lower().replace('_', ''):
                    if k in valid_layouts:
                        valid_map_dirnames.add(dirname)
                    break
        for dirname in sorted(valid_map_dirnames):
            f.write(f"extern const struct MapHeader gHoennHeader_{dirname};\n")
        f.write("\n")

        # Extern for layout/tileset funcs
        f.write("extern void Hoenn_InitTilesets(void);\n")
        f.write("extern const struct MapLayout *Hoenn_GetMapLayout(u16 layoutId);\n\n")

        # MAPSEC name lookup
        f.write("static const u8 *const sHoennMapsecNames[] = {\n")
        for mapsec_id in sorted(MAPSEC_NAMES.keys()):
            name = MAPSEC_NAMES[mapsec_id]
            f.write(f'    [{mapsec_id} - 210] = _("{name}"),\n')
        f.write("};\n\n")
        f.write(f"const u8 *Hoenn_GetMapsecName(u16 mapsec) {{\n")
        f.write(f"    if (mapsec >= 210 && mapsec < {NEXT_MAPSEC[0]})\n")
        f.write(f"        return sHoennMapsecNames[mapsec - 210];\n")
        f.write(f"    return NULL;\n}}\n\n")

        # Group arrays
        for ruby_group in sorted(groups.keys()):
            maps_in_group = groups[ruby_group]
            hoenn_group = ruby_group + HOENN_GROUP_BASE
            f.write(f"static const struct MapHeader *const sGroup_{hoenn_group}[] = {{\n")
            for map_num, map_id in maps_in_group:
                dirname = map_id_to_dirname(map_id)
                if dirname in valid_map_dirnames:
                    f.write(f"    [{map_num}] = &gHoennHeader_{dirname},\n")
                else:
                    f.write(f"    [{map_num}] = NULL, // {map_id}\n")
            f.write("};\n")
        f.write("\n")

        # Group size table
        f.write("static const struct { const struct MapHeader *const *headers; int count; } sHoennGroups[] = {\n")
        for ruby_group in sorted(groups.keys()):
            hoenn_group = ruby_group + HOENN_GROUP_BASE
            count = max(mn for mn, _ in groups[ruby_group]) + 1
            f.write(f"    [{ruby_group}] = {{ sGroup_{hoenn_group}, {count} }},\n")
        f.write("};\n")
        f.write(f"#define HOENN_NUM_GROUPS {len(groups)}\n\n")

        # Lookup function
        f.write("static bool8 sHoennInited = FALSE;\n\n")
        f.write("const struct MapHeader *Hoenn_GetMapHeader(u16 group, u16 num) {\n")
        f.write("    if (!sHoennInited) { Hoenn_InitTilesets(); sHoennInited = TRUE; }\n")
        f.write(f"    u16 hg = group - {HOENN_GROUP_BASE};\n")
        f.write("    if (hg < HOENN_NUM_GROUPS && num < sHoennGroups[hg].count)\n")
        f.write("        if (sHoennGroups[hg].headers[num])\n")
        f.write("            return sHoennGroups[hg].headers[num];\n")
        f.write("    return sGroup_43[9]; // fallback to Littleroot\n}\n\n")

        f.write("#endif // PORTABLE\n")

    print(f"Generated {maps_path}")
    print(f"  {len(groups)} groups, {sum(len(g) for g in groups.values())} total map entries")

if __name__ == '__main__':
    main()
