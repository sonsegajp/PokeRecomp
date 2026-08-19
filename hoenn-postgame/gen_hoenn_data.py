"""
Generate hoenn_bindata.s and hoenn_data.c from pokeruby source data.
Reads all map layouts, tilesets, and generates the complete data files.
"""
import os, re, json, struct

POKERUBY = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'pokeruby'))
OUTDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'src', 'platform'))

# Parse layouts.inc to get all layout info
def parse_layouts():
    layouts = {}
    path = os.path.join(POKERUBY, 'data', 'layouts', 'layouts.inc')
    with open(path, 'r') as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        # Match layout label like "LittlerootTown_Layout::"
        m = re.match(r'^(\w+)_Layout::', line)
        if m:
            name = m.group(1)
            # Read the layout fields
            width = height = 0
            primary_ts = secondary_ts = ""
            for j in range(i+1, min(i+10, len(lines))):
                l = lines[j].strip()
                if '.4byte' in l and 'Border' not in l and 'Blockdata' not in l:
                    val = l.split('.4byte')[1].strip()
                    if 'gTileset_' in val:
                        ts_name = val.replace('gTileset_', '')
                        if not primary_ts:
                            primary_ts = ts_name
                        else:
                            secondary_ts = ts_name
                    else:
                        try:
                            v = int(val)
                            if not width:
                                width = v
                            elif not height:
                                height = v
                        except:
                            pass

            if width and height and primary_ts and secondary_ts:
                layouts[name] = {
                    'width': width, 'height': height,
                    'primary': primary_ts, 'secondary': secondary_ts
                }
        i += 1
    return layouts

# Parse map_groups.h to get map->layout mapping
def parse_map_headers():
    maps = {}
    maps_dir = os.path.join(POKERUBY, 'data', 'maps')
    for d in sorted(os.listdir(maps_dir)):
        header = os.path.join(maps_dir, d, 'header.inc')
        if os.path.isfile(header):
            with open(header, 'r') as f:
                content = f.read()
            # Find layout reference
            m = re.search(r'\.4byte (\w+)_Layout', content)
            layout = m.group(1) if m else None
            # Find music
            m2 = re.search(r'\.2byte (MUS_\w+)', content)
            music = m2.group(1) if m2 else 'MUS_DUMMY'
            # Find weather
            m3 = re.search(r'\.byte (WEATHER_\w+)', content)
            weather = m3.group(1) if m3 else 'WEATHER_SUNNY'
            # Find map type
            m4 = re.search(r'\.byte (MAP_TYPE_\w+)', content)
            maptype = m4.group(1) if m4 else 'MAP_TYPE_ROUTE'

            maps[d] = {
                'layout': layout,
                'music': music,
                'weather': weather,
                'maptype': maptype,
            }
    return maps

# Get tileset info
def camel_to_snake(name):
    """Convert CamelCase to snake_case"""
    s = re.sub(r'([A-Z])', r'_\1', name).lower().lstrip('_')
    # Fix specific cases
    s = s.replace('brendans_mays', 'brendans_mays')
    return s

def get_tileset_info():
    tilesets = {}
    ts_base = os.path.join(POKERUBY, 'data', 'tilesets')
    for kind in ['primary', 'secondary']:
        kind_dir = os.path.join(ts_base, kind)
        if not os.path.isdir(kind_dir):
            continue
        for name in sorted(os.listdir(kind_dir)):
            ts_dir = os.path.join(kind_dir, name)
            if not os.path.isdir(ts_dir):
                continue
            tiles = os.path.join(ts_dir, 'tiles.4bpp')
            meta = os.path.join(ts_dir, 'metatiles.bin')
            attrs = os.path.join(ts_dir, 'metatile_attributes.bin')
            pals_dir = os.path.join(ts_dir, 'palettes')

            tile_count = os.path.getsize(tiles) // 32 if os.path.isfile(tiles) else 0
            meta_count = os.path.getsize(meta) // 16 if os.path.isfile(meta) else 0
            pal_count = len([f for f in os.listdir(pals_dir) if f.endswith('.gbapal')]) if os.path.isdir(pals_dir) else 0

            tilesets[name] = {
                'kind': kind,
                'tile_count': tile_count,
                'metatile_count': meta_count,
                'pal_count': pal_count,
                'has_tiles': os.path.isfile(tiles),
                'path': f'{kind}/{name}',
            }
    return tilesets

