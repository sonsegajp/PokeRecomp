"""
Port Ruby's actual scripts into FireRed.
Remaps FLAGS, VARs, and TRAINER IDs to Hoenn space.
Stubs out Ruby-specific specials that don't exist in FireRed.
"""
import os, re

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
POKEFR = r'c:\Users\hyper\projects\pokefirered-native\pokefirered'
HOENN_FLAGS_BASE = 0x1000
HOENN_VARS_BASE = 0x5000
HOENN_TRAINER_BASE = 768

# ===== 1. Read all Ruby constants =====

def read_defines(path, prefix):
    defs = {}
    if not os.path.isfile(path): return defs
    for line in open(path):
        m = re.match(rf'#define\s+({prefix}\w+)\s+(.+)', line)
        if m:
            name = m.group(1)
            val_str = m.group(2).split('//')[0].strip()
            try:
                val = eval(val_str)
                defs[name] = val
            except:
                pass
    return defs

rb_flags = read_defines(os.path.join(POKERUBY, 'include', 'constants', 'flags.h'), 'FLAG_')
rb_vars = read_defines(os.path.join(POKERUBY, 'include', 'constants', 'vars.h'), 'VAR_')
rb_trainers = read_defines(os.path.join(POKERUBY, 'include', 'constants', 'opponents.h'), 'TRAINER_')

# Read ALL Ruby constants from ALL header files
rb_all = {}
for root, dirs, files in os.walk(os.path.join(POKERUBY, 'include')):
    for fn in files:
        if not fn.endswith('.h'): continue
        try:
            for line in open(os.path.join(root, fn)):
                m = re.match(r'#define\s+(\w+)\s+(.+)', line)
                if m:
                    name = m.group(1)
                    val_str = m.group(2).split('//')[0].split('/*')[0].strip()
                    if val_str and not val_str.startswith('(') and not val_str.startswith('"'):
                        try:
                            val = eval(val_str)
                            if isinstance(val, int):
                                rb_all[name] = val
                        except: pass
        except: pass
print(f"Ruby total constants: {len(rb_all)}")

print(f"Ruby: {len(rb_flags)} flags, {len(rb_vars)} vars, {len(rb_trainers)} trainers")

# ===== 2. Read ALL FireRed #defines (not just FLAG/VAR/TRAINER) =====
fr_all_defs = set()
for search_dir in [os.path.join(POKEFR, 'include'), os.path.join(POKEFR, 'gflib')]:
    for root, dirs, files in os.walk(search_dir):
        for fn in files:
            if not fn.endswith('.h'): continue
            try:
                for line in open(os.path.join(root, fn)):
                    m = re.match(r'#define\s+(\w+)\b', line)
                    if m: fr_all_defs.add(m.group(1))
            except: pass
print(f"FireRed defines {len(fr_all_defs)} total constants to skip")

# ===== 3. Generate remapped constants assembly file =====

