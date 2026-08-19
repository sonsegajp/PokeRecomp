import re

with open('src/m4a.c', 'r') as f:
    lines = f.readlines()

# Find function definitions and add M4A_CHECK after opening brace
result = []
i = 0
skip_funcs = ['m4aSoundInit', 'MPlayOpen']  # these already have special handling

# Functions needing M4A_CHECK_VAL instead of M4A_CHECK
val_funcs = {
    'IsPokemonCryPlaying': 'FALSE',
    'SetPokemonCryTone': 'NULL',
    'MidiKeyToCgbFreq': '0',
    'MidiKeyToFreq': '0',
}

while i < len(lines):
    line = lines[i]
    result.append(line)

    # Check if this looks like a function definition in m4a.c
    # Pattern: return_type FunctionName(args...) at start of line, followed by {
    if i + 1 < len(lines) and lines[i+1].strip() == '{':
        # Extract function name
        m = re.match(r'^(?:void|u8|u16|u32|s32|bool32|struct\s+\w+\s*\*)\s+(\w+)\s*\(', line)
        if m:
            fname = m.group(1)
            if fname not in skip_funcs and not line.strip().startswith('static'):
                # Add the opening brace
                result.append(lines[i+1])  # {
                i += 2

                # Check if M4A_CHECK is already there
                if i < len(lines) and 'M4A_CHECK' in lines[i]:
                    continue

                # Add appropriate check
                if fname in val_funcs:
                    result.append(f'    M4A_CHECK_VAL({val_funcs[fname]});\n')
                else:
                    result.append('    M4A_CHECK;\n')
                continue
    i += 1

with open('src/m4a.c', 'w') as f:
    f.writelines(result)

print('Done')
