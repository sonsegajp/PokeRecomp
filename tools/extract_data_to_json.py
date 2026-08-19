#!/usr/bin/env python3
"""
Extract game data from pokefirered C source into JSON for Godot port.
Parses species info, moves, items, type chart, evolutions, exp tables, and learnsets.
"""

import json
import re
import os
import math

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INCLUDE_DIR = os.path.join(BASE_DIR, "include")
CONSTANTS_DIR = os.path.join(INCLUDE_DIR, "constants")
SRC_DIR = os.path.join(BASE_DIR, "src")
DATA_DIR = os.path.join(SRC_DIR, "data")
POKEMON_DATA_DIR = os.path.join(DATA_DIR, "pokemon")
OUTPUT_DIR = os.path.join(BASE_DIR, "extracted_data")

os.makedirs(OUTPUT_DIR, exist_ok=True)


# ─── Constant Parsing ───────────────────────────────────────────────────────────

def parse_defines(filepath, prefix=None):
    """Parse #define NAME VALUE lines from a header file into a dict."""
    defines = {}
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            m = re.match(r'#define\s+(\w+)\s+(\d+)', line.strip())
            if m:
                name, value = m.group(1), int(m.group(2))
                if prefix is None or name.startswith(prefix):
                    defines[name] = value
    return defines


def build_reverse_map(defines, prefix):
    """Build value->name map, stripping the prefix for clean names."""
    return {v: k.replace(prefix, "", 1) for k, v in defines.items() if k.startswith(prefix)}


def load_all_constants():
    """Load all constant definitions we need."""
    consts = {}

    species_defs = parse_defines(os.path.join(CONSTANTS_DIR, "species.h"), "SPECIES_")
    consts["species"] = species_defs
    consts["species_names"] = build_reverse_map(species_defs, "SPECIES_")

    move_defs = parse_defines(os.path.join(CONSTANTS_DIR, "moves.h"), "MOVE_")
    consts["moves"] = move_defs
    consts["move_names"] = build_reverse_map(move_defs, "MOVE_")

    item_defs = parse_defines(os.path.join(CONSTANTS_DIR, "items.h"), "ITEM_")
    consts["items"] = item_defs
    consts["item_names"] = build_reverse_map(item_defs, "ITEM_")

    ability_defs = parse_defines(os.path.join(CONSTANTS_DIR, "abilities.h"), "ABILITY_")
    consts["abilities"] = ability_defs
    consts["ability_names"] = build_reverse_map(ability_defs, "ABILITY_")

    pokemon_defs = parse_defines(os.path.join(CONSTANTS_DIR, "pokemon.h"))
    consts["pokemon"] = pokemon_defs

    type_map = {}
    for k, v in pokemon_defs.items():
        if k.startswith("TYPE_") and k not in ("TYPE_NONE",):
            type_map[v] = k.replace("TYPE_", "", 1)
    consts["type_names"] = type_map

    growth_map = {}
    for k, v in pokemon_defs.items():
        if k.startswith("GROWTH_"):
            growth_map[v] = k.replace("GROWTH_", "", 1)
    consts["growth_names"] = growth_map

    egg_group_map = {}
    for k, v in pokemon_defs.items():
        if k.startswith("EGG_GROUP_"):
            egg_group_map[v] = k.replace("EGG_GROUP_", "", 1)
    consts["egg_group_names"] = egg_group_map

    body_color_map = {}
    for k, v in pokemon_defs.items():
        if k.startswith("BODY_COLOR_"):
            body_color_map[v] = k.replace("BODY_COLOR_", "", 1)
    consts["body_color_names"] = body_color_map

    evo_map = {}
    for k, v in pokemon_defs.items():
        if k.startswith("EVO_") and not k.startswith("EVO_MODE") and not k.startswith("EVOS_"):
            evo_map[v] = k.replace("EVO_", "", 1)
    consts["evo_names"] = evo_map

    return consts


# ─── Species Info ────────────────────────────────────────────────────────────────