constants_path = os.path.join(POKEFR, 'hoenn-postgame', 'hoenn_script_constants.inc')
with open(constants_path, 'w') as f:
    f.write("@ Auto-generated Ruby constants remapped to Hoenn space\n\n")

    # Flags: remap to HOENN_FLAGS_BASE + value
    f.write("@ Ruby FLAGS -> Hoenn flag space\n")
    for name, val in sorted(rb_flags.items()):
        if name in fr_all_defs:
            continue  # Skip if already defined in FireRed
        remapped = HOENN_FLAGS_BASE + val
        f.write(f".equ {name}, {remapped}\n")

    f.write("\n@ Ruby VARs -> Hoenn var space\n")
    for name, val in sorted(rb_vars.items()):
        if name in fr_all_defs:
            continue
        remapped = HOENN_VARS_BASE + (val - 0x4000) if val >= 0x4000 else HOENN_VARS_BASE + val
        f.write(f".equ {name}, {remapped}\n")

    f.write("\n@ Ruby TRAINER IDs -> Hoenn trainer space\n")
    for name, val in sorted(rb_trainers.items()):
        if name in fr_all_defs:
            continue
        remapped = HOENN_TRAINER_BASE + val
        f.write(f".equ {name}, {remapped}\n")

    # ALL remaining Ruby constants (MAP_*, HEAL_*, DECOR_*, STD_*, etc.)
    f.write("\n@ All Ruby constants not already in FireRed\n")
    # Also read ALL FR defines from assembly includes
    for root, dirs, files in os.walk(os.path.join(POKEFR, 'asm')):
        for fn in files:
            if not fn.endswith('.inc'): continue
            try:
                for line in open(os.path.join(root, fn)):
                    m = re.match(r'\.equ\s+(\w+)', line)
                    if m: fr_all_defs.add(m.group(1))
            except: pass
    # Also check data includes
    for root, dirs, files in os.walk(os.path.join(POKEFR, 'data')):
        for fn in files:
            if not fn.endswith('.inc'): continue
            try:
                for line in open(os.path.join(root, fn)):
                    m = re.match(r'\.equ\s+(\w+)', line)
                    if m: fr_all_defs.add(m.group(1))
            except: pass

    # Movement command aliases (Ruby has fastest variants, FR only has fast)
    f.write("\n@ Movement command aliases\n")
    f.write(".macro walk_fastest_down\n\twalk_fast_down\n.endm\n")
    f.write(".macro walk_fastest_up\n\twalk_fast_up\n.endm\n")
    f.write(".macro walk_fastest_left\n\twalk_fast_left\n.endm\n")
    f.write(".macro walk_fastest_right\n\twalk_fast_right\n.endm\n")
    f.write(".macro walk_in_place_fastest_down\n\twalk_in_place_fast_down\n.endm\n")
    f.write(".macro walk_in_place_fastest_up\n\twalk_in_place_fast_up\n.endm\n")
    f.write(".macro walk_in_place_fastest_left\n\twalk_in_place_fast_left\n.endm\n")
    f.write(".macro walk_in_place_fastest_right\n\twalk_in_place_fast_right\n.endm\n")

    written = 0
    for name, val in sorted(rb_all.items()):
        if name in fr_all_defs: continue
        if name.startswith('_') or name.startswith('GUARD_'): continue
        f.write(f".equ {name}, {val}\n")
        written += 1
    print(f"Wrote {written} Ruby constants to assembly")

print(f"Generated {constants_path}")

# ===== 4. Identify Ruby specials that don't exist in FireRed =====

# Read all specials from Ruby's scripts
maps_dir = os.path.join(POKERUBY, 'data', 'maps')
ruby_specials = set()
for dirname in os.listdir(maps_dir):
    sp = os.path.join(maps_dir, dirname, 'scripts.inc')
    if not os.path.isfile(sp): continue
    content = open(sp, 'r', errors='replace').read()
    ruby_specials.update(re.findall(r'\bspecial\s+(\w+)', content))

# Check which specials exist in FireRed
fr_specials = set()
specials_path = os.path.join(POKEFR, 'src', 'data', 'specials.inc')
if os.path.isfile(specials_path):
    content = open(specials_path, 'r', errors='replace').read()
    fr_specials = set(re.findall(r'def_special\s+(\w+)', content))

missing_specials = ruby_specials - fr_specials
print(f"Ruby specials: {len(ruby_specials)}, FR specials: {len(fr_specials)}, Missing: {len(missing_specials)}")

# ===== 5. Generate combined Ruby scripts with translations =====

# Ruby script commands that reference functions not in FireRed
STUB_COMMANDS = {'callstd', 'setdiveid', 'setescapewarp'}

output_path = os.path.join(POKEFR, 'hoenn-postgame', 'hoenn_ruby_scripts.inc')
text_output_path = os.path.join(POKEFR, 'hoenn-postgame', 'hoenn_ruby_texts.inc')

script_count = 0
text_count = 0

