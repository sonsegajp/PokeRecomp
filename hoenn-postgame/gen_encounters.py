"""Generate Hoenn wild encounter entries for FireRed's gWildMonHeaders arrays."""
import json, os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Ruby MAP_ID -> Hoenn (group, mapNum)
GROUP43 = {
    'MAP_PETALBURG_CITY': (43,0), 'MAP_SLATEPORT_CITY': (43,1), 'MAP_MAUVILLE_CITY': (43,2),
    'MAP_RUSTBORO_CITY': (43,3), 'MAP_FORTREE_CITY': (43,4), 'MAP_LILYCOVE_CITY': (43,5),
    'MAP_MOSSDEEP_CITY': (43,6), 'MAP_SOOTOPOLIS_CITY': (43,7), 'MAP_EVER_GRANDE_CITY': (43,8),
    'MAP_LITTLEROOT_TOWN': (43,9), 'MAP_OLDALE_TOWN': (43,10), 'MAP_DEWFORD_TOWN': (43,11),
    'MAP_LAVARIDGE_TOWN': (43,12), 'MAP_FALLARBOR_TOWN': (43,13), 'MAP_VERDANTURF_TOWN': (43,14),
    'MAP_PACIFIDLOG_TOWN': (43,15),
}
for i in range(101, 135):
    GROUP43[f'MAP_ROUTE{i}'] = (43, i - 101 + 16)

# Parse Ruby's wild_encounters.json for the actual data
enc_json = os.path.join(POKERUBY, 'src', 'data', 'wild_encounters.json')
with open(enc_json) as f:
    data = json.load(f)

