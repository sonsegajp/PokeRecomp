"""
Hoenn NPC Generator — Fresh start.
Pulls ALL data from Ruby faithfully, generates scripts matching FireRed's exact format.
"""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
POKEFR = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN_DIR = os.path.join(POKEFR, 'hoenn-postgame')
MAPS_DIR = os.path.join(POKERUBY, 'data', 'maps')
HOENN_TRAINER_BASE = 768

# ========== 1. CONSTANTS FROM RUBY ==========

# Ruby trainer IDs
rb_trainers = {}
with open(os.path.join(POKERUBY, 'include', 'constants', 'opponents.h')) as f:
    for line in f:
        m = re.match(r'#define\s+(TRAINER_\w+)\s+(\d+)', line)
        if m: rb_trainers[m.group(1)] = int(m.group(2))

# Ruby item IDs
rb_items = {}
with open(os.path.join(POKERUBY, 'include', 'constants', 'items.h')) as f:
    for line in f:
        m = re.match(r'#define\s+(ITEM_\w+)\s+(\d+)', line)
        if m: rb_items[m.group(1)] = int(m.group(2))

# Ruby GFX IDs -> Hoenn sprite IDs
import importlib.util
_spec = importlib.util.spec_from_file_location("ids", os.path.join(HOENN_DIR, 'hoenn_gfx_ids.py'))
_mod = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_mod)
HOENN_GFX = _mod.HOENN_GFX_IDS

FR_GFX = {
    'OBJ_EVENT_GFX_ITEM_BALL': 92,
    'OBJ_EVENT_GFX_BRENDAN_NORMAL': 14, 'OBJ_EVENT_GFX_MAY_NORMAL': 15,
    'OBJ_EVENT_GFX_PIKACHU': 120,
    'OBJ_EVENT_GFX_BERRY_TREE': 95, 'OBJ_EVENT_GFX_BERRY_TREE_EARLY': 95,
    'OBJ_EVENT_GFX_BERRY_TREE_MID': 95, 'OBJ_EVENT_GFX_BERRY_TREE_LATE': 95,
    'OBJ_EVENT_GFX_BIRCHS_BAG': 92,
    'OBJ_EVENT_GFX_KECLEON_1': 92,
    'OBJ_EVENT_GFX_CABLE_CAR': 92, 'OBJ_EVENT_GFX_SS_TIDAL': 92,
    'OBJ_EVENT_GFX_MOVING_BOX': 92, 'OBJ_EVENT_GFX_POOCHYENA': 92,
}

def get_gfx(name):
    if name in HOENN_GFX: return HOENN_GFX[name]
    if name in FR_GFX: return FR_GFX[name]
    return 155  # BOY_1 fallback

MOVE_MAP = {
    'MOVEMENT_TYPE_NONE':0, 'MOVEMENT_TYPE_LOOK_AROUND':1,
    'MOVEMENT_TYPE_WANDER_AROUND':2, 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN':3,
    'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT':4, 'MOVEMENT_TYPE_FACE_UP':5,
    'MOVEMENT_TYPE_FACE_DOWN':6, 'MOVEMENT_TYPE_FACE_LEFT':7,
    'MOVEMENT_TYPE_FACE_RIGHT':8,
}

# ========== 2. PARSE RUBY SCRIPTS FOR TRAINER IDS & ITEM IDS ==========

def parse_trainer_ids(script_path):
    m = {}
    if not os.path.isfile(script_path): return m
    content = open(script_path, 'r', errors='replace').read()
    labels = list(re.finditer(r'^(\w+)::', content, re.MULTILINE))
    for i, lm in enumerate(labels):
        end = labels[i+1].start() if i+1 < len(labels) else len(content)
        block = content[lm.start():end]
        tm = re.search(r'trainerbattle[^\n]*?(\bTRAINER_\w+)', block)
        if tm and tm.group(1) in rb_trainers:
            m[lm.group(1)] = rb_trainers[tm.group(1)]
    return m

# Global item script search (item balls reference scripts in shared files)
global_item_map = {}
for root, dirs, files in os.walk(os.path.join(POKERUBY, 'data')):
    for fn in files:
        if not fn.endswith('.inc'): continue
        try: content = open(os.path.join(root, fn), 'r', errors='replace').read()
        except: continue
        for m in re.finditer(r'^(\w+)::?(.*?)(?=^\w+::?|\Z)', content, re.MULTILINE|re.DOTALL):
            im = re.search(r'(ITEM_\w+)', m.group(2))
            if im and im.group(1) in rb_items:
                global_item_map[m.group(1)] = rb_items[im.group(1)]
