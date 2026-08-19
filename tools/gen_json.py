#!/usr/bin/env python3
"""Generate pokemon.json and moves.json from decomp source."""
import re, json, os, sys

root = sys.argv[1] if len(sys.argv) > 1 else '.'
outdir = sys.argv[2] if len(sys.argv) > 2 else 'extracted_assets/frlg/data'
os.makedirs(outdir, exist_ok=True)

def rf(p):
    return open(os.path.join(root, p), 'r', errors='replace').read()

def gv(b, k):
    m = re.search(r'\.' + k + r'\s*=\s*(\d+)', b)
    return int(m.group(1)) if m else 0

# Pokemon
try:
    ids = {m.group(1): int(m.group(2)) for m in re.finditer(r'#define\s+SPECIES_(\w+)\s+(\d+)', rf('include/constants/species.h'))}
    nm = {k: v for k, v in re.findall(r'\[SPECIES_(\w+)\]\s*=\s*_\("([^"]*)"\)', rf('src/data/text/species_names.h'))}
    blocks = re.findall(r'\[SPECIES_(\w+)\]\s*=\s*\{([^}]+)\}', rf('src/data/pokemon/species_info.h'), re.DOTALL)
    r = {}
    for n, b in blocks:
        sid = ids.get(n, -1)
        if sid < 0: continue
        r[sid] = {'id': sid, 'name': nm.get(n, n), 'hp': gv(b, 'baseHP'), 'attack': gv(b, 'baseAttack'),
                  'defense': gv(b, 'baseDefense'), 'speed': gv(b, 'baseSpeed'),
                  'sp_attack': gv(b, 'baseSpAttack'), 'sp_defense': gv(b, 'baseSpDefense')}
    mx = max(ids.values()) + 1
    json.dump([r.get(i, {'id': i, 'name': '?'}) for i in range(mx)], open(os.path.join(outdir, 'pokemon.json'), 'w'), indent=2)
    print(f'  pokemon.json: {len(r)} entries')
except Exception as e:
    print(f'  pokemon.json failed: {e}')

# Moves
try:
    ids = {m.group(1): int(m.group(2)) for m in re.finditer(r'#define\s+MOVE_(\w+)\s+(\d+)', rf('include/constants/moves.h'))}
    nm = {k: v for k, v in re.findall(r'\[MOVE_(\w+)\]\s*=\s*_\("([^"]*)"\)', rf('src/data/text/move_names.h'))}
    blocks = re.findall(r'\[MOVE_(\w+)\]\s*=\s*\{([^}]+)\}', rf('src/data/battle_moves.h'), re.DOTALL)
    r = {}
    for n, b in blocks:
        mid = ids.get(n, -1)
        if mid < 0: continue
        r[mid] = {'id': mid, 'name': nm.get(n, n), 'power': gv(b, 'power'), 'pp': gv(b, 'pp'), 'accuracy': gv(b, 'accuracy')}
    mx = max(ids.values()) + 1
    json.dump([r.get(i, {'id': i, 'name': '?'}) for i in range(mx)], open(os.path.join(outdir, 'moves.json'), 'w'), indent=2)
    print(f'  moves.json: {len(r)} entries')
except Exception as e:
    print(f'  moves.json failed: {e}')
