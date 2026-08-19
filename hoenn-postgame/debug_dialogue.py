import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
maps_dir = os.path.join(POKERUBY, 'data', 'maps')

# Test with LittlerootTown
dirname = 'LittlerootTown'
jp = os.path.join(maps_dir, dirname, 'map.json')
sp = os.path.join(maps_dir, dirname, 'scripts.inc')
tp = os.path.join(maps_dir, dirname, 'text.inc')

data = json.load(open(jp))
npcs = data.get('object_events', [])
print(f"NPCs in {dirname}:")
for i, npc in enumerate(npcs):
    print(f"  {i}: script={npc.get('script','')}")

# Parse scripts
print(f"\nScript -> text mappings:")
with open(sp, 'r') as f:
    current = None
    for line in f:
        stripped = line.strip()
        m = re.match(r'^(\w+)::', stripped)
        if m:
            current = m.group(1)
        if current:
            m2 = re.match(r'msgbox\s+(\w+)', stripped)
            if m2:
                print(f"  {current} -> {m2.group(1)}")
                current = None  # reset after first match

# Parse texts
print(f"\nText labels:")
with open(tp, 'r') as f:
    for line in f:
        m = re.match(r'^(\w+)::?\s*$', line.rstrip())
        if m:
            print(f"  {m.group(1)}")
