"""Generate C code additions for new Hoenn tilesets and maps.
Outputs code snippets to paste into hoenn_data.c and hoenn_maps.c."""
import os

POKERUBY = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'pokeruby'))

# New tilesets to add (name, snake_case_dir, tile_count, metatile_count)
NEW_TILESETS = [
    ("Rustboro", "rustboro", 498, 350),
    ("Dewford", "dewford", 343, 144),
    ("Slateport", "slateport", 504, 364),
    ("Mauville", "mauville", 450, 434),
    ("Fortree", "fortree", 493, 267),
    ("Lilycove", "lilycove", 432, 351),
    ("Mossdeep", "mossdeep", 499, 363),
    ("Sootopolis", "sootopolis", 180, 254),
    ("EverGrande", "ever_grande", 320, 168),
    ("Fallarbor", "fallarbor", 374, 338),
    ("Lavaridge", "lavaridge", 348, 286),
    ("Pacifidlog", "pacifidlog", 504, 191),
]

# New maps: (name, width, height, tileset_name, layout_id, mapsec_id, map_type, group43_idx)
# map_type: T=town, C=city, R=route
NEXT_LAYOUT_ID = 306
NEXT_MAPSEC = 230

new_maps = []
# Routes with their tilesets
routes = [
    ("Route104", 40, 80, "Rustboro", "R", 19),
    ("Route105", 20, 80, "Dewford", "R", 20),
    ("Route106", 30, 20, "Dewford", "R", 21),
    ("Route107", 40, 20, "Dewford", "R", 22),
    ("Route108", 40, 20, "Slateport", "R", 23),
    ("Route109", 20, 60, "Slateport", "R", 24),
    ("Route110", 20, 100, "Mauville", "R", 25),
    ("Route111", 20, 140, "Mauville", "R", 26),
    ("Route112", 20, 60, "Lavaridge", "R", 27),
    ("Route113", 40, 20, "Fallarbor", "R", 28),
    ("Route114", 20, 80, "Fallarbor", "R", 29),
    ("Route115", 20, 80, "Fallarbor", "R", 30),
    ("Route116", 80, 20, "Rustboro", "R", 31),
    ("Route117", 60, 20, "Mauville", "R", 32),
    ("Route118", 40, 20, "Mauville", "R", 33),
    ("Route119", 20, 140, "Fortree", "R", 34),
    ("Route120", 20, 100, "Fortree", "R", 35),
    ("Route121", 80, 20, "Lilycove", "R", 36),
    ("Route122", 20, 40, "Lilycove", "R", 37),
    ("Route123", 80, 20, "Lilycove", "R", 38),
    ("Route124", 40, 80, "Mossdeep", "R", 39),
    ("Route125", 20, 40, "Mossdeep", "R", 40),
    ("Route126", 40, 80, "Mossdeep", "R", 41),
    ("Route127", 40, 80, "Mossdeep", "R", 42),
    ("Route128", 80, 40, "Mossdeep", "R", 43),
    ("Route129", 40, 40, "Mossdeep", "R", 44),
    ("Route130", 40, 40, "Pacifidlog", "R", 45),  # actually Cave but using Pacifidlog for now
    ("Route131", 40, 40, "Pacifidlog", "R", 46),
    ("Route132", 40, 40, "Pacifidlog", "R", 47),
    ("Route133", 60, 40, "Pacifidlog", "R", 48),
    ("Route134", 80, 40, "Pacifidlog", "R", 49),  # actually Underwater but using Pacifidlog
]

# Cities/towns
cities = [
    ("SlateportCity", 30, 30, "Slateport", "C", 1),
    ("MauvilleCity", 30, 30, "Mauville", "C", 2),
    ("RustboroCity", 30, 30, "Rustboro", "C", 3),
    ("FortreeCity", 30, 20, "Fortree", "C", 4),
    ("LilycoveCity", 40, 30, "Lilycove", "C", 5),
    ("MossdeepCity", 30, 30, "Mossdeep", "C", 6),
    ("SootopolisCity", 30, 30, "Sootopolis", "C", 7),
    ("EverGrandeCity", 30, 40, "EverGrande", "C", 8),
    ("DewfordTown", 20, 20, "Dewford", "T", 11),
    ("LavaridgeTown", 20, 20, "Lavaridge", "T", 12),
    ("FallarborTown", 20, 20, "Fallarbor", "T", 13),
    ("VerdanturfTown", 20, 20, "Mauville", "T", 14),
    ("PacifidlogTown", 20, 20, "Pacifidlog", "T", 15),
]

lid = NEXT_LAYOUT_ID
msid = NEXT_MAPSEC
all_entries = routes + cities
for name, w, h, ts, mt, idx in all_entries:
    new_maps.append((name, w, h, ts, lid, msid, mt, idx))
    lid += 1
    msid += 1

# Print externs for binary data
print("// === ADD TO EXTERNS SECTION ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"extern const u16 gHoennTilesetTiles_{ts_name}[];")
    print(f"extern const u16 gHoennTilesetPalettes_{ts_name}[][16];")
    print(f"extern const u16 gHoennMetatiles_{ts_name}[];")
    print(f"extern const u16 gHoennRawMetatileAttrs_{ts_name}[];")
print()
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"extern const u16 gHoennLayout_{name}_Border[];")
    print(f"extern const u16 gHoennLayout_{name}_Map[];")

