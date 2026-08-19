#!/usr/bin/env python3
"""Pokemon FireRed Save Editor for PokeRecomp"""
import struct, os, sys, json, tkinter as tk
from tkinter import ttk, filedialog, messagebox

# Save format constants
SDS = 3968
SECTOR_SIZE = 0x1000
NUM_SECTORS = 14
FLASH_SIZE = 0x20000
SECTOR_SIG = 0x08012025

# Our game's sector data sizes
SIZES = [3872, 3968, 3968, 3968, 3688, 3968, 3968, 3968, 3968, 3968, 3968, 3968, 3968, 2000]

# Offsets
SB1_PARTY_COUNT = 52
SB1_PARTY = 56
SB1_MONEY = 656
SB1_FLAGS = 3872

# Pokemon substructure ordering
SUBORDER = [
    [0,1,2,3],[0,1,3,2],[0,2,1,3],[0,3,1,2],[0,2,3,1],[0,3,2,1],
    [1,0,2,3],[1,0,3,2],[2,0,1,3],[3,0,1,2],[2,0,3,1],[3,0,2,1],
    [1,2,0,3],[1,3,0,2],[2,1,0,3],[3,1,0,2],[2,3,0,1],[3,2,0,1],
    [1,2,3,0],[1,3,2,0],[2,1,3,0],[3,1,2,0],[2,3,1,0],[3,2,1,0]]

# GBA text decode
def decode_gba_str(data, max_len=10):
    chars = []
    for b in data[:max_len]:
        if b == 0xFF: break
        if 0xBB <= b <= 0xD4: chars.append(chr(ord('A') + b - 0xBB))
        elif 0xD5 <= b <= 0xEE: chars.append(chr(ord('a') + b - 0xD5))
        elif 0xA1 <= b <= 0xAA: chars.append(chr(ord('0') + b - 0xA1))
        elif b == 0x00: chars.append(' ')
        elif b == 0xAD: chars.append('.')
        elif b == 0xAB: chars.append('!')
        elif b == 0xB4: chars.append("'")
        else: chars.append('?')
    return ''.join(chars).strip()

SPECIES_NAMES = [
    "???","Bulbasaur","Ivysaur","Venusaur","Charmander","Charmeleon","Charizard",
    "Squirtle","Wartortle","Blastoise","Caterpie","Metapod","Butterfree","Weedle",
    "Kakuna","Beedrill","Pidgey","Pidgeotto","Pidgeot","Rattata","Raticate",
    "Spearow","Fearow","Ekans","Arbok","Pikachu","Raichu","Sandshrew","Sandslash",
    "Nidoran F","Nidorina","Nidoqueen","Nidoran M","Nidorino","Nidoking",
    "Clefairy","Clefable","Vulpix","Ninetales","Jigglypuff","Wigglytuff",
    "Zubat","Golbat","Oddish","Gloom","Vileplume","Paras","Parasect",
    "Venonat","Venomoth","Diglett","Dugtrio","Meowth","Persian","Psyduck",
    "Golduck","Mankey","Primeape","Growlithe","Arcanine","Poliwag","Poliwhirl",
    "Poliwrath","Abra","Kadabra","Alakazam","Machop","Machoke","Machamp",
    "Bellsprout","Weepinbell","Victreebel","Tentacool","Tentacruel","Geodude",
    "Graveler","Golem","Ponyta","Rapidash","Slowpoke","Slowbro","Magnemite",
    "Magneton","Farfetch'd","Doduo","Dodrio","Seel","Dewgong","Grimer","Muk",
    "Shellder","Cloyster","Gastly","Haunter","Gengar","Onix","Drowzee","Hypno",
    "Krabby","Kingler","Voltorb","Electrode","Exeggcute","Exeggutor","Cubone",
    "Marowak","Hitmonlee","Hitmonchan","Lickitung","Koffing","Weezing","Rhyhorn",
    "Rhydon","Chansey","Tangela","Kangaskhan","Horsea","Seadra","Goldeen",
    "Seaking","Staryu","Starmie","Mr. Mime","Scyther","Jynx","Electabuzz",
    "Magmar","Pinsir","Tauros","Magikarp","Gyarados","Lapras","Ditto","Eevee",
    "Vaporeon","Jolteon","Flareon","Porygon","Omanyte","Omastar","Kabuto",
    "Kabutops","Aerodactyl","Snorlax","Articuno","Zapdos","Moltres","Dratini",
    "Dragonair","Dragonite","Mewtwo","Mew",
]
# Pad to 412
while len(SPECIES_NAMES) < 412: SPECIES_NAMES.append(f"Pokemon {len(SPECIES_NAMES)}")

