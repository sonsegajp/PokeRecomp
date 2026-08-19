import json, os
data = json.load(open(r'c:\Users\hyper\projects\pokefirered-native\pokeruby\data\maps\LittlerootTown\map.json'))
for i, npc in enumerate(data.get('object_events', [])):
    print(f"NPC {i}: script={npc.get('script', '')}")
