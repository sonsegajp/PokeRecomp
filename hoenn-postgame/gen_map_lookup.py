"""Generate correct indoor map lookup for hoenn_maps.c using Ruby directory names."""
import re, os

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'

# Get actual directory names from Ruby's maps folder
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
actual_dirs = set(os.listdir(maps_dir))

# Parse Ruby's map_groups.h
mg_path = os.path.join(POKERUBY, 'include', 'constants', 'map_groups.h')
entries = []

with open(mg_path) as f:
    for line in f:
        m = re.match(r'#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
        if m:
            map_id = m.group(1)
            map_num = int(m.group(2))
            ruby_group = int(m.group(3))

            # Convert MAP_OLDALE_TOWN_HOUSE1 to possible directory names
            raw = map_id.replace('MAP_', '')
            # Try to find matching directory
            dirname = None
            for d in actual_dirs:
                # Compare normalized
                if d.lower().replace('_', '') == raw.lower().replace('_', ''):
                    dirname = d
                    break
            if not dirname:
                # CamelCase conversion
                parts = raw.split('_')
                dirname = ''.join(p.capitalize() for p in parts)

            entries.append((ruby_group, map_num, map_id, dirname))

# Check which maps exist in hoenn_data.c
data_path = r'c:\Users\hyper\projects\pokefirered-native\pokefirered\src\platform\hoenn_data.c'
with open(data_path) as f:
    data_content = f.read()

existing = set()
for m in re.finditer(r'const struct MapHeader gHoennMapHeader_(\w+)\s*=', data_content):
    existing.add(m.group(1))

print(f"Found {len(existing)} map headers in hoenn_data.c")

# Generate lookup
indoor_entries = [(rg, mn, mid, dn) for rg, mn, mid, dn in entries if rg > 0]

matched = 0
unmatched_list = []
lines = []

for ruby_group, map_num, map_id, dirname in indoor_entries:
    hoenn_group = ruby_group + 43

    if dirname in existing:
        lines.append(f"    if (group == {hoenn_group} && num == {map_num}) return &gHoennMapHeader_{dirname};")
        matched += 1
    else:
        # Try without common variations
        found = False
        for ex in existing:
            if ex.lower().replace('_', '') == dirname.lower().replace('_', ''):
                lines.append(f"    if (group == {hoenn_group} && num == {map_num}) return &gHoennMapHeader_{ex};")
                matched += 1
                found = True
                break
        if not found:
            unmatched_list.append(f"    // MISSING: g={hoenn_group} n={map_num} {map_id} ({dirname})")
            unmatched_list.append(f"    // Not in existing: searched for '{dirname}'")

print(f"Matched: {matched}, Missing: {len(unmatched_list)//2}")

outpath = os.path.join(os.path.dirname(__file__), 'map_lookup.txt')
with open(outpath, 'w') as f:
    f.write("// Indoor map lookup — replace the if-chain in Hoenn_GetMapHeader\n")
    for line in lines:
        f.write(line + "\n")
    f.write("\n// === MISSING MAPS ===\n")
    for line in unmatched_list:
        f.write(line + "\n")
print(f"Generated {outpath}")