print(f"Global item map: {len(global_item_map)} scripts")

# ========== 3. PARSE TEXT FROM RUBY ==========

def parse_texts(path):
    texts = {}
    if not os.path.isfile(path): return texts
    label, lines = None, []
    for line in open(path, 'r', errors='replace'):
        line = line.rstrip()
        m = re.match(r'^(\w+)::?', line)
        if m and '.string' not in line:
            if label and lines: texts[label] = ''.join(lines)
            label, lines = m.group(1), []
        elif label:
            m2 = re.search(r'\.string\s+"(.*)"', line)
            if m2: lines.append(m2.group(1))
    if label and lines: texts[label] = ''.join(lines)
    return texts

def parse_script_texts(path):
    m = {}
    if not os.path.isfile(path): return m
    current = None
    for line in open(path, 'r', errors='replace'):
        s = line.strip()
        lm = re.match(r'^(\w+)::', s)
        if lm: current = lm.group(1)
        if current and current not in m:
            tm = re.match(r'msgbox\s+(\w+)|message\s+(\w+)', s)
            if tm: m[current] = tm.group(1) or tm.group(2)
    return m

# ========== 4. EXTRACT ALL NPCS FROM RUBY ==========

SKIP_GFX = {'OBJ_EVENT_GFX_PUSHABLE_BOULDER','OBJ_EVENT_GFX_BREAKABLE_ROCK',
            'OBJ_EVENT_GFX_CUTTABLE_TREE'}

# Special NPCs to include despite flags
INCLUDE_FLAGS = {'OBJ_EVENT_GFX_PROF_BIRCH','OBJ_EVENT_GFX_NURSE',
                 'OBJ_EVENT_GFX_MART_EMPLOYEE','OBJ_EVENT_GFX_BIRCHS_BAG',
                 'OBJ_EVENT_GFX_POOCHYENA'}
# Route101-only NPCs
ROUTE101_ONLY = {'OBJ_EVENT_GFX_BIRCHS_BAG','OBJ_EVENT_GFX_POOCHYENA'}

all_maps = {}  # dirname -> [npc_entries]
all_scripts = []  # (type, key, data) for script generation
stats = {'npcs':0,'trainers':0,'items':0,'nurses':0,'marts':0,'dialogue':0}

for dirname in sorted(os.listdir(MAPS_DIR)):
    jp = os.path.join(MAPS_DIR, dirname, 'map.json')
    if not os.path.isfile(jp): continue
    data = json.load(open(jp))
    npcs = data.get('object_events', [])
    if not npcs: continue

    texts = parse_texts(os.path.join(MAPS_DIR, dirname, 'text.inc'))
    script_texts = parse_script_texts(os.path.join(MAPS_DIR, dirname, 'scripts.inc'))
    trainer_ids = parse_trainer_ids(os.path.join(MAPS_DIR, dirname, 'scripts.inc'))

    npc_list = []
    for i, npc in enumerate(npcs):
        flag = str(npc.get('flag', '0'))
        gfx_name = npc.get('graphics_id', '')
        trainer_type = npc.get('trainer_type', 'TRAINER_TYPE_NONE')
        script_name = npc.get('script', '')

        # Route101-only filter
        if gfx_name in ROUTE101_ONLY and dirname != 'Route101':
            continue

        # Keep: flag=0, FLAG_ITEM_*, or special NPCs
        if flag != '0' and not flag.startswith('FLAG_ITEM') and gfx_name not in INCLUDE_FLAGS:
            continue
        if gfx_name in SKIP_GFX or gfx_name.startswith('OBJ_EVENT_GFX_VAR_'):
            continue

        gfx = get_gfx(gfx_name)
        if gfx == 0: continue

        idx = len(npc_list)
        key = f"{dirname}_{idx}"

        # Use Ruby's ORIGINAL script label if it exists and isn't "0x0"
        ruby_script = script_name if script_name and script_name != '0x0' and script_name != '0' else None

        # Count types for stats
        if gfx_name == 'OBJ_EVENT_GFX_NURSE': stats['nurses'] += 1
        elif gfx_name == 'OBJ_EVENT_GFX_MART_EMPLOYEE': stats['marts'] += 1
        elif trainer_type == 'TRAINER_TYPE_NORMAL': stats['trainers'] += 1
        elif gfx_name == 'OBJ_EVENT_GFX_ITEM_BALL': stats['items'] += 1
        elif ruby_script: stats['dialogue'] += 1

        npc_list.append({
            'localId': i + 1, 'gfx': gfx,
            'x': npc.get('x', 0), 'y': npc.get('y', 0),
            'elev': npc.get('elevation', 3),
            'movement': MOVE_MAP.get(npc.get('movement_type', ''), 6),
            'rx': npc.get('movement_range_x', 0),
            'ry': npc.get('movement_range_y', 0),
            'trainerType': 1 if trainer_type == 'TRAINER_TYPE_NORMAL' else 0,
            'ruby_script': ruby_script,
        })
        stats['npcs'] += 1

    if npc_list:
        all_maps[dirname] = npc_list