def decrypt_pokemon(raw):
    """Decrypt a Pokemon's substructures and return species/level/moves."""
    pid = struct.unpack_from('<I', raw, 0)[0]
    otid = struct.unpack_from('<I', raw, 4)[0]
    nickname = decode_gba_str(raw[8:18])

    # Decrypt substructs
    key = pid ^ otid
    sub = bytearray(raw[32:80])
    for i in range(0, 48, 4):
        v = struct.unpack_from('<I', sub, i)[0]
        struct.pack_into('<I', sub, i, v ^ key)

    # Find growth substruct
    order = SUBORDER[pid % 24]
    growth_slot = order.index(0)
    attacks_slot = order.index(1)

    species = struct.unpack_from('<H', sub, growth_slot * 12)[0]

    moves = []
    for i in range(4):
        m = struct.unpack_from('<H', sub, attacks_slot * 12 + i * 2)[0]
        moves.append(m)

    level = raw[84] if len(raw) > 84 else 0
    hp = struct.unpack_from('<H', raw, 86)[0] if len(raw) > 87 else 0
    maxhp = struct.unpack_from('<H', raw, 88)[0] if len(raw) > 89 else 0

    return {
        'species': species,
        'name': nickname,
        'level': level,
        'hp': hp,
        'maxhp': maxhp,
        'moves': moves,
        'pid': pid,
        'species_name': SPECIES_NAMES[species] if species < len(SPECIES_NAMES) else f"#{species}"
    }

def checksum(data, size):
    cs = 0
    for i in range(0, size & ~3, 4):
        cs = (cs + struct.unpack_from('<I', data, i)[0]) & 0xFFFFFFFF
    return ((cs >> 16) + cs) & 0xFFFF

class SaveEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("PokeRecomp Save Editor")
        self.root.geometry("900x700")
        self.save_data = None
        self.sb2 = None; self.sb1 = None; self.ps = None
        self.active_slot = 0
        self.save_path = None

        menu = tk.Menu(root)
        fm = tk.Menu(menu, tearoff=0)
        fm.add_command(label="Open .sav...", command=self.open_save)
        fm.add_command(label="Save", command=self.save)
        fm.add_command(label="Save As...", command=self.save_as)
        menu.add_cascade(label="File", menu=fm)
        root.config(menu=menu)

        self.nb = ttk.Notebook(root)
        self.nb.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.info_var = tk.StringVar(value="Open a .sav file")
        ttk.Label(root, textvariable=self.info_var, font=("Consolas", 9)).pack(anchor='w', padx=5)

    def open_save(self):
        path = filedialog.askopenfilename(filetypes=[("Save files", "*.sav"), ("All", "*.*")])
        if not path: return
        self.save_path = path
        self.save_data = bytearray(open(path, 'rb').read())

        if len(self.save_data) < FLASH_SIZE:
            self.save_data += bytearray([0xFF] * (FLASH_SIZE - len(self.save_data)))

        # Find active slot
        cnt0 = self._slot_counter(0)
        cnt1 = self._slot_counter(14)
        self.active_slot = 14 if cnt1 >= cnt0 else 0

        # Extract data
        self.sb2 = bytearray(3872)
        self.sb1 = bytearray(15592)
        self.ps = bytearray(33744)

        for s in range(14):
            off = (self.active_slot + s) * SECTOR_SIZE
            sig = struct.unpack_from('<I', self.save_data, off + 0xFF8)[0]
            if sig != SECTOR_SIG: continue
            sid = struct.unpack_from('<H', self.save_data, off + 0xFF4)[0]
            sz = SIZES[sid] if sid < len(SIZES) else 0
            if sid == 0: self.sb2[:sz] = self.save_data[off:off+sz]
            elif 1 <= sid <= 4:
                d = (sid-1)*SDS; self.sb1[d:d+sz] = self.save_data[off:off+sz]
            elif 5 <= sid <= 13:
                d = (sid-5)*SDS; self.ps[d:d+sz] = self.save_data[off:off+sz]

        self.info_var.set(f"Loaded: {os.path.basename(path)} (slot {'B' if self.active_slot else 'A'}, counter {max(cnt0,cnt1)})")
        self._build_tabs()

    def _slot_counter(self, base):
        for s in range(14):
            off = (base + s) * SECTOR_SIZE
            if off + SECTOR_SIZE > len(self.save_data): return 0
            sig = struct.unpack_from('<I', self.save_data, off + 0xFF8)[0]
            if sig == SECTOR_SIG:
                return struct.unpack_from('<I', self.save_data, off + 0xFFC)[0]
        return 0

    def _build_tabs(self):
        for tab in self.nb.tabs(): self.nb.forget(tab)
        self._party_tab()
        self._pc_tab()
        self._trainer_tab()
        self._flags_tab()

    def _party_tab(self):
        f = ttk.Frame(self.nb); self.nb.add(f, text="Party")
        pcnt = struct.unpack_from('<I', self.sb1, SB1_PARTY_COUNT)[0]
        ttk.Label(f, text=f"Party ({pcnt} Pokemon):", font=("Consolas", 10, "bold")).pack(anchor='w', padx=10, pady=5)

        for i in range(min(pcnt, 6)):
            raw = self.sb1[SB1_PARTY + i*100 : SB1_PARTY + (i+1)*100]
            info = decrypt_pokemon(raw)
            frame = ttk.LabelFrame(f, text=f"Slot {i+1}")
            frame.pack(fill=tk.X, padx=10, pady=2)
            ttk.Label(frame, text=f"{info['species_name']} Lv{info['level']}  HP:{info['hp']}/{info['maxhp']}  Moves:{info['moves']}",
                      font=("Consolas", 9)).pack(anchor='w', padx=5)

    def _pc_tab(self):
        f = ttk.Frame(self.nb); self.nb.add(f, text="PC Boxes")
        tree = ttk.Treeview(f, columns=('box','slot','species','level'), show='headings', height=20)
        tree.heading('box', text='Box'); tree.heading('slot', text='Slot')
        tree.heading('species', text='Species'); tree.heading('level', text='Level')
        tree.column('box', width=50); tree.column('slot', width=50)
        tree.column('species', width=150); tree.column('level', width=50)
        vs = ttk.Scrollbar(f, orient=tk.VERTICAL, command=tree.yview)
        tree.config(yscrollcommand=vs.set)
        vs.pack(side=tk.RIGHT, fill=tk.Y); tree.pack(fill=tk.BOTH, expand=True)

        for box in range(14):
            for slot in range(30):
                off = 4 + (box * 30 + slot) * 80
                if off + 80 > len(self.ps): break
                raw = self.ps[off:off+80] + bytes(20)  # pad to 100 for decrypt
                info = decrypt_pokemon(raw)
                if info['species'] > 0 and info['species'] < 412:
                    tree.insert('', tk.END, values=(box+1, slot+1, info['species_name'], info.get('level',50)))

    def _trainer_tab(self):
        f = ttk.Frame(self.nb); self.nb.add(f, text="Trainer")
        name = decode_gba_str(self.sb2[0:8])
        gender = "Male" if self.sb2[8] == 0 else "Female"
        otid = struct.unpack_from('<I', self.sb2, 10)[0]
        hours = struct.unpack_from('<H', self.sb2, 14)[0]
        mins = self.sb2[16]
        money = struct.unpack_from('<I', self.sb1, SB1_MONEY)[0]

        info = [
            ("Name", name), ("Gender", gender),
            ("Trainer ID", str(otid & 0xFFFF)), ("Secret ID", str(otid >> 16)),
            ("Play Time", f"{hours}h {mins}m"),
            ("Money", f"${money:,}"),
        ]

        self.trainer_vars = {}
        for label, value in info:
            row = ttk.Frame(f); row.pack(fill=tk.X, padx=10, pady=2)
            ttk.Label(row, text=f"{label}:", width=15, anchor='e').pack(side=tk.LEFT)
            sv = tk.StringVar(value=value)
            ttk.Entry(row, textvariable=sv, width=30).pack(side=tk.LEFT, padx=5)
            self.trainer_vars[label] = sv

        ttk.Button(f, text="Apply Changes", command=self._apply_trainer).pack(pady=10)

    def _apply_trainer(self):
        try:
            money = int(self.trainer_vars["Money"].get().replace("$","").replace(",",""))
            struct.pack_into('<I', self.sb1, SB1_MONEY, min(money, 999999))
            self.info_var.set("Money updated! Save to write to file.")
        except: pass

    def _flags_tab(self):
        f = ttk.Frame(self.nb); self.nb.add(f, text="Flags")

        flags = [
            ("Badge 1 (Boulder)", 0x820), ("Badge 2 (Cascade)", 0x821),
            ("Badge 3 (Thunder)", 0x822), ("Badge 4 (Rainbow)", 0x823),
            ("Badge 5 (Soul)", 0x824), ("Badge 6 (Marsh)", 0x825),
            ("Badge 7 (Volcano)", 0x826), ("Badge 8 (Earth)", 0x827),
            ("Pokedex Obtained", 0x829), ("Game Clear (E4)", 0x82C),
            ("National Dex", 0x840),
        ]

        self.flag_vars = {}
        for label, flag_id in flags:
            off = SB1_FLAGS + (flag_id >> 3)
            is_set = bool(self.sb1[off] & (1 << (flag_id & 7))) if off < len(self.sb1) else False
            var = tk.BooleanVar(value=is_set)
            self.flag_vars[flag_id] = var
            ttk.Checkbutton(f, text=label, variable=var).pack(anchor='w', padx=10, pady=1)

        ttk.Button(f, text="Apply Flags", command=self._apply_flags).pack(pady=10)

    def _apply_flags(self):
        for flag_id, var in self.flag_vars.items():
            off = SB1_FLAGS + (flag_id >> 3)
            if off >= len(self.sb1): continue
            if var.get():
                self.sb1[off] |= (1 << (flag_id & 7))
            else:
                self.sb1[off] &= ~(1 << (flag_id & 7))
        self.info_var.set("Flags updated! Save to write to file.")

    def save(self):
        if not self.save_path or not self.save_data: return
        self._write_back()
        open(self.save_path, 'wb').write(self.save_data)
        self.info_var.set(f"Saved: {self.save_path}")

    def save_as(self):
        path = filedialog.asksaveasfilename(defaultextension=".sav", filetypes=[("Save files","*.sav")])
        if not path: return
        self.save_path = path
        self.save()

    def _write_back(self):
        for slot_base in [0, 14]:
            for s in range(14):
                phys = (slot_base + s) * SECTOR_SIZE
                sig = struct.unpack_from('<I', self.save_data, phys + 0xFF8)[0]
                if sig != SECTOR_SIG: continue
                sid = struct.unpack_from('<H', self.save_data, phys + 0xFF4)[0]
                if sid >= len(SIZES): continue
                sz = SIZES[sid]

                if sid == 0: src = self.sb2[:sz]
                elif 1 <= sid <= 4:
                    d = (sid-1)*SDS; src = self.sb1[d:d+sz]
                elif 5 <= sid <= 13:
                    d = (sid-5)*SDS; src = self.ps[d:d+sz]
                else: continue

                self.save_data[phys:phys+len(src)] = src
                cs = checksum(self.save_data[phys:phys+SDS], sz)
                struct.pack_into('<H', self.save_data, phys + 0xFF6, cs)

def main():
    root = tk.Tk()
    app = SaveEditor(root)
    if len(sys.argv) > 1 and os.path.exists(sys.argv[1]):
        app.save_path = sys.argv[1]
        app.save_data = bytearray(open(sys.argv[1], 'rb').read())
        app.open_save.__wrapped__ = True  # hack
    root.mainloop()

if __name__ == '__main__':
    main()
