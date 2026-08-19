"""Generate corrected connection data with proper offsets from Ruby map.json"""
import json, os, re

maps_dir = r'c:\Users\hyper\projects\pokefirered-native\pokeruby\data\maps'

# Ruby MAP_ID -> (group, mapNum) in our Hoenn space
# Group 43 map indices from Ruby's map_groups.h group 0
GROUP43_MAP = {
    'MAP_PETALBURG_CITY': 0, 'MAP_SLATEPORT_CITY': 1, 'MAP_MAUVILLE_CITY': 2,
    'MAP_RUSTBORO_CITY': 3, 'MAP_FORTREE_CITY': 4, 'MAP_LILYCOVE_CITY': 5,
    'MAP_MOSSDEEP_CITY': 6, 'MAP_SOOTOPOLIS_CITY': 7, 'MAP_EVER_GRANDE_CITY': 8,
    'MAP_LITTLEROOT_TOWN': 9, 'MAP_OLDALE_TOWN': 10, 'MAP_DEWFORD_TOWN': 11,
    'MAP_LAVARIDGE_TOWN': 12, 'MAP_FALLARBOR_TOWN': 13, 'MAP_VERDANTURF_TOWN': 14,
    'MAP_PACIFIDLOG_TOWN': 15,
    'MAP_ROUTE101': 16, 'MAP_ROUTE102': 17, 'MAP_ROUTE103': 18, 'MAP_ROUTE104': 19,
    'MAP_ROUTE105': 20, 'MAP_ROUTE106': 21, 'MAP_ROUTE107': 22, 'MAP_ROUTE108': 23,
    'MAP_ROUTE109': 24, 'MAP_ROUTE110': 25, 'MAP_ROUTE111': 26, 'MAP_ROUTE112': 27,
    'MAP_ROUTE113': 28, 'MAP_ROUTE114': 29, 'MAP_ROUTE115': 30, 'MAP_ROUTE116': 31,
    'MAP_ROUTE117': 32, 'MAP_ROUTE118': 33, 'MAP_ROUTE119': 34, 'MAP_ROUTE120': 35,
    'MAP_ROUTE121': 36, 'MAP_ROUTE122': 37, 'MAP_ROUTE123': 38, 'MAP_ROUTE124': 39,
    'MAP_ROUTE125': 40, 'MAP_ROUTE126': 41, 'MAP_ROUTE127': 42, 'MAP_ROUTE128': 43,
    'MAP_ROUTE129': 44, 'MAP_ROUTE130': 45, 'MAP_ROUTE131': 46, 'MAP_ROUTE132': 47,
    'MAP_ROUTE133': 48, 'MAP_ROUTE134': 49,
}

DIR_C = {'up': 'CONNECTION_NORTH', 'down': 'CONNECTION_SOUTH',
         'left': 'CONNECTION_WEST', 'right': 'CONNECTION_EAST'}

def map_id_to_varname(map_id):
    parts = map_id.replace('MAP_', '').split('_')
    return ''.join(p.capitalize() for p in parts)

# Collect all outdoor maps we have
our_maps = [
    'LittlerootTown', 'OldaleTown', 'PetalburgCity',
    'Route101', 'Route102', 'Route103',
    'Route104', 'Route105', 'Route106', 'Route107', 'Route108', 'Route109',
    'Route110', 'Route111', 'Route112', 'Route113', 'Route114', 'Route115',
    'Route116', 'Route117', 'Route118', 'Route119', 'Route120', 'Route121',
    'Route122', 'Route123', 'Route124', 'Route125', 'Route126', 'Route127',
    'Route128', 'Route129', 'Route130', 'Route131', 'Route132', 'Route133', 'Route134',
    'SlateportCity', 'MauvilleCity', 'RustboroCity', 'FortreeCity',
    'LilycoveCity', 'MossdeepCity', 'SootopolisCity', 'EverGrandeCity',
    'DewfordTown', 'LavaridgeTown', 'FallarborTown', 'VerdanturfTown', 'PacifidlogTown',
]

for dirname in our_maps:
    jp = os.path.join(maps_dir, dirname, 'map.json')
    if not os.path.isfile(jp):
        continue
    data = json.load(open(jp))
    conns = data.get('connections', [])
    if not conns:
        print(f"// {dirname}: no connections")
        continue

    # Filter to only connections to maps we have, skip dive/emerge
    valid_conns = []
    for c in conns:
        if c['direction'] not in DIR_C:
            continue
        dest = c['map']
        if dest in GROUP43_MAP:
            valid_conns.append(c)

    if not valid_conns:
        print(f"// {dirname}: no valid connections")
        continue

    print(f"static const struct MapConnection s{dirname}Conns[] = {{")
    for c in valid_conns:
        d = DIR_C[c['direction']]
        off = c['offset']
        dest = c['map']
        mn = GROUP43_MAP[dest]
        print(f"    {{ .direction = {d}, .offset = {off}, .mapGroup = 43, .mapNum = {mn} }},")
    print(f"}};")
    print(f"static const struct MapConnections s{dirname}ConnData = {{ {len(valid_conns)}, s{dirname}Conns }};")
    print()
