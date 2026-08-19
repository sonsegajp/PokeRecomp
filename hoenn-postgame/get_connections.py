import json, os
maps_dir = r'c:\Users\hyper\projects\pokefirered-native\pokeruby\data\maps'
for d in sorted(os.listdir(maps_dir)):
    jp = os.path.join(maps_dir, d, 'map.json')
    if not os.path.isfile(jp):
        continue
    data = json.load(open(jp))
    conns = data.get('connections', [])
    if not conns:
        continue
    mid = data.get('id', '')
    if not any(x in mid for x in ['ROUTE', 'TOWN', 'CITY']):
        continue
    if 'UNDERWATER' in mid:
        continue
    parts = []
    for c in conns:
        parts.append(f"{c['direction']}:{c['offset']}:{c['map']}")
    print(f"{d}: {' | '.join(parts)}")
