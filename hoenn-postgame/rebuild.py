"""Rebuild hoenn_data.c from all saved generator outputs."""
import os, re, json

BASE = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN = os.path.join(BASE, 'hoenn-postgame')
POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
OUT = os.path.join(BASE, 'src', 'platform', 'hoenn_data.c')

# We need to reconstruct from the additions.txt, indoor_additions.txt,
# connections.txt, warp_data.h, and npc_data_with_dialogue.h
# Plus the hand-written tileset/core code

# Read the NPC data with dialogue
npc_path = os.path.join(HOENN, 'npc_data.h')
with open(npc_path, 'r') as f:
    npc_raw = f.read()

# Extract extern declarations
npc_externs = []
npc_arrays = []
current_array = []
in_array = False

for line in npc_raw.split('\n'):
    if line.startswith('extern '):
        npc_externs.append(line)
    elif 'static const struct ObjectEventTemplate' in line:
        in_array = True
        current_array = [line]
    elif in_array:
        current_array.append(line)
        if line.startswith('};'):
            npc_arrays.append('\n'.join(current_array))
            in_array = False
            current_array = []

# Extract event update comments
event_updates = {}  # dirname -> (count, array_name)
for line in npc_raw.split('\n'):
    m = re.match(r'// (\w+): \.objectEventCount = (\d+), \.objectEvents = (sNPCs_\w+)', line)
    if m:
        event_updates[m.group(1)] = (int(m.group(2)), m.group(3))

print(f"NPC externs: {len(npc_externs)}")
print(f"NPC arrays: {len(npc_arrays)}")
print(f"Event updates: {len(event_updates)}")

# Read warp data
warp_path = os.path.join(HOENN, 'warp_data.h')
with open(warp_path, 'r') as f:
    warp_raw = f.read()

# Extract warp arrays and event structs
warp_section = []
event_section = []
in_events = False
for line in warp_raw.split('\n'):
    if line.startswith('// Auto-generated') or line.strip() == '':
        continue
    if '// Event structs' in line:
        in_events = True
        continue
    if '// Map header' in line:
        break
    if in_events:
        event_section.append(line)
    else:
        warp_section.append(line)

print(f"Warp lines: {len(warp_section)}")
print(f"Event struct lines: {len(event_section)}")

# Read connections
conn_path = os.path.join(HOENN, 'connections.txt')
with open(conn_path, 'r') as f:
    conn_raw = f.read()

conn_lines = [l for l in conn_raw.split('\n') if l.strip() and not l.startswith('//')]
print(f"Connection lines: {len(conn_lines)}")

print("\nAll data loaded. Reconstruction needs the tileset/layout/header code.")
print("This was built up manually over the session and isn't in a single generator.")
print("The deployed exe works - we need to reconstruct the source for continued development.")