print(f"Maps: {len(all_maps)}, NPCs: {stats['npcs']}")
print(f"Trainers: {stats['trainers']}, Items: {stats['items']}, Nurses: {stats['nurses']}, Marts: {stats['marts']}, Dialogue: {stats['dialogue']}")

# ========== 5. GENERATE SCRIPTS (matching FR's exact format) ==========

script_path = os.path.join(HOENN_DIR, 'hoenn_npc_dialogue.inc')
with open(script_path, 'w') as f:
    f.write("@ Auto-generated Hoenn NPC scripts\n\n")

    for stype, key, sdata in all_scripts:
        if stype == 'dialogue':
            f.write(f"HoennScript_{key}::\n")
            f.write(f"\tmsgbox HoennText_{key}, MSGBOX_NPC\n")
            f.write(f"\tend\n\n")
            f.write(f"HoennText_{key}:\n")
            f.write(f'\t.string "{sdata}"\n\n')
        elif stype == 'trainer':
            f.write(f"HoennScript_{key}::\n")
            f.write(f"\ttrainerbattle_no_intro {sdata}, HoennDefeat_{key}\n")
            f.write(f"\tmsgbox HoennDefeat_{key}, MSGBOX_DEFAULT\n")
            f.write(f"\trelease\n")
            f.write(f"\tend\n\n")
            f.write(f'HoennDefeat_{key}:\n')
            f.write(f'\t.string "You\'re strong!$"\n\n')
        elif stype == 'item':
            f.write(f"HoennScript_{key}::\n")
            f.write(f"\tfinditem {sdata}, 1\n")
            f.write(f"\tend\n\n")

print(f"Generated {script_path}")

# ========== 6. GENERATE HOENN_COMMON.INC ==========

common_path = os.path.join(POKEFR, 'data', 'scripts', 'hoenn_common.inc')
with open(common_path, 'w') as f:
    f.write("@ Hoenn common scripts — matches FireRed's exact working format\n\n")

    # Generic NPC (fallback)
    f.write("EventScript_HoennGenericNPC::\n")
    f.write('\tmsgbox HoennGeneric_Text, MSGBOX_NPC\n')
    f.write("\tend\n\n")
    f.write('HoennGeneric_Text:\n')
    f.write('\t.string "...$"\n\n')

    # Nurse — calls the SAME shared nurse script FireRed uses
    f.write("EventScript_HoennNurse::\n")
    f.write("\tlock\n\tfaceplayer\n")
    f.write("\tcall EventScript_PkmnCenterNurse\n")
    f.write("\trelease\n\tend\n\n")

    # Mart — exact copy of FireRed's working Pewter mart format
    f.write("EventScript_HoennMart::\n")
    f.write("\tlock\n\tfaceplayer\n")
    f.write("\tmessage Text_MayIHelpYou\n")
    f.write("\twaitmessage\n")
    f.write("\tpokemart HoennMart_Items\n")
    f.write("\tmsgbox Text_PleaseComeAgain\n")
    f.write("\trelease\n\tend\n\n")
    f.write("\t.align 2\n")
    f.write("HoennMart_Items::\n")
    for item in ['ITEM_POKE_BALL','ITEM_GREAT_BALL','ITEM_ULTRA_BALL',
                 'ITEM_POTION','ITEM_SUPER_POTION','ITEM_HYPER_POTION',
                 'ITEM_MAX_POTION','ITEM_FULL_RESTORE','ITEM_REVIVE',
                 'ITEM_ANTIDOTE','ITEM_PARALYZE_HEAL','ITEM_FULL_HEAL',
                 'ITEM_ESCAPE_ROPE','ITEM_REPEL','ITEM_SUPER_REPEL',
                 'ITEM_MAX_REPEL','ITEM_NONE']:
        f.write(f"\t.2byte {item}\n")
    f.write("\trelease\n\tend\n\n")

