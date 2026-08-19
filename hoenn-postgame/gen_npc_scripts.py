"""Extract NPC dialogue from Ruby's map text.inc files and generate scripts."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

maps_dir = os.path.join(POKERUBY, 'data', 'maps')

# For each map, get NPC script references and find their first text
all_scripts = {}  # dirname -> [(npc_index, script_label, text_label, text_content)]

for dirname in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, dirname, 'map.json')
    tp = os.path.join(maps_dir, dirname, 'text.inc')
    sp = os.path.join(maps_dir, dirname, 'scripts.inc')

    if not os.path.isfile(jp): continue

    data = json.load(open(jp))
    npcs = data.get('object_events', [])
    if not npcs: continue

    # Read text file
    texts = {}
    if os.path.isfile(tp):
        with open(tp, 'r', errors='replace') as f:
            current_label = None
            current_text = []
            for line in f:
                line = line.rstrip()
                m = re.match(r'^(\w+)::?\s*$', line)
                if m:
                    if current_label and current_text:
                        texts[current_label] = '\n'.join(current_text)
                    current_label = m.group(1)
                    current_text = []
                elif current_label and '.string' in line:
                    m2 = re.search(r'\.string\s+"(.*)"', line)
                    if m2:
                        current_text.append(m2.group(1))
            if current_label and current_text:
                texts[current_label] = '\n'.join(current_text)

    # Read scripts to find which text each NPC script references
    script_texts = {}  # script_label -> first text label referenced
    if os.path.isfile(sp):
        with open(sp, 'r', errors='replace') as f:
            current_script = None
            for line in f:
                line = line.strip()
                m = re.match(r'^(\w+)::?\s*(@.*)?$', line)
                if m:
                    current_script = m.group(1)
                # Also match labels with @ address comments
                m = re.match(r'^(\w+)::\s+@', line)
                if m:
                    current_script = m.group(1)
                if current_script and current_script not in script_texts:
                    # Look for msgbox or message command with text reference
                    m2 = re.match(r'\s*(?:msgbox|message)\s+(\w+)', line)
                    if m2:
                        text_label = m2.group(1)
                        script_texts[current_script] = text_label

    # Match NPCs to their text
    npc_texts = []
    for i, npc in enumerate(npcs):
        script = npc.get('script', '')
        if not script or script == '0' or script == '0x0':
            continue
        text_label = script_texts.get(script, None)
        if text_label and text_label in texts:
            npc_texts.append((i, script, text_label, texts[text_label]))
        elif script in script_texts:
            text_label = script_texts[script]
            if text_label in texts:
                npc_texts.append((i, script, text_label, texts[text_label]))

    if npc_texts:
        all_scripts[dirname] = npc_texts

# Count
total_with_text = sum(len(v) for v in all_scripts.values())
print(f"Maps with NPC text: {len(all_scripts)}")
print(f"NPCs with dialogue: {total_with_text}")

# Generate assembly script output
outpath = os.path.join(os.path.dirname(__file__), 'hoenn_npc_dialogue.inc')
with open(outpath, 'w') as f:
    f.write("@ Auto-generated Hoenn NPC dialogue scripts\n\n")

    for dirname, npc_texts in sorted(all_scripts.items()):
        for npc_idx, script_label, text_label, text in npc_texts:
            safe_label = f"HoennScript_{dirname}_{npc_idx}"
            text_safe = f"HoennText_{dirname}_{npc_idx}"

            f.write(f"{safe_label}::\n")
            f.write(f"\tlock\n")
            f.write(f"\tfaceplayer\n")
            f.write(f"\tmsgbox {text_safe}, MSGBOX_DEFAULT\n")
            f.write(f"\trelease\n")
            f.write(f"\tend\n\n")

            # Write text - combine lines with proper GBA text format
            full_text = text.replace('\n', '')
            # Make sure it ends with $
            if not full_text.endswith('$'):
                full_text += '$'
            f.write(f"{text_safe}:\n")
            f.write(f'\t.string "{full_text}"\n\n')

# Generate a C mapping file: npc_index -> script label
map_path = os.path.join(os.path.dirname(__file__), 'hoenn_npc_script_map.h')
with open(map_path, 'w') as f:
    f.write("// Auto-generated NPC script mapping\n")
    f.write("// Maps (dirname, npc_index) -> script label\n\n")

    for dirname, npc_texts in sorted(all_scripts.items()):
        for npc_idx, script_label, text_label, text in npc_texts:
            safe_label = f"HoennScript_{dirname}_{npc_idx}"
            f.write(f"// {dirname} NPC {npc_idx}: {script_label}\n")
            f.write(f"extern const u8 {safe_label}[];\n")

    f.write("\n// Script assignment table\n")
    f.write("typedef struct { const char *map; u8 npcIdx; const u8 *script; } HoennNPCScriptEntry;\n")
    f.write("static const HoennNPCScriptEntry sHoennNPCScripts[] = {\n")
    for dirname, npc_texts in sorted(all_scripts.items()):
        for npc_idx, script_label, text_label, text in npc_texts:
            safe_label = f"HoennScript_{dirname}_{npc_idx}"
            f.write(f'    {{"{dirname}", {npc_idx}, {safe_label}}},\n')
    f.write(f"    {{NULL, 0, NULL}},\n")
    f.write("};\n")

print(f"Generated {outpath}")
print(f"Generated {map_path}")
