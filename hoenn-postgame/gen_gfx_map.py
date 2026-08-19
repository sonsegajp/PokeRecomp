import re
fr = {}
with open(r'c:\Users\hyper\projects\pokefirered-native\pokefirered\include\constants\event_objects.h') as f:
    for line in f:
        m = re.match(r'#define (OBJ_EVENT_GFX_\w+)\s+(\d+)', line)
        if m: fr[m.group(1)] = int(m.group(2))
rb = {}
with open(r'c:\Users\hyper\projects\pokefirered-native\pokeruby\include\constants\event_objects.h') as f:
    for line in f:
        m = re.match(r'#define (OBJ_EVENT_GFX_\w+)\s+(\d+)', line)
        if m: rb[m.group(1)] = int(m.group(2))
print("# All matching GFX names between Ruby and FireRed:")
for name in sorted(rb):
    if name in fr:
        print(f"    '{name}': {fr[name]},")