print(f"Generated {common_path}")

# ========== 7. GENERATE HOENN_STORY.INC ==========

story_path = os.path.join(POKEFR, 'data', 'scripts', 'hoenn_story.inc')
with open(story_path, 'w') as f:
    f.write("@ Hoenn story — Birch Route 101 encounter + gym leaders + E4\n\n")

    # Birch encounter (faithful to Ruby: panicking, pick starter, fight Poochyena)
    f.write("EventScript_HoennBirch::\n")
    f.write("\tlock\n\tfaceplayer\n")
    f.write("\tgoto_if_set HFLAG_GOT_STARTER, HoennBirch_After\n")
    f.write("\tgoto_if_set HFLAG_MET_BIRCH, HoennBirch_Choose\n")
    f.write("\tsetflag HFLAG_MET_BIRCH\n")
    f.write('\tmsgbox HoennBirch_Help, MSGBOX_DEFAULT\n')
    f.write('\tmsgbox HoennBirch_PickOne, MSGBOX_DEFAULT\n')
    f.write("HoennBirch_Choose:\n")
    f.write('\tmsgbox HoennBirch_AskTreecko, MSGBOX_YESNO\n')
    f.write("\tcompare VAR_RESULT, 1\n\tgoto_if_eq HoennBirch_Treecko\n")
    f.write('\tmsgbox HoennBirch_AskTorchic, MSGBOX_YESNO\n')
    f.write("\tcompare VAR_RESULT, 1\n\tgoto_if_eq HoennBirch_Torchic\n")
    f.write('\tmsgbox HoennBirch_AskMudkip, MSGBOX_YESNO\n')
    f.write("\tcompare VAR_RESULT, 1\n\tgoto_if_eq HoennBirch_Mudkip\n")
    f.write('\tmsgbox HoennBirch_Hurry, MSGBOX_DEFAULT\n')
    f.write("\tgoto HoennBirch_Choose\n\n")

    for species in ['TREECKO','TORCHIC','MUDKIP']:
        f.write(f"HoennBirch_{species.title()}:\n")
        f.write(f"\tgivemon SPECIES_{species}, 5, ITEM_NONE\n")
        f.write(f"\tgoto HoennBirch_Battle\n\n")

    f.write("HoennBirch_Battle:\n")
    f.write("\tplayfanfare MUS_LEVEL_UP\n")
    f.write('\tmsgbox HoennBirch_GotIt, MSGBOX_DEFAULT\n')
    f.write("\twaitfanfare\n\tclosemessage\n")
    f.write("\tsetwildbattle SPECIES_POOCHYENA, 5, ITEM_NONE\n")
    f.write("\tdowildbattle\n")
    f.write("\tsetflag HFLAG_GOT_STARTER\n")
    f.write('\tmsgbox HoennBirch_Thanks, MSGBOX_DEFAULT\n')
    f.write('\tmsgbox HoennBirch_KeepIt, MSGBOX_DEFAULT\n')
    f.write("\trelease\n\tend\n\n")

    f.write("HoennBirch_After:\n")
    f.write('\tmsgbox HoennBirch_AfterText, MSGBOX_DEFAULT\n')
    f.write("\trelease\n\tend\n\n")

    # Birch bag — same script as talking to Birch
    f.write("EventScript_HoennBirchBag::\n")
    f.write("\tgoto EventScript_HoennBirch\n\tend\n\n")

    # Text
    for label, text in [
        ('HoennBirch_Help', 'H-help me!\\pI was studying POKeMON\\nin the field when a\\lPOOCHYENA attacked!\\pYou! You look like a\\nstrong trainer!$'),
        ('HoennBirch_PickOne', 'In my bag!\\nThere are POKeMON inside!\\pQuick, pick one and\\nhelp me!$'),
        ('HoennBirch_AskTreecko', 'Use TREECKO, the\\nWood Gecko POKeMON?$'),
        ('HoennBirch_AskTorchic', 'Use TORCHIC, the\\nChick POKeMON?$'),
        ('HoennBirch_AskMudkip', 'Use MUDKIP, the\\nMud Fish POKeMON?$'),
        ('HoennBirch_Hurry', 'Hurry! The POOCHYENA\\nis getting closer!$'),
        ('HoennBirch_GotIt', '{PLAYER} grabbed a\\nPOKeMON from the bag!$'),
        ('HoennBirch_Thanks', 'BIRCH: Whew!\\nThat was close!\\pThanks for saving me!\\pSo you are the CHAMPION\\nfrom KANTO that OAK\\ltold me about!$'),
        ('HoennBirch_KeepIt', "BIRCH: That POKeMON\\nseems to like you.\\pWhy don't you keep it?\\pHOENN has eight GYMs.\\nStart with ROXANNE in\\lRUSTBORO CITY!\\pCome visit my lab\\nin LITTLEROOT anytime!$"),
        ('HoennBirch_AfterText', "BIRCH: {PLAYER}!\\nHow's HOENN treating you?$"),
    ]:
        f.write(f'{label}:\n\t.string "{text}"\n\n')

    # Gym leaders + E4 + Steven
    gym_data = [
        (1033, 'HFLAG_DEFEATED_ROXANNE', 'HFLAG_BADGE01_GET', 'ROXANNE', "I'm ROXANNE, RUSTBORO's\\nGYM LEADER!\\pShow me your skills!$", "You're incredible!$", "STONE BADGE earned!$"),
        (1034, 'HFLAG_DEFEATED_BRAWLY', 'HFLAG_BADGE02_GET', 'BRAWLY', "I'm BRAWLY!\\nDEWFORD's GYM LEADER!\\pLet's ride this wave!$", "Whoa! Total wipeout!$", None),
        (1035, 'HFLAG_DEFEATED_WATTSON', 'HFLAG_BADGE03_GET', 'WATTSON', "Wahahahah!\\nI'm WATTSON!\\pCan you shock me?$", "Wahahahah!\\nYou got me!$", None),
        (1036, 'HFLAG_DEFEATED_FLANNERY', 'HFLAG_BADGE04_GET', 'FLANNERY', "I'm FLANNERY!\\nDon't underestimate me!$", "I lost?!$", None),
        (1037, 'HFLAG_DEFEATED_NORMAN', 'HFLAG_BADGE05_GET', 'NORMAN', "I'm NORMAN.\\nPETALBURG's GYM LEADER.\\pI won't go easy on you.$", "I see why you're\\nthe CHAMPION...$", None),
        (1038, 'HFLAG_DEFEATED_WINONA', 'HFLAG_BADGE06_GET', 'WINONA', "I am WINONA.\\pCan you soar above\\nmy team?$", "You truly soar\\nabove the rest...$", None),
        (1039, 'HFLAG_DEFEATED_LIZA_TATE', 'HFLAG_BADGE07_GET', 'TATE_LIZA', "TATE: Hehehe!\\nLIZA: Let's battle!$", "TATE: Wow!\\nLIZA: You beat us!$", None),
        (1040, 'HFLAG_DEFEATED_WALLACE', 'HFLAG_BADGE08_GET', 'WALLACE', "I am WALLACE.\\pLet us see whose art\\nis superior!$", "Magnificent!$", None),
    ]

    e4_data = [
        (1029, 'HFLAG_DEFEATED_SIDNEY', None, 'SIDNEY', "I'm SIDNEY!\\nLet's do this!$", "You're as tough\\nas they say!$"),
        (1030, 'HFLAG_DEFEATED_PHOEBE', None, 'PHOEBE', "I'm PHOEBE!\\nLet's battle!$", "My Ghost POKeMON\\ncouldn't scare you!$"),
        (1031, 'HFLAG_DEFEATED_GLACIA', None, 'GLACIA', "I am GLACIA.\\pCan your spirit\\nwithstand my ice?$", "You melted my\\nicy defenses...$"),
        (1032, 'HFLAG_DEFEATED_DRAKE', None, 'DRAKE', "I am DRAKE.\\nShow me your bonds!$", "Superb! Truly\\nunbreakable bonds!$"),
        (1103, 'HFLAG_DEFEATED_STEVEN', None, 'STEVEN', "I'm STEVEN, HOENN's\\nCHAMPION.\\pLet's see which region\\nraised the stronger\\ltrainer!$", "You truly are the\\nstrongest in both\\lregions!$"),
    ]

    for tid, dflag, bflag, name, intro, defeat, badge_text in gym_data:
        f.write(f"EventScript_HoennGym{name}::\n")
        f.write(f"\tlock\n\tfaceplayer\n")
        f.write(f"\tgoto_if_set {dflag}, HoennGym{name}_After\n")
        f.write(f"\tmsgbox HoennGym{name}_Intro, MSGBOX_DEFAULT\n")
        f.write(f"\ttrainerbattle_no_intro {tid}, HoennGym{name}_Defeat\n")
        f.write(f"\tsetflag {dflag}\n")
        if bflag: f.write(f"\tsetflag {bflag}\n")
        if badge_text:
            f.write(f"\tmsgbox HoennGym{name}_Badge, MSGBOX_DEFAULT\n")
        f.write(f"\trelease\n\tend\n\n")
        f.write(f"HoennGym{name}_After:\n")
        f.write(f'\tmsgbox HoennGym{name}_Defeat, MSGBOX_NPC\n\tend\n\n')
        f.write(f'HoennGym{name}_Intro:\n\t.string "{intro}"\n\n')
        f.write(f'HoennGym{name}_Defeat:\n\t.string "{defeat}"\n\n')
        if badge_text:
            f.write(f'HoennGym{name}_Badge:\n\t.string "{badge_text}"\n\n')

    for tid, dflag, _, name, intro, defeat in e4_data:
        f.write(f"EventScript_HoennE4{name}::\n")
        f.write(f"\tlock\n\tfaceplayer\n")
        f.write(f"\tgoto_if_set {dflag}, HoennE4{name}_After\n")
        f.write(f"\tmsgbox HoennE4{name}_Intro, MSGBOX_DEFAULT\n")
        f.write(f"\ttrainerbattle_no_intro {tid}, HoennE4{name}_Defeat\n")
        f.write(f"\tsetflag {dflag}\n")
        if name == 'STEVEN':
            f.write(f"\tsetflag HFLAG_SYS_GAME_CLEAR\n")
            f.write(f'\tmsgbox HoennE4{name}_Congrats, MSGBOX_DEFAULT\n')
        f.write(f"\trelease\n\tend\n\n")
        f.write(f"HoennE4{name}_After:\n")
        f.write(f'\tmsgbox HoennE4{name}_Defeat, MSGBOX_NPC\n\tend\n\n')
        f.write(f'HoennE4{name}_Intro:\n\t.string "{intro}"\n\n')
        f.write(f'HoennE4{name}_Defeat:\n\t.string "{defeat}"\n\n')
        if name == 'STEVEN':
            f.write(f'HoennE4STEVEN_Congrats:\n\t.string "Congratulations!\\pYou are the CHAMPION\\nof two regions!$"\n\n')

