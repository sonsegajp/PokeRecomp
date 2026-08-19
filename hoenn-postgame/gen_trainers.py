"""Generate scaled Hoenn trainer data for FireRed's trainer table."""
import re, os

POKERUBY = r'c:\Users\hyper\projects\pokefirered-native\pokeruby'
HOENN_TRAINER_BASE = 768

# Read Ruby trainer data
trainers_path = os.path.join(POKERUBY, 'src', 'data', 'trainers_en.h')
parties_path = os.path.join(POKERUBY, 'src', 'data', 'trainer_parties.h')

# Scale level: original range ~5-60, target 50-70
def scale_level(lv):
    # Linear scale: 5->50, 60->70
    scaled = 50 + (lv - 5) * (20.0 / 55.0)
    return max(50, min(70, int(scaled)))

# Read parties and scale levels
with open(parties_path, 'r') as f:
    content = f.read()

# Find all party arrays and scale levels
scaled_parties = content
# Replace .level = N with scaled version
def scale_match(m):
    lv = int(m.group(1))
    return f'.level = {scale_level(lv)}'

scaled_parties = re.sub(r'\.level = (\d+)', scale_match, scaled_parties)

# Rename all symbols to avoid conflicts with existing FireRed trainers
# Prefix everything with 'Hoenn_'
scaled_parties = re.sub(r'(gTrainerParty_)', r'gHoennTrainerParty_', scaled_parties)

# Write scaled parties
outpath_parties = os.path.join(os.path.dirname(__file__), 'hoenn_trainer_parties.h')
with open(outpath_parties, 'w') as f:
    f.write("// Auto-generated scaled Hoenn trainer parties (levels 50-70)\n")
    f.write("#ifdef PORTABLE\n")
    f.write(scaled_parties)
    f.write("\n#endif\n")

# Count entries
party_count = len(re.findall(r'gHoennTrainerParty_\w+\[\]', scaled_parties))
print(f"Generated {outpath_parties} with {party_count} trainer parties")

# Read trainer table and rename references
with open(trainers_path, 'r') as f:
    trainer_content = f.read()

# Rename party references
scaled_trainers = trainer_content.replace('sParty_', 'sHoennParty_')
# Also fix the .party references to use our renamed parties
scaled_trainers = re.sub(r'(gTrainerParty_)', r'gHoennTrainerParty_', scaled_trainers)

# Rename the trainer array
scaled_trainers = scaled_trainers.replace('gTrainers[]', 'gHoennTrainers[]')

outpath_trainers = os.path.join(os.path.dirname(__file__), 'hoenn_trainers.h')
with open(outpath_trainers, 'w') as f:
    f.write("// Auto-generated scaled Hoenn trainer data\n")
    f.write("#ifdef PORTABLE\n")
    f.write(scaled_trainers)
    f.write("\n#endif\n")

trainer_count = len(re.findall(r'\[TRAINER_', scaled_trainers))
print(f"Generated {outpath_trainers} with ~{trainer_count} trainer entries")