# Parse map_groups.h for indoor maps
mg_path = os.path.join(POKERUBY, 'include', 'constants', 'map_groups.h')
with open(mg_path) as f:
    for line in f:
        m = re.match(r'#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
        if m and m.group(1) not in GROUP43:
            name = m.group(1)
            mn = int(m.group(2))
            gn = int(m.group(3))
            GROUP43[name] = (gn + 43, mn)

# Process encounters
ruby_entries = []
sapphire_entries = []

for group in data.get('wild_encounter_groups', []):
    for enc in group.get('encounters', []):
        map_id = enc.get('map', '')
        if map_id not in GROUP43:
            continue

        hoenn_group, hoenn_num = GROUP43[map_id]

        # Check for Ruby/Sapphire specific entries
        base_label = enc.get('base_label', '')

        land = enc.get('land_mons', None)
        water = enc.get('water_mons', None)
        fishing = enc.get('fishing_mons', None)
        rock = enc.get('rock_smash_mons', None)

        if land or water or fishing or rock:
            entry = {
                'map_id': map_id,
                'group': hoenn_group,
                'num': hoenn_num,
                'label': base_label,
                'land': land,
                'water': water,
                'fishing': fishing,
                'rock': rock,
            }

            # Determine if Ruby or Sapphire specific
            if '_Ruby' in base_label or 'Ruby' in base_label:
                ruby_entries.append(entry)
            elif '_Sapphire' in base_label or 'Sapphire' in base_label:
                sapphire_entries.append(entry)
            else:
                ruby_entries.append(entry)
                sapphire_entries.append(entry)

print(f"// Ruby entries: {len(ruby_entries)}, Sapphire entries: {len(sapphire_entries)}")

# Generate encounter data arrays
def gen_mon_array(label, mons, suffix=""):
    if not mons:
        return None
    arr_name = f"sHoenn_{label}{suffix}_Mons"
    lines = [f"static const struct WildPokemon {arr_name}[] = {{"]
    for mon in mons:
        species = mon.get('species', 'SPECIES_NONE')
        min_lv = mon.get('min_level', 5)
        max_lv = mon.get('max_level', 5)
        lines.append(f"    {{ {min_lv}, {max_lv}, {species} }},")
    lines.append("};")
    return arr_name, '\n'.join(lines)

def gen_info(label, mons, kind, suffix=""):
    if not mons:
        return None
    arr_name = f"sHoenn_{label}{suffix}_Mons"
    info_name = f"sHoenn_{label}{suffix}_Info"
    rate = mons[0].get('encounter_rate', 20) if isinstance(mons, list) else 20
    # Get encounter rate from the encounter data
    return info_name, f"static const struct WildPokemonInfo {info_name} = {{ {rate}, {arr_name} }};"

# Output
all_arrays = []
all_headers_fr = []
all_headers_lg = []

for entry in ruby_entries:
    label = entry['label'].replace(' ', '_')
    g, n = entry['group'], entry['num']

    land_arr = water_arr = fish_arr = rock_arr = None
    land_info = water_info = fish_info = rock_info = None

    if entry['land']:
        mons = entry['land'].get('mons', [])
        rate = entry['land'].get('encounter_rate', 20)
        if mons:
            aname = f"sHoenn_{label}_LandMons"
            all_arrays.append(f"static const struct WildPokemon {aname}[] = {{")
            for m in mons:
                all_arrays.append(f"    {{ {m['min_level']}, {m['max_level']}, {m['species']} }},")
            all_arrays.append("};")
            iname = f"sHoenn_{label}_LandInfo"
            all_arrays.append(f"static const struct WildPokemonInfo {iname} = {{ {rate}, {aname} }};")
            land_info = f"&{iname}"

    if entry['water']:
        mons = entry['water'].get('mons', [])
        rate = entry['water'].get('encounter_rate', 4)
        if mons:
            aname = f"sHoenn_{label}_WaterMons"
            all_arrays.append(f"static const struct WildPokemon {aname}[] = {{")
            for m in mons:
                all_arrays.append(f"    {{ {m['min_level']}, {m['max_level']}, {m['species']} }},")
            all_arrays.append("};")
            iname = f"sHoenn_{label}_WaterInfo"
            all_arrays.append(f"static const struct WildPokemonInfo {iname} = {{ {rate}, {aname} }};")
            water_info = f"&{iname}"

    if entry['fishing']:
        mons = entry['fishing'].get('mons', [])
        rate = entry['fishing'].get('encounter_rate', 30)
        if mons:
            aname = f"sHoenn_{label}_FishMons"
            all_arrays.append(f"static const struct WildPokemon {aname}[] = {{")
            for m in mons:
                all_arrays.append(f"    {{ {m['min_level']}, {m['max_level']}, {m['species']} }},")
            all_arrays.append("};")
            iname = f"sHoenn_{label}_FishInfo"
            all_arrays.append(f"static const struct WildPokemonInfo {iname} = {{ {rate}, {aname} }};")
            fish_info = f"&{iname}"

    if entry['rock']:
        mons = entry['rock'].get('mons', [])
        rate = entry['rock'].get('encounter_rate', 10)
        if mons:
            aname = f"sHoenn_{label}_RockMons"
            all_arrays.append(f"static const struct WildPokemon {aname}[] = {{")
            for m in mons:
                all_arrays.append(f"    {{ {m['min_level']}, {m['max_level']}, {m['species']} }},")
            all_arrays.append("};")
            iname = f"sHoenn_{label}_RockInfo"
            all_arrays.append(f"static const struct WildPokemonInfo {iname} = {{ {rate}, {aname} }};")
            rock_info = f"&{iname}"

    if land_info or water_info or fish_info or rock_info:
        all_headers_fr.append(f"    {{ .mapGroup = {g}, .mapNum = {n}, .landMonsInfo = {land_info or 'NULL'}, .waterMonsInfo = {water_info or 'NULL'}, .rockSmashMonsInfo = {rock_info or 'NULL'}, .fishingMonsInfo = {fish_info or 'NULL'} }},")

# Write output
outpath = os.path.join(os.path.dirname(__file__), 'encounter_data.h')
with open(outpath, 'w') as f:
    f.write("// Auto-generated Hoenn encounter data\n")
    f.write("// Add to src/data/wild_encounters.h before the MAP_UNDEFINED terminator in gWildMonHeaders_FR/LG\n\n")
    f.write("#ifdef PORTABLE\n")
    f.write("// Hoenn encounter arrays\n")
    for line in all_arrays:
        f.write(line + "\n")
    f.write("\n// Hoenn encounter headers (add to gWildMonHeaders_FR before terminator)\n")
    for line in all_headers_fr:
        f.write(line + "\n")
    f.write("#endif\n")

print(f"Generated {outpath}")
print(f"  {len(all_arrays)} data lines, {len(all_headers_fr)} header entries")