# Pre-seed seen_labels with ALL labels from FireRed's existing scripts
seen_labels = set()
for root, dirs, files in os.walk(os.path.join(POKEFR, 'data')):
    for fn in files:
        if not fn.endswith('.inc'): continue
        try:
            content = open(os.path.join(root, fn), 'r', errors='replace').read()
            for lm in re.finditer(r'^(\w+)::?(?:\s|$)', content, re.MULTILINE):
                seen_labels.add(lm.group(1))
        except: pass
print(f"Pre-seeded {len(seen_labels)} labels from FireRed scripts")

with open(output_path, 'w') as sf, open(text_output_path, 'w') as tf:
    sf.write("@ Ruby map scripts ported to FireRed\n")
    sf.write("@ Constants remapped to Hoenn space\n\n")
    tf.write("@ Ruby map text ported to FireRed\n\n")

    for dirname in sorted(os.listdir(maps_dir)):
        sp = os.path.join(maps_dir, dirname, 'scripts.inc')
        tp = os.path.join(maps_dir, dirname, 'text.inc')

        if os.path.isfile(sp):
            content = open(sp, 'r', errors='replace').read()

            # Comment out lines with missing specials
            for spec in missing_specials:
                content = re.sub(rf'(\s+special\s+{spec}\b)', r'@STUB\1', content)

            # Comment out callstd (Ruby-specific)
            content = re.sub(r'(\s+callstd\s+)', r'@STUB\1', content)

            # Comment out Ruby-specific script commands not in FireRed
            ruby_only_cmds = [
                'givedecoration', 'setobjectpriority', 'resetobjectpriority',
                'moveobjectoffscreen', 'setflashradius', 'showcontestwinner',
                'getpricereduction', 'init_affine_anim',
            ]
            for cmd in ruby_only_cmds:
                content = re.sub(rf'^([\t ]+{cmd}\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)

            # Fix commands with different argument counts in Ruby vs FireRed
            content = re.sub(r'^([\t ]+updatemoneybox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+removemoneybox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+showmoneybox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+braillemessage\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+hidemoneybox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+hidecoinsbox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+showcoinsbox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+updatecoinsbox\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+setdivewarp\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+setescapewarp\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            content = re.sub(r'^([\t ]+setdynamicwarp\b.*)$', r'@ STUB \1', content, flags=re.MULTILINE)
            # Stub ANY line that references undefined MAP_* constants
            content = re.sub(r'^([\t ]+\w+\b.*\bMAP_\w+.*)$', r'@ STUB \1', content, flags=re.MULTILINE)

            # Fix encoding issues
            content = content.replace('é', 'e')
            content = ''.join(c if ord(c) < 128 else 'e' for c in content)

            # Deduplicate: track all labels, comment out dupes AND their body
            lines = content.split('\n')
            deduped = []
            skip_until_next_label = False
            for line in lines:
                lm = re.match(r'^(\w+)::?(\s|$)', line)
                if lm:
                    label = lm.group(1)
                    if label in seen_labels:
                        skip_until_next_label = True
                        deduped.append(f'@ DUP {line}')
                        continue
                    seen_labels.add(label)
                    skip_until_next_label = False
                if skip_until_next_label:
                    deduped.append(f'@ DUP {line}')
                else:
                    deduped.append(line)
            content = '\n'.join(deduped)

            sf.write(f"\n@ ===== {dirname} =====\n")
            sf.write(content)
            sf.write("\n")
            script_count += 1

        if os.path.isfile(tp):
            content = open(tp, 'r', errors='replace').read()
            content = content.replace('é', 'e')
            content = ''.join(c if ord(c) < 128 else 'e' for c in content)

            tf.write(f"\n@ ===== {dirname} =====\n")
            tf.write(content)
            tf.write("\n")
            text_count += 1

print(f"Generated {output_path} ({script_count} maps)")
print(f"Generated {text_output_path} ({text_count} maps)")
