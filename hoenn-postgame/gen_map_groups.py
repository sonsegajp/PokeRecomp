"""Generate hoenn_map_groups.h from Ruby's map_groups.h"""
import re

INPUT = r"c:\Users\hyper\projects\pokefirered-native\pokefirered\hoenn-postgame\constants\map_groups.h"
OUTPUT = r"c:\Users\hyper\projects\pokefirered-native\pokefirered\include\constants\hoenn_map_groups.h"

HOENN_GROUP_BASE = 43

with open(INPUT, "r") as f:
    lines = f.readlines()

out = []
out.append("#ifndef GUARD_CONSTANTS_HOENN_MAP_GROUPS_H")
out.append("#define GUARD_CONSTANTS_HOENN_MAP_GROUPS_H")
out.append("")
out.append("// Auto-generated from pokeruby map_groups.h")
out.append("// Hoenn map groups start at group 43 (after FireRed's 43 groups)")
out.append("")

map_count = 0
group_comments = []
current_comment = None

for line in lines:
    line = line.rstrip()

    # Match group comments like "// gMapGroup_TownsAndRoutes"
    m = re.match(r'^// (gMapGroup_\w+)', line)
    if m:
        current_comment = m.group(1)
        out.append(f"// {current_comment} (Hoenn)")
        continue

    # Match map defines like "#define MAP_PETALBURG_CITY   (0 | (0 << 8))"
    m = re.match(r'^#define (MAP_\w+)\s+\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
    if m:
        name = m.group(1)
        map_num = int(m.group(2))
        group_num = int(m.group(3))
        new_group = group_num + HOENN_GROUP_BASE
        hoenn_name = f"MAP_HOENN_{name[4:]}"  # MAP_X -> MAP_HOENN_X
        out.append(f"#define {hoenn_name:<60s} ({map_num} | ({new_group} << 8))")
        map_count += 1
        continue

    # Match MAP_GROUPS_COUNT
    m = re.match(r'^#define MAP_GROUPS_COUNT\s+(\d+)', line)
    if m:
        ruby_groups = int(m.group(1))
        out.append("")
        out.append(f"#define HOENN_MAP_GROUPS_COUNT {ruby_groups}")
        out.append(f"#define MAP_GROUPS_COUNT_WITH_HOENN ({HOENN_GROUP_BASE} + {ruby_groups})")
        continue

    # Empty lines
    if line.strip() == "":
        out.append("")

out.append("")
out.append(f"// Total Hoenn maps: {map_count}")
out.append("")
out.append("#endif // GUARD_CONSTANTS_HOENN_MAP_GROUPS_H")

with open(OUTPUT, "w") as f:
    f.write("\n".join(out) + "\n")

print(f"Generated {OUTPUT}")
print(f"  {map_count} maps remapped across groups {HOENN_GROUP_BASE}+")