def main():
    layouts = parse_layouts()
    maps = parse_map_headers()
    tilesets = get_tileset_info()

    print(f"Found {len(layouts)} layouts, {len(maps)} maps, {len(tilesets)} tilesets")

    # Build CamelCase -> snake_case tileset name mapping
    ts_name_map = {}
    for ts_name in tilesets:
        ts_name_map[ts_name] = ts_name
    # Also try CamelCase versions
    for camel_name in set():
        pass  # will populate from layouts

    # Determine which tilesets are actually used
    used_tilesets = set()
    for name, info in layouts.items():
        used_tilesets.add(info['primary'])
        used_tilesets.add(info['secondary'])

    # Build CamelCase -> snake_case tileset mapping
    ts_resolve = {}
    for ts_camel in used_tilesets:
        snake = camel_to_snake(ts_camel)
        if snake in tilesets:
            ts_resolve[ts_camel] = snake
        elif ts_camel.lower() in tilesets:
            ts_resolve[ts_camel] = ts_camel.lower()
        else:
            # Try exact match
            for k in tilesets:
                if k.lower().replace('_', '') == ts_camel.lower():
                    ts_resolve[ts_camel] = k
                    break

    print(f"Used tilesets: {len(used_tilesets)}, resolved: {len(ts_resolve)}")
    for ts in sorted(used_tilesets):
        if ts in ts_resolve:
            info = tilesets[ts_resolve[ts]]
            print(f"  {ts} -> {ts_resolve[ts]}: {info['tile_count']} tiles, {info['metatile_count']} metatiles")
        else:
            print(f"  {ts}: UNRESOLVED")

    # Generate hoenn_bindata.s
    asm_path = os.path.join(OUTDIR, 'hoenn_bindata.s')
    with open(asm_path, 'w') as f:
        f.write("# Auto-generated Hoenn tileset and layout binary data\n")
        f.write("# Generated by hoenn-postgame/gen_hoenn_data.py\n\n")
        f.write("\t.data\n\t.align 4\n\n")

        # Emit tileset data for all used tilesets
        for ts_camel in sorted(used_tilesets):
            if ts_camel not in ts_resolve:
                continue
            ts_snake = ts_resolve[ts_camel]
            info = tilesets[ts_snake]
            ts_name = ts_camel  # Use CamelCase for symbol names
            if not info['has_tiles']:
                continue

            prefix = f"gHoenn"
            ts_path = f"../pokeruby/data/tilesets/{info['path']}"

            # Tiles
            f.write(f"# Tileset: {ts_name} ({info['kind']}, {info['tile_count']} tiles, {info['metatile_count']} metatiles)\n")
            f.write(f"\t.align 4\n")
            f.write(f"\t.global {prefix}TilesetTiles_{ts_name}\n")
            f.write(f"{prefix}TilesetTiles_{ts_name}:\n")
            f.write(f'\t.incbin "{ts_path}/tiles.4bpp"\n\n')

            # Palettes
            f.write(f"\t.align 4\n")
            f.write(f"\t.global {prefix}TilesetPalettes_{ts_name}\n")
            f.write(f"{prefix}TilesetPalettes_{ts_name}:\n")
            for p in range(info['pal_count']):
                f.write(f'\t.incbin "{ts_path}/palettes/{p:02d}.gbapal"\n')
            f.write("\n")

            # Metatiles
            f.write(f"\t.align 4\n")
            f.write(f"\t.global {prefix}Metatiles_{ts_name}\n")
            f.write(f"{prefix}Metatiles_{ts_name}:\n")
            f.write(f'\t.incbin "{ts_path}/metatiles.bin"\n\n')

            # Metatile attributes (raw Ruby u16)
            f.write(f"\t.align 4\n")
            f.write(f"\t.global {prefix}RawMetatileAttrs_{ts_name}\n")
            f.write(f"{prefix}RawMetatileAttrs_{ts_name}:\n")
            f.write(f'\t.incbin "{ts_path}/metatile_attributes.bin"\n\n')

        # Emit layout data (deduplicated - many maps share layouts)
        emitted_layouts = set()
        for map_name, map_info in sorted(maps.items()):
            layout_name = map_info['layout']
            if layout_name not in layouts:
                continue
            if layout_name in emitted_layouts:
                continue
            emitted_layouts.add(layout_name)

            border_path = f"../pokeruby/data/layouts/{layout_name}/border.bin"
            map_path = f"../pokeruby/data/layouts/{layout_name}/map.bin"

            # Check files exist
            abs_border = os.path.join(POKERUBY, 'data', 'layouts', layout_name, 'border.bin')
            abs_map = os.path.join(POKERUBY, 'data', 'layouts', layout_name, 'map.bin')
            if not os.path.isfile(abs_border) or not os.path.isfile(abs_map):
                continue

            f.write(f"# Layout: {layout_name} ({layouts[layout_name]['width']}x{layouts[layout_name]['height']})\n")
            f.write(f"\t.align 4\n")
            f.write(f"\t.global gHoennLayout_{layout_name}_Border\n")
            f.write(f"gHoennLayout_{layout_name}_Border:\n")
            f.write(f'\t.incbin "{border_path}"\n\n')

            f.write(f"\t.align 4\n")
            f.write(f"\t.global gHoennLayout_{layout_name}_Map\n")
            f.write(f"gHoennLayout_{layout_name}_Map:\n")
            f.write(f'\t.incbin "{map_path}"\n\n')

    print(f"Generated {asm_path}")

    # Count what we generated
    ts_count = len([t for t in used_tilesets if t in ts_resolve and tilesets[ts_resolve[t]]['has_tiles']])
    layout_count = len(set(m['layout'] for m in maps.values() if m['layout'] in layouts))
    print(f"  {ts_count} tilesets, {layout_count} layouts")

if __name__ == '__main__':
    main()