print(f"Generated {story_path}")

# ========== 8. GENERATE NPC DATA HEADER ==========

npc_path = os.path.join(HOENN_DIR, 'npc_data.h')
with open(npc_path, 'w') as f:
    f.write("// Auto-generated Hoenn NPC data — fresh from Ruby\n\n")

    # Collect all unique Ruby script labels for extern declarations
    all_ruby_scripts = set()
    for dirname, npcs in all_maps.items():
        for npc in npcs:
            if npc['ruby_script']:
                all_ruby_scripts.add(npc['ruby_script'])

    f.write("extern const u8 EventScript_HoennGenericNPC[];\n")
    for label in sorted(all_ruby_scripts):
        f.write(f"extern const u8 {label}[];\n")
    f.write("\n")

    for dirname, npcs in sorted(all_maps.items()):
        f.write(f"static const struct ObjectEventTemplate sNPCs_{dirname}[] = {{\n")
        for npc in npcs:
            # Use Ruby's ACTUAL script label, fall back to generic
            script = npc['ruby_script'] if npc['ruby_script'] else "EventScript_HoennGenericNPC"

            f.write(f"    {{ .localId = {npc['localId']}, .graphicsId = {npc['gfx']}, .kind = 0, ")
            f.write(f".x = {npc['x']}, .y = {npc['y']}, ")
            f.write(f".objUnion = {{ .normal = {{ .elevation = {npc['elev']}, ")
            f.write(f".movementType = {npc['movement']}, ")
            f.write(f".movementRangeX = {npc['rx']}, .movementRangeY = {npc['ry']}, ")
            f.write(f".trainerType = {npc['trainerType']}, .trainerRange_berryTreeId = 0 }} }}, ")
            f.write(f".script = {script}, .flagId = 0 }},\n")
        f.write(f"}};\n\n")

    f.write("// Event struct updates:\n")
    for dirname, npcs in sorted(all_maps.items()):
        f.write(f"// {dirname}: .objectEventCount = {len(npcs)}, .objectEvents = sNPCs_{dirname}\n")

print(f"Generated {npc_path}")