# Print tileset defines
print("\n// === ADD TO DEFINES ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"#define {ts_name.upper()}_METATILE_COUNT {mc}")

# Print static arrays
print("\n// === ADD TO STATIC ARRAYS ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"static u32 sAttrs_{ts_name}[NUM_METATILES_TOTAL - NUM_METATILES_IN_PRIMARY];")
    print(f"static u16 sMeta_{ts_name}[{mc} * TILES_PER_METATILE];")
    print(f"static u16 sPals_{ts_name}[16][16];")
print()
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"static u16 sMap_{name}[{w} * {h}];")

# Print tileset struct declarations
print("\n// === ADD TO TILESET STRUCT DECLARATIONS ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"struct Tileset gHoennTileset_{ts_name};")

# Print IsHoennTileset additions
print("\n// === ADD TO Hoenn_IsHoennTileset ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"    if (ts == &gHoennTileset_{ts_name}) return 1;")

# Print GetTileCount additions
print("\n// === ADD TO Hoenn_GetTileCount ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"    if (ts == &gHoennTileset_{ts_name}) return {tc};")

# Print init code
print("\n// === ADD TO Hoenn_InitTilesets (conversion) ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"    for (int i = 0; i < {mc}; i++) {{")
    print(f"        u16 r = gHoennRawMetatileAttrs_{ts_name}[i];")
    print(f"        sAttrs_{ts_name}[i] = (r & 0xFF) | (((r >> 12) & 3) << 29);")
    print(f"    }}")
    print(f"    for (int i = 0; i < {mc} * TILES_PER_METATILE; i++)")
    print(f"        sMeta_{ts_name}[i] = ShiftTile(gHoennMetatiles_{ts_name}[i]);")
    print(f"    memset(sPals_{ts_name}, 0, sizeof(sPals_{ts_name}));")
    print(f"    for (int i = 0; i < 6; i++)")
    print(f"        memcpy(sPals_{ts_name}[i + NUM_PALS_IN_PRIMARY],")
    print(f"               gHoennTilesetPalettes_{ts_name}[i + RUBY_PALS_PRIMARY], 32);")
    print()

# Print tileset struct init
print("// === ADD TO Hoenn_InitTilesets (struct setup) ===")
for ts_name, _, tc, mc in NEW_TILESETS:
    print(f"    gHoennTileset_{ts_name}.isCompressed = FALSE;")
    print(f"    gHoennTileset_{ts_name}.isSecondary = TRUE;")
    print(f"    gHoennTileset_{ts_name}.tiles = (const u32 *)gHoennTilesetTiles_{ts_name};")
    print(f"    gHoennTileset_{ts_name}.palettes = sPals_{ts_name};")
    print(f"    gHoennTileset_{ts_name}.metatiles = sMeta_{ts_name};")
    print(f"    gHoennTileset_{ts_name}.callback = NULL;")
    print(f"    gHoennTileset_{ts_name}.metatileAttributes = sAttrs_{ts_name};")
    print()

# Print ShiftBlocks calls
print("// === ADD TO Hoenn_InitTilesets (ShiftBlocks) ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"    ShiftBlocks(gHoennLayout_{name}_Map, sMap_{name}, {w}*{h});")

# Print layout structs
print("\n// === ADD LAYOUT STRUCTS ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"const struct MapLayout gHoennLayout_{name} = {{")
    print(f"    .width = {w}, .height = {h}, .border = gHoennLayout_{name}_Border,")
    print(f"    .map = sMap_{name}, .primaryTileset = &gHoennTileset_General,")
    print(f"    .secondaryTileset = &gHoennTileset_{ts}, .borderWidth = 2, .borderHeight = 2,")
    print(f"}};")

# Print layout lookup
print("\n// === ADD TO Hoenn_GetMapLayout ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"    case {lid}: return &gHoennLayout_{name};")

# Print map headers
mtype_map = {"T": "MAP_TYPE_TOWN", "C": "MAP_TYPE_CITY", "R": "MAP_TYPE_ROUTE"}
print("\n// === ADD MAP HEADERS ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    mtype = mtype_map[mt]
    print(f"const struct MapHeader gHoennMapHeader_{name} = {{")
    print(f"    .mapLayout = &gHoennLayout_{name}, .events = &sNoEvents,")
    print(f"    .mapScripts = sScript, .connections = NULL,")
    print(f"    .music = 141, .mapLayoutId = {lid}, .regionMapSectionId = {msid},")
    print(f"    .cave = FALSE, .weather = WEATHER_SUNNY, .mapType = {mtype},")
    print(f"    .bikingAllowed = TRUE, .allowEscaping = 0, .allowRunning = 1, .showMapName = 1,")
    print(f"    .floorNum = 0, .battleType = 0,")
    print(f"}};")

# Print MAPSEC names for hoenn_maps.c
print("\n// === MAPSEC NAMES (for hoenn_maps.c) ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    nice = name.replace("City", " CITY").replace("Town", " TOWN")
    # Convert CamelCase to SPACE SEPARATED
    import re
    nice = re.sub(r'([a-z])([A-Z])', r'\1 \2', name).upper()
    nice = nice.replace("ROUTE", "ROUTE ")
    print(f'    case {msid}: return sName_{name};')

print("\n// === MAPSEC NAME STRINGS ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    import re
    nice = re.sub(r'([a-z])([A-Z])', r'\1 \2', name).upper()
    nice = nice.replace("ROUTE", "ROUTE ")
    print(f'static const u8 sName_{name}[] = _("{nice}");')

# Print group43 entries
print("\n// === GROUP 43 ENTRIES ===")
for name, w, h, ts, lid, msid, mt, idx in new_maps:
    print(f"    [{idx}] = &gHoennMapHeader_{name},")
