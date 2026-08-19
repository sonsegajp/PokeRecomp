"""
Rebuild hoenn_data.c from all generator outputs.
Combines: tilesets, layouts, NPC data (with dialogue), warps, connections, events, map headers.
"""
import os, re

BASE = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN = os.path.join(BASE, 'hoenn-postgame')

# Read the NPC data with dialogue scripts (the current broken file IS the NPC data)
npc_file = os.path.join(HOENN, 'npc_data_with_dialogue.h')
with open(npc_file, 'r') as f:
    npc_content = f.read()

# Extract just the NPC arrays and externs (skip the #ifdef and other stuff)
npc_lines = []
in_npc_section = False
for line in npc_content.split('\n'):
    if line.startswith('extern const u8 HoennScript_'):
        npc_lines.append(line)
    elif line.startswith('static const struct ObjectEventTemplate sNPCs_'):
        in_npc_section = True
        npc_lines.append(line)
    elif in_npc_section:
        npc_lines.append(line)
        if line.startswith('};'):
            in_npc_section = False
            npc_lines.append('')

# Read warp data
warp_file = os.path.join(HOENN, 'warp_data.h')
with open(warp_file, 'r') as f:
    warp_content = f.read()

# Read connection data
conn_file = os.path.join(HOENN, 'connections.txt')
with open(conn_file, 'r') as f:
    conn_content = f.read()

print(f"NPC lines: {len(npc_lines)}")
print(f"Warp data: {len(warp_content)} bytes")
print(f"Connection data: {len(conn_content)} bytes")
print("All source data loaded. Now need to reconstruct the full hoenn_data.c")
print("This requires the tileset/layout/header code which was built up manually.")
print("The deployed exe still works - source just needs reconstruction.")
