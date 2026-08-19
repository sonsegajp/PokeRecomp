"""
Hoenn sprite generator — Fresh start.
Uses FireRed's palette tags (0x1103-0x1106), overwrites color data when entering Hoenn.
"""
import re, os, json

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
POKEFR = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN_GFX = os.path.join(POKEFR, 'hoenn-postgame', 'graphics', 'object_events')
OUT_DIR = os.path.join(POKEFR, 'hoenn-postgame')

# 1. Parse Ruby's graphics info
rb_info_path = os.path.join(POKERUBY, 'src', 'data', 'object_events', 'object_event_graphics_info.h')
with open(rb_info_path) as f:
    content = f.read()

structs = re.findall(
    r'gObjectEventGraphicsInfo_(\w+)\s*=\s*\{[^}]*?'
    r'\.paletteTag\s*=\s*(0x[0-9a-fA-F]+).*?'
    r'\.size\s*=\s*(\d+).*?'
    r'\.width\s*=\s*(\d+).*?'
    r'\.height\s*=\s*(\d+).*?'
    r'\.inanimate\s*=\s*(\w+)',
    content, re.DOTALL)

rb_sprites = {}
for name, pal_tag, size, width, height, inanimate in structs:
    pal_num = int(pal_tag, 16) - 0x1100
    rb_sprites[name] = {'pal': pal_num, 'size': int(size), 'width': int(width), 'height': int(height), 'inanimate': inanimate == 'TRUE'}

# 2. Parse Ruby's GFX pointer table
rb_ptrs_path = os.path.join(POKERUBY, 'src', 'data', 'object_events', 'object_event_graphics_info_pointers.h')
with open(rb_ptrs_path) as f:
    ptr_content = f.read()
gfx_to_info = {}
for m in re.finditer(r'\[(OBJ_EVENT_GFX_\w+)\]\s*=\s*&gObjectEventGraphicsInfo_(\w+)', ptr_content):
    gfx_to_info[m.group(1)] = m.group(2)

# 3. Map to sprite files
pics_dir = os.path.join(HOENN_GFX, 'pics', 'people')
available = {f.replace('.4bpp',''): os.path.getsize(os.path.join(pics_dir, f))
             for f in os.listdir(pics_dir) if f.endswith('.4bpp')}

MANUAL = {
    'Boy1':'boy_1','Boy2':'boy_2','Boy3':'boy_3','Boy4':'boy_4','Boy5':'boy_5',
    'Girl1':'girl_1','Girl2':'girl_2','Girl3':'girl_3',
    'Man1':'man_1','Man2':'man_2','Man3':'man_3','Man4':'man_4','Man5':'man_5','Man6':'man_6','Man7':'man_7',
    'Woman1':'woman_1','Woman2':'woman_2','Woman3':'woman_3','Woman4':'woman_4',
    'Woman5':'woman_5','Woman6':'woman_6','Woman7':'woman_7','Woman8':'woman_8',
    'OldMan1':'old_man_1','OldMan2':'old_man_2','OldWoman1':'old_woman_1','OldWoman2':'old_woman_2',
    'LittleBoy1':'little_boy_1','LittleBoy2':'little_boy_2','LittleBoy3':'little_boy_3',
    'LittleGirl1':'little_girl_1','LittleGirl2':'little_girl_2',
    'AquaMemberM':'aqua_member_m','AquaMemberF':'aqua_member_f',
    'MagmaMemberM':'magma_member_m','MagmaMemberF':'magma_member_f',
    'Scientist1':'scientist_1','Scientist2':'scientist_2',
    'SwimmerM':'swimmer_m','SwimmerF':'swimmer_f',
    'TuberM':'tuber_m','TuberF':'tuber_f','TuberMSwimming':'tuber_m_swimming',
    'RunningTriathleteM':'running_triathlete_m','RunningTriathleteF':'running_triathlete_f',
    'CyclingTriathleteM':'cycling_triathlete_m','CyclingTriathleteF':'cycling_triathlete_f',
    'HotSpringsOldWoman':'hot_springs_old_woman',
    'MauvilleOldMan1':'mauville_old_man_1','MauvilleOldMan2':'mauville_old_man_2',
    'PsychicM':'psychic_m','ReporterF':'reporter_f','ReporterM':'reporter_m',
    'RooftopSaleWoman':'rooftop_sale_woman','SchoolKidM':'school_kid_m',
    'MartEmployee':'mart_employee','ProfBirch':'prof_birch',
    'ContestJudge':'contest_judge','FatMan':'fat_man',
    'HexManiac':'hex_maniac','BlackBelt':'black_belt','BugCatcher':'bug_catcher',
    'QuintyPlump':'quinty_plump',
}