def extract_species(consts):
    """Parse species_info.h into a list of Pokemon."""
    filepath = os.path.join(POKEMON_DATA_DIR, "species_info.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    all_constants = {}
    all_constants.update(consts["species"])
    all_constants.update(consts["items"])
    all_constants.update(consts["abilities"])
    all_constants.update(consts["pokemon"])

    species_list = []

    # Skip macros like OLD_UNOWN_SPECIES_INFO
    entries_text = text[text.find("gSpeciesInfo[]"):]

    # Split on species entries - match [SPECIES_XXX] = { ... },
    # Use a pattern that handles nested braces by matching to "\n    },"
    pattern = re.compile(
        r'\[(SPECIES_\w+)\]\s*=\s*\n\s*\{(.*?)\n\s*\}',
        re.DOTALL
    )

    for match in pattern.finditer(entries_text):
        species_name = match.group(1)
        body = match.group(2)

        if species_name == "SPECIES_NONE":
            continue

        species_id = consts["species"].get(species_name, -1)
        clean_name = species_name.replace("SPECIES_", "")

        entry = {"id": species_id, "name": clean_name}

        for field_match in re.finditer(r'\.(\w+)\s*=\s*(.+?)(?:,\s*$|\s*$)', body, re.MULTILINE):
            field = field_match.group(1)
            val = field_match.group(2).strip().rstrip(",")

            if field in ("baseHP", "baseAttack", "baseDefense", "baseSpeed",
                         "baseSpAttack", "baseSpDefense", "catchRate", "expYield",
                         "eggCycles", "friendship", "safariZoneFleeRate"):
                entry[field] = int(val)
            elif field.startswith("evYield_"):
                entry[field] = int(val)
            elif field == "types":
                types_match = re.findall(r'(TYPE_\w+)', val)
                entry["types"] = [consts["type_names"].get(
                    consts["pokemon"].get(t, -1), t
                ) for t in types_match]
            elif field == "abilities":
                ab_match = re.findall(r'(ABILITY_\w+)', val)
                entry["abilities"] = [consts["ability_names"].get(
                    consts["abilities"].get(a, -1), a
                ) for a in ab_match if a != "ABILITY_NONE"]
            elif field == "eggGroups":
                eg_match = re.findall(r'(EGG_GROUP_\w+)', val)
                entry["egg_groups"] = [consts["egg_group_names"].get(
                    consts["pokemon"].get(e, -1), e
                ) for e in eg_match]
            elif field == "growthRate":
                gv = consts["pokemon"].get(val.strip(), -1)
                entry["growth_rate"] = consts["growth_names"].get(gv, val.strip())
            elif field == "genderRatio":
                val = val.strip()
                if val == "MON_GENDERLESS":
                    entry["gender_ratio"] = "GENDERLESS"
                elif val == "MON_FEMALE":
                    entry["gender_ratio"] = 100.0
                elif val == "MON_MALE":
                    entry["gender_ratio"] = 0.0
                elif "PERCENT_FEMALE" in val:
                    pct = re.search(r'PERCENT_FEMALE\((\d+\.?\d*)\)', val)
                    entry["gender_ratio"] = float(pct.group(1)) if pct else val
                else:
                    entry["gender_ratio"] = val
            elif field == "itemCommon":
                if val.strip() != "ITEM_NONE":
                    entry["item_common"] = consts["item_names"].get(
                        consts["items"].get(val.strip(), -1), val.strip()
                    )
            elif field == "itemRare":
                if val.strip() != "ITEM_NONE":
                    entry["item_rare"] = consts["item_names"].get(
                        consts["items"].get(val.strip(), -1), val.strip()
                    )
            elif field == "bodyColor":
                cv = consts["pokemon"].get(val.strip(), -1)
                entry["body_color"] = consts["body_color_names"].get(cv, val.strip())
            elif field == "noFlip":
                entry["no_flip"] = val.strip() == "TRUE"

        species_list.append(entry)

    species_list.sort(key=lambda x: x["id"])
    return species_list


# ─── Moves ───────────────────────────────────────────────────────────────────────

def extract_moves(consts):
    """Parse battle_moves.h into a list of moves."""
    filepath = os.path.join(DATA_DIR, "battle_moves.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    moves_list = []
    pattern = re.compile(r'\[(MOVE_\w+)\]\s*=\s*\{(.*?)\}', re.DOTALL)

    flag_names = {
        1: "MAKES_CONTACT",
        2: "PROTECT_AFFECTED",
        4: "MAGIC_COAT_AFFECTED",
        8: "SNATCH_AFFECTED",
        16: "MIRROR_MOVE_AFFECTED",
        32: "KINGS_ROCK_AFFECTED",
    }

    for match in pattern.finditer(text):
        move_name = match.group(1)
        body = match.group(2)

        move_id = consts["moves"].get(move_name, -1)
        clean_name = move_name.replace("MOVE_", "")

        entry = {"id": move_id, "name": clean_name}

        for field_match in re.finditer(r'\.(\w+)\s*=\s*(.+?)(?:,\s*$|\s*$)', body, re.MULTILINE):
            field = field_match.group(1)
            val = field_match.group(2).strip().rstrip(",")

            if field in ("power", "accuracy", "pp", "secondaryEffectChance", "priority"):
                try:
                    entry[field] = int(val)
                except ValueError:
                    entry[field] = val
            elif field == "type":
                tv = consts["pokemon"].get(val.strip(), -1)
                entry["type"] = consts["type_names"].get(tv, val.strip())
            elif field == "effect":
                entry["effect"] = val.strip().replace("EFFECT_", "")
            elif field == "target":
                entry["target"] = val.strip().replace("MOVE_TARGET_", "")
            elif field == "flags":
                flags_val = val.strip()
                if flags_val == "0":
                    entry["flags"] = []
                else:
                    flag_tokens = re.findall(r'FLAG_(\w+)', flags_val)
                    entry["flags"] = flag_tokens

        moves_list.append(entry)

    moves_list.sort(key=lambda x: x["id"])
    return moves_list


# ─── Type Effectiveness ──────────────────────────────────────────────────────────

def extract_type_chart(consts):
    """Parse the type effectiveness table from battle_main.c."""
    filepath = os.path.join(SRC_DIR, "battle_main.c")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    start = text.find("gTypeEffectiveness[")
    end = text.find("};", start) + 2
    table_text = text[start:end]

    mul_map = {
        "TYPE_MUL_NO_EFFECT": 0.0,
        "TYPE_MUL_NOT_EFFECTIVE": 0.5,
        "TYPE_MUL_SUPER_EFFECTIVE": 2.0,
    }

    type_chart = []
    for line in table_text.split("\n"):
        line = line.strip().rstrip(",")
        tokens = re.findall(r'(TYPE_\w+)', line)
        if len(tokens) == 3:
            atk, dfc, mul = tokens
            if atk in ("TYPE_ENDTABLE", "TYPE_FORESIGHT"):
                continue
            atk_name = consts["type_names"].get(consts["pokemon"].get(atk, -1), atk)
            def_name = consts["type_names"].get(consts["pokemon"].get(dfc, -1), dfc)
            multiplier = mul_map.get(mul, 1.0)
            type_chart.append({
                "attacking": atk_name,
                "defending": def_name,
                "multiplier": multiplier
            })

    return type_chart


# ─── Evolution ───────────────────────────────────────────────────────────────────

def extract_evolutions(consts):
    """Parse evolution.h into evolution data."""
    filepath = os.path.join(POKEMON_DATA_DIR, "evolution.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    evolutions = {}
    for line in text.split("\n"):
        species_match = re.match(r'\s*\[(SPECIES_\w+)\]\s*=\s*\{(.+)\}', line)
        if not species_match:
            continue

        species_name = species_match.group(1).replace("SPECIES_", "")
        evo_body = species_match.group(2)

        evos = []
        for evo_match in re.finditer(r'\{(\w+),\s*(\w+|\d+),\s*(SPECIES_\w+)\}', evo_body):
            method_str = evo_match.group(1)
            param_str = evo_match.group(2)
            target_str = evo_match.group(3).replace("SPECIES_", "")

            method_val = consts["pokemon"].get(method_str, -1)
            method_name = consts["evo_names"].get(method_val, method_str)

            if param_str.isdigit():
                param = int(param_str)
            else:
                param = consts["items"].get(param_str, param_str)
                if isinstance(param, int) and param_str.startswith("ITEM_"):
                    param = param_str.replace("ITEM_", "")

            evos.append({
                "method": method_name,
                "param": param,
                "target": target_str
            })

        if evos:
            evolutions[species_name] = evos

    return evolutions


# ─── Experience Tables ───────────────────────────────────────────────────────────

def extract_experience_tables():
    """Compute experience tables from the formulas."""
    def exp_medium_fast(n): return n ** 3
    def exp_erratic(n):
        if n <= 50: return ((100 - n) * n**3) // 50
        elif n <= 68: return ((150 - n) * n**3) // 100
        elif n <= 98: return (((1911 - 10 * n) // 3) * n**3) // 500
        else: return ((160 - n) * n**3) // 100
    def exp_fluctuating(n):
        if n <= 15: return (((n + 1) // 3 + 24) * n**3) // 50
        elif n <= 36: return ((n + 14) * n**3) // 50
        else: return (((n // 2) + 32) * n**3) // 50
    def exp_medium_slow(n): return (6 * n**3) // 5 - 15 * n**2 + 100 * n - 140
    def exp_slow(n): return (5 * n**3) // 4
    def exp_fast(n): return (4 * n**3) // 5

    tables = {}
    growth_funcs = {
        "MEDIUM_FAST": exp_medium_fast,
        "ERRATIC": exp_erratic,
        "FLUCTUATING": exp_fluctuating,
        "MEDIUM_SLOW": exp_medium_slow,
        "SLOW": exp_slow,
        "FAST": exp_fast,
    }

    for name, func in growth_funcs.items():
        table = [0, 1]  # levels 0 and 1
        for level in range(2, 101):
            table.append(func(level))
        tables[name] = table

    return tables


# ─── Level-Up Learnsets ──────────────────────────────────────────────────────────

def extract_level_up_learnsets(consts):
    """Parse level_up_learnsets.h."""
    filepath = os.path.join(POKEMON_DATA_DIR, "level_up_learnsets.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    learnsets = {}
    pattern = re.compile(
        r'static\s+const\s+u16\s+s(\w+)LevelUpLearnset\[\]\s*=\s*\{(.*?)\};',
        re.DOTALL
    )

    for match in pattern.finditer(text):
        pokemon_name = match.group(1)
        body = match.group(2)

        moves = []
        for move_match in re.finditer(r'LEVEL_UP_MOVE\((\d+),\s*(MOVE_\w+)\)', body):
            level = int(move_match.group(1))
            move_name = move_match.group(2).replace("MOVE_", "")
            moves.append({"level": level, "move": move_name})

        if moves:
            learnsets[pokemon_name.upper()] = moves

    return learnsets


# ─── TM/HM Learnsets ────────────────────────────────────────────────────────────

def extract_tmhm_learnsets(consts):
    """Parse tmhm_learnsets.h."""
    filepath = os.path.join(POKEMON_DATA_DIR, "tmhm_learnsets.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    learnsets = {}
    pattern = re.compile(
        r'\[(SPECIES_\w+)\]\s*=\s*TMHM_LEARNSET\((.*?)\)\)',
        re.DOTALL
    )

    for match in pattern.finditer(text):
        species = match.group(1).replace("SPECIES_", "")
        body = match.group(2)

        tms = re.findall(r'TMHM\((TM\d+_\w+|HM\d+_\w+)\)', body)
        if tms:
            learnsets[species] = tms

    # Also handle the simple case: TMHM_LEARNSET(0)
    pattern_zero = re.compile(r'\[(SPECIES_\w+)\]\s*=\s*TMHM_LEARNSET\(0\)')
    # We skip those since they have no TMs

    return learnsets


# ─── Egg Moves ───────────────────────────────────────────────────────────────────

def extract_egg_moves(consts):
    """Parse egg_moves.h."""
    filepath = os.path.join(POKEMON_DATA_DIR, "egg_moves.h")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    egg_moves = {}
    pattern = re.compile(
        r'egg_moves\((\w+)(.*?)\)(?:\s*,|\s*EGG_MOVES_TERMINATOR)',
        re.DOTALL
    )

    for match in pattern.finditer(text):
        species = match.group(1)
        body = match.group(2)
        moves = re.findall(r'MOVE_(\w+)', body)
        if moves:
            egg_moves[species] = moves

    return egg_moves


# ─── Items (already JSON) ────────────────────────────────────────────────────────

def extract_items():
    """Load items.json and clean it up."""
    filepath = os.path.join(DATA_DIR, "items.json")
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        data = json.load(f)

    items = []
    for item in data.get("items", []):
        cleaned = {
            "id": item.get("itemId", "").replace("ITEM_", ""),
            "name": item.get("english", ""),
            "price": item.get("price", 0),
            "hold_effect": item.get("holdEffect", "").replace("HOLD_EFFECT_", ""),
            "hold_effect_param": item.get("holdEffectParam", 0),
            "description": item.get("description_english", "").replace("\\n", " "),
            "pocket": item.get("pocket", "").replace("POCKET_", ""),
            "battle_usage": item.get("battleUsage", 0),
        }
        items.append(cleaned)

    return items


# ─── Main ────────────────────────────────────────────────────────────────────────

def save_json(data, filename):
    filepath = os.path.join(OUTPUT_DIR, filename)
    with open(filepath, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    count = len(data) if isinstance(data, (list, dict)) else "?"
    print(f"  -> {filename}: {count} entries")


def main():
    print("Loading constants...")
    consts = load_all_constants()
    print(f"  Species: {len(consts['species'])}, Moves: {len(consts['moves'])}, "
          f"Items: {len(consts['items'])}, Abilities: {len(consts['abilities'])}")

    print("\nExtracting species data...")
    species = extract_species(consts)
    save_json(species, "species.json")

    print("Extracting moves data...")
    moves = extract_moves(consts)
    save_json(moves, "moves.json")

    print("Extracting items data...")
    items = extract_items()
    save_json(items, "items.json")

    print("Extracting type effectiveness chart...")
    type_chart = extract_type_chart(consts)
    save_json(type_chart, "type_chart.json")

    print("Extracting evolution data...")
    evolutions = extract_evolutions(consts)
    save_json(evolutions, "evolutions.json")

    print("Computing experience tables...")
    exp_tables = extract_experience_tables()
    save_json(exp_tables, "experience_tables.json")

    print("Extracting level-up learnsets...")
    level_up = extract_level_up_learnsets(consts)
    save_json(level_up, "learnsets_level_up.json")

    print("Extracting TM/HM learnsets...")
    tmhm = extract_tmhm_learnsets(consts)
    save_json(tmhm, "learnsets_tmhm.json")

    print("Extracting egg moves...")
    egg = extract_egg_moves(consts)
    save_json(egg, "learnsets_egg_moves.json")

    print(f"\nAll data extracted to: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