def camel_to_snake(name):
    return re.sub(r'(?<=[a-z])(?=[A-Z0-9])', '_', name).lower()

sprite_map = {}
for gfx_name, info_name in gfx_to_info.items():
    if info_name not in rb_sprites: continue
    info = rb_sprites[info_name]
    snake = MANUAL.get(info_name, camel_to_snake(info_name))
    if snake not in available: continue
    file_size = available[snake]
    frame_size = (info['width'] // 8) * (info['height'] // 8) * 32
    frames = file_size // frame_size
    sprite_map[gfx_name] = {
        'file': snake, 'pal': info['pal'], 'width': info['width'], 'height': info['height'],
        'frames': frames, 'inanimate': info['inanimate'], 'info_name': info_name
    }

# 4. Filter to used sprites
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
used_gfx = set()
for dirname in os.listdir(maps_dir):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    for o in data.get('object_events', []):
        used_gfx.add(o.get('graphics_id', ''))

hoenn_sprites = {k: v for k, v in sorted(sprite_map.items()) if k in used_gfx and k not in {'OBJ_EVENT_GFX_ITEM_BALL'}}
print(f"Hoenn sprites: {len(hoenn_sprites)}")

# 5. Assign GFX IDs
HOENN_GFX_BASE = 152
gfx_id_map = {}
for i, gfx_name in enumerate(sorted(hoenn_sprites.keys())):
    gfx_id_map[gfx_name] = HOENN_GFX_BASE + i
NEW_NUM_GFX = HOENN_GFX_BASE + len(hoenn_sprites)
print(f"NUM_OBJ_EVENT_GFX: {NEW_NUM_GFX}")

# 6. Palette mapping — use FireRed's EXISTING tags
# Ruby pal 3 -> FR tag 0x1103, pal 4 -> 0x1104, pal 5 -> 0x1105, pal 6 -> 0x1106
FR_PAL_TAGS = {3: 0x1103, 4: 0x1104, 5: 0x1105, 6: 0x1106}
FR_PAL_SLOTS = {3: 1, 4: 2, 5: 3, 6: 4}
used_pals = sorted(set(s['pal'] for s in hoenn_sprites.values()))
print(f"Palettes used: {used_pals}")

# 7. Generate GFX ID mapping for NPC generator
gfx_ids_path = os.path.join(OUT_DIR, 'hoenn_gfx_ids.py')
with open(gfx_ids_path, 'w') as f:
    f.write("HOENN_GFX_IDS = {\n")
    for gfx_name, gfx_id in sorted(gfx_id_map.items(), key=lambda x: x[1]):
        f.write(f"    '{gfx_name}': {gfx_id},\n")
    f.write("}\n")
print(f"Generated {gfx_ids_path}")

# 8. Read Ruby's palette data as hex for embedding
pal_hex = {}
for p in used_pals:
    pal_file = os.path.join(HOENN_GFX, f'{p:02d}.gbapal')
    if os.path.isfile(pal_file):
        data = open(pal_file, 'rb').read()
        vals = [f'0x{data[i]|data[i+1]<<8:04X}' for i in range(0, 32, 2)]
        pal_hex[p] = ','.join(vals)

# 9. Generate hoenn_object_event_graphics.h
header_path = os.path.join(POKEFR, 'src', 'data', 'object_events', 'hoenn_object_event_graphics.h')
with open(header_path, 'w') as f:
    f.write("// Auto-generated Hoenn NPC sprites — uses FR palette tags, overwrites color data\n")
    f.write("#ifdef PORTABLE\n\n")

    # GFX ID defines
    for gfx_name, gfx_id in sorted(gfx_id_map.items(), key=lambda x: x[1]):
        hoenn_name = gfx_name.replace('OBJ_EVENT_GFX_', 'OBJ_EVENT_GFX_HOENN_')
        f.write(f"#define {hoenn_name} {gfx_id}\n")
    f.write(f"#define HOENN_GFX_BASE {HOENN_GFX_BASE}\n\n")

    # Embedded Ruby palette data
    f.write("// Ruby's NPC palette colors (embedded as hex)\n")
    for p in used_pals:
        if p in pal_hex:
            f.write(f"static const u16 sRubyPal_{p:02d}[] = {{{pal_hex[p]}}};\n")
    f.write("\n")

    # Sprite sheet data
    f.write("// Sprite sheet data\n")
    seen = set()
    for gfx_name in sorted(hoenn_sprites.keys()):
        s = hoenn_sprites[gfx_name]
        if s['file'] in seen: continue
        seen.add(s['file'])
        f.write(f"static const u16 sHoennPic_{s['file']}[] = INCBIN_U16(\"hoenn-postgame/graphics/object_events/pics/people/{s['file']}.4bpp\");\n")
    f.write("\n")

    # Pic tables
    f.write("// Pic tables\n")
    seen = set()
    for gfx_name in sorted(hoenn_sprites.keys()):
        s = hoenn_sprites[gfx_name]
        if s['file'] in seen: continue
        seen.add(s['file'])
        tw, th = s['width']//8, s['height']//8
        f.write(f"static const struct SpriteFrameImage sPicTable_Hoenn_{s['file']}[] = {{\n")
        for frame in range(s['frames']):
            f.write(f"    overworld_frame(sHoennPic_{s['file']}, {tw}, {th}, {frame}),\n")
        f.write("};\n\n")

    # Graphics info structs — using FR's palette tags
    f.write("// Graphics info (FR palette tags 0x1103-0x1106)\n")
    for gfx_name in sorted(hoenn_sprites.keys()):
        s = hoenn_sprites[gfx_name]
        pal_tag = FR_PAL_TAGS.get(s['pal'], 0x1105)
        pal_slot = FR_PAL_SLOTS.get(s['pal'], 3)
        oam = '16x32' if s['width'] == 16 else '32x32'
        frame_size = (s['width']//8) * (s['height']//8) * 32
        f.write(f"static const struct ObjectEventGraphicsInfo sHoennGfxInfo_{s['file']} = {{\n")
        f.write(f"    .tileTag = TAG_NONE, .paletteTag = 0x{pal_tag:04X}, .reflectionPaletteTag = 0x{pal_tag:04X},\n")
        f.write(f"    .size = {frame_size}, .width = {s['width']}, .height = {s['height']},\n")
        f.write(f"    .paletteSlot = {pal_slot}, .shadowSize = SHADOW_SIZE_M,\n")
        f.write(f"    .inanimate = {'TRUE' if s['inanimate'] else 'FALSE'}, .disableReflectionPaletteLoad = FALSE,\n")
        f.write(f"    .tracks = TRACKS_FOOT, .oam = &gObjectEventBaseOam_{oam},\n")
        f.write(f"    .subspriteTables = gObjectEventSpriteOamTables_{oam},\n")
        f.write(f"    .anims = sAnimTable_Standard, .images = sPicTable_Hoenn_{s['file']},\n")
        f.write(f"    .affineAnims = gDummySpriteAffineAnimTable,\n")
        f.write(f"}};\n\n")

    # Lookup table
    f.write(f"static const struct ObjectEventGraphicsInfo *const sHoennGfxInfoPtrs[{NEW_NUM_GFX - HOENN_GFX_BASE}] = {{\n")
    for gfx_name in sorted(hoenn_sprites.keys()):
        s = hoenn_sprites[gfx_name]
        gfx_id = gfx_id_map[gfx_name]
        f.write(f"    [{gfx_id - HOENN_GFX_BASE}] = &sHoennGfxInfo_{s['file']},\n")
    f.write("};\n\n")

    # Lookup function
    f.write("const struct ObjectEventGraphicsInfo *Hoenn_GetGraphicsInfo(u16 graphicsId) {\n")
    f.write(f"    if (graphicsId >= {HOENN_GFX_BASE} && graphicsId < {NEW_NUM_GFX})\n")
    f.write(f"        return sHoennGfxInfoPtrs[graphicsId - {HOENN_GFX_BASE}];\n")
    f.write("    return NULL;\n}\n\n")

    # Palette overwrite function — replaces FR's NPC palette COLORS with Ruby's
    f.write("void Hoenn_LoadSpritePalettes(void) {\n")
    f.write("    u8 slot;\n")
    for p in used_pals:
        if p not in FR_PAL_TAGS or p not in pal_hex: continue
        tag = FR_PAL_TAGS[p]
        f.write(f"    slot = IndexOfSpritePaletteTag(0x{tag:04X});\n")
        f.write(f"    if (slot < 16) LoadPalette(sRubyPal_{p:02d}, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);\n")
    f.write("}\n\n")

    f.write("#endif // PORTABLE\n")

print(f"Generated {header_path}")
