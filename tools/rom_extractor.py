#!/usr/bin/env python3
"""
Pokemon FireRed ROM Extractor
Extracts all game assets from a .pkmn file or raw ROM into organized folders.
This is step 1 of the Ship of Harkinian-style asset pipeline.
"""
import struct, os, sys, json
from PIL import Image

PKMN_MAGIC = 0x4E4D4B50

# FireRed US ROM offsets (BPRE)
ROM_OFFSETS = {
    # Pokemon sprite tables (FireRed Rev 1 BPRE)
    'mon_front_pic_table':      0x234778,
    'mon_back_pic_table':       0x234DE8,
    'mon_palette_table':        0x235458,
    'mon_shiny_palette_table':  0x235AC8,
    'mon_icon_table':           0x23CB38,
    'mon_icon_palette_ids':     0x23D138,
    'mon_icon_palettes':        0x23D2BC,
    'mon_footprint_table':      0x2358E8,

    # Species data
    'base_stats':               0x2547A0,
    'species_names':            0x245F50,

    # Move data
    'move_data':                0x250C74,
    'move_names':               0x247104,

    # Trainer data
    'trainer_front_pic_table':  0x23B2C8,
    'trainer_back_pic_table':   0x23B398,
    'trainer_palette_table':    0x23B468,
    'trainer_data':             0x23FA28,

    # Map data
    'map_group_table':          0x3526A8,

    # Music
    'song_table':               0x4A18FC,
}

NUM_SPECIES = 412  # Including forms
NUM_MOVES = 355
NUM_TRAINERS = 743
NUM_ITEMS = 375
NUM_MAPS = 428
NUM_SONGS = 350
SPECIES_NAME_LEN = 11
MOVE_NAME_LEN = 13

class RomExtractor:
    def __init__(self, rom_data):
        self.rom = rom_data
        self.rom_size = len(rom_data)

    def read_u8(self, off):
        return self.rom[off] if off < self.rom_size else 0

    def read_u16(self, off):
        if off + 2 > self.rom_size: return 0
        return struct.unpack_from('<H', self.rom, off)[0]

    def read_u32(self, off):
        if off + 4 > self.rom_size: return 0
        return struct.unpack_from('<I', self.rom, off)[0]

    def rom_ptr(self, off):
        val = self.read_u32(off)
        if val >= 0x08000000 and val < 0x0A000000:
            return val & 0x01FFFFFF
        return None

    def lz77_decompress(self, offset):
        if offset >= self.rom_size or self.rom[offset] != 0x10:
            return None
        size = self.rom[offset+1] | (self.rom[offset+2] << 8) | (self.rom[offset+3] << 16)
        if size == 0 or size > 0x40000:
            return None
        out = bytearray()
        pos = offset + 4
        while len(out) < size and pos < self.rom_size:
            flags = self.rom[pos]; pos += 1
            for i in range(8):
                if len(out) >= size: break
                if flags & 0x80:
                    if pos + 1 >= self.rom_size: return bytes(out)
                    b1, b2 = self.rom[pos], self.rom[pos+1]; pos += 2
                    length = (b1 >> 4) + 3
                    disp = ((b1 & 0xF) << 8) | b2
                    for j in range(length):
                        if len(out) >= size: break
                        ref = len(out) - disp - 1
                        out.append(out[ref] if ref >= 0 else 0)
                else:
                    if pos >= self.rom_size: return bytes(out)
                    out.append(self.rom[pos]); pos += 1
                flags <<= 1
        return bytes(out)

    def read_palette(self, offset, compressed=True):
        if compressed:
            data = self.lz77_decompress(offset)
            if not data or len(data) < 32:
                data = self.rom[offset:offset+32]
        else:
            data = self.rom[offset:offset+32]
        colors = []
        for i in range(0, min(len(data), 32), 2):
            c = struct.unpack_from('<H', data, i)[0]
            colors.append(((c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, ((c >> 10) & 0x1F) << 3))
        while len(colors) < 16:
            colors.append((0, 0, 0))
        return colors

    def decode_4bpp_to_image(self, data, palette, width_tiles=8):
        tiles = []
        for t in range(0, len(data), 32):
            tile = []
            for row in range(8):
                for col in range(0, 8, 2):
                    idx = t + row * 4 + col // 2
                    byte = data[idx] if idx < len(data) else 0
                    tile.append(byte & 0xF)
                    tile.append((byte >> 4) & 0xF)
            tiles.append(tile)
        if not tiles: return Image.new('RGBA', (8, 8))
        wt = width_tiles
        ht = max(1, len(tiles) // wt)
        img = Image.new('RGBA', (wt*8, ht*8), (0,0,0,0))
        px = img.load()
        for ty in range(ht):
            for tx in range(wt):
                ti = ty * wt + tx
                if ti >= len(tiles): continue
                tile = tiles[ti]
                for py in range(8):
                    for ppx in range(8):
                        ci = tile[py*8+ppx]
                        x, y = tx*8+ppx, ty*8+py
                        if ci == 0: px[x,y] = (0,0,0,0)
                        else: px[x,y] = (*palette[ci % 16], 255)
        return img

    def save_palette_png(self, palette, path):
        img = Image.new('RGB', (16 * 16, 16))
        px = img.load()
        for i, c in enumerate(palette):
            for y in range(16):
                for x in range(16):
                    px[i*16+x, y] = c
        img.save(path)

    def read_gba_string(self, offset, max_len=32):
        chars = []
        for i in range(max_len):
            b = self.rom[offset + i]
            if b == 0xFF: break
            if 0xBB <= b <= 0xD4: chars.append(chr(ord('A') + b - 0xBB))
            elif 0xD5 <= b <= 0xEE: chars.append(chr(ord('a') + b - 0xD5))
            elif 0xA1 <= b <= 0xAA: chars.append(chr(ord('0') + b - 0xA1))
            elif b == 0x00: chars.append(' ')
            elif b == 0xAB: chars.append('!')
            elif b == 0xAC: chars.append('?')
            elif b == 0xAD: chars.append('.')
            elif b == 0xAE: chars.append('-')
            elif b == 0xB4: chars.append("'")
            elif b == 0xB1: chars.append('x')  # multiplication sign
            else: chars.append(f'[{b:02X}]')
        return ''.join(chars).strip()

    # ---- Extraction methods ----

    def extract_pokemon_sprites(self, out_dir):
        print("Extracting Pokemon sprites...")
        species_dir = os.path.join(out_dir, 'pokemon')
        os.makedirs(species_dir, exist_ok=True)

        names = []
        for i in range(NUM_SPECIES):
            name = self.read_gba_string(ROM_OFFSETS['species_names'] + i * SPECIES_NAME_LEN)
            names.append(name if name else f"SPECIES_{i:03d}")

        count = 0
        for i in range(1, NUM_SPECIES):
            name = names[i].replace(' ', '_').replace("'", '').lower()
            name = ''.join(c for c in name if c.isalnum() or c in '_-')
            sdir = os.path.join(species_dir, f"{i:03d}_{name}")
            os.makedirs(sdir, exist_ok=True)

            # Front sprite
            front_ptr = self.rom_ptr(ROM_OFFSETS['mon_front_pic_table'] + i * 8)
            if front_ptr:
                data = self.lz77_decompress(front_ptr)
                if data:
                    pal_ptr = self.rom_ptr(ROM_OFFSETS['mon_palette_table'] + i * 8)
                    pal = self.read_palette(pal_ptr) if pal_ptr else [(j*17,j*17,j*17) for j in range(16)]
                    img = self.decode_4bpp_to_image(data, pal, 8)
                    img.save(os.path.join(sdir, 'front.png'))

                    # Shiny palette
                    shiny_ptr = self.rom_ptr(ROM_OFFSETS['mon_shiny_palette_table'] + i * 8)
                    if shiny_ptr:
                        shiny_pal = self.read_palette(shiny_ptr)
                        shiny_img = self.decode_4bpp_to_image(data, shiny_pal, 8)
                        shiny_img.save(os.path.join(sdir, 'front_shiny.png'))

            # Back sprite
            back_ptr = self.rom_ptr(ROM_OFFSETS['mon_back_pic_table'] + i * 8)
            if back_ptr:
                data = self.lz77_decompress(back_ptr)
                if data:
                    pal_ptr = self.rom_ptr(ROM_OFFSETS['mon_palette_table'] + i * 8)
                    pal = self.read_palette(pal_ptr) if pal_ptr else [(j*17,j*17,j*17) for j in range(16)]
                    img = self.decode_4bpp_to_image(data, pal, 8)
                    img.save(os.path.join(sdir, 'back.png'))

                    shiny_ptr = self.rom_ptr(ROM_OFFSETS['mon_shiny_palette_table'] + i * 8)
                    if shiny_ptr:
                        shiny_pal = self.read_palette(shiny_ptr)
                        shiny_img = self.decode_4bpp_to_image(data, shiny_pal, 8)
                        shiny_img.save(os.path.join(sdir, 'back_shiny.png'))

            # Icon
            icon_ptr = self.rom_ptr(ROM_OFFSETS['mon_icon_table'] + i * 4)
            if icon_ptr:
                data = self.rom[icon_ptr:icon_ptr + 512]  # icons are 32x64, uncompressed
                pal_id = self.rom[ROM_OFFSETS['mon_icon_palette_ids'] + i]
                pal_ptr = self.rom_ptr(ROM_OFFSETS['mon_icon_palettes'] + pal_id * 8)
                pal = self.read_palette(pal_ptr, compressed=False) if pal_ptr else [(j*17,j*17,j*17) for j in range(16)]
                img = self.decode_4bpp_to_image(data, pal, 4)
                img.save(os.path.join(sdir, 'icon.png'))

            count += 1
            if count % 50 == 0:
                print(f"  {count}/{NUM_SPECIES-1} Pokemon...")

        print(f"  Extracted {count} Pokemon sprites")

    def extract_pokemon_data(self, out_dir):
        print("Extracting Pokemon data...")
        data_dir = os.path.join(out_dir, 'data')
        os.makedirs(data_dir, exist_ok=True)

        pokemon = []
        for i in range(NUM_SPECIES):
            name = self.read_gba_string(ROM_OFFSETS['species_names'] + i * SPECIES_NAME_LEN)
            off = ROM_OFFSETS['base_stats'] + i * 28
            stats = struct.unpack_from('<BBBBBBBBBBHBBBBBB', self.rom, off)
            pokemon.append({
                'id': i, 'name': name,
                'hp': stats[0], 'attack': stats[1], 'defense': stats[2],
                'speed': stats[3], 'sp_attack': stats[4], 'sp_defense': stats[5],
                'type1': stats[6], 'type2': stats[7],
                'catch_rate': stats[8], 'exp_yield': stats[9],
            })

        with open(os.path.join(data_dir, 'pokemon.json'), 'w') as f:
            json.dump(pokemon, f, indent=2)
        print(f"  Extracted {len(pokemon)} Pokemon entries")

    def extract_move_data(self, out_dir):
        print("Extracting move data...")
        data_dir = os.path.join(out_dir, 'data')
        os.makedirs(data_dir, exist_ok=True)

        moves = []
        for i in range(NUM_MOVES):
            name = self.read_gba_string(ROM_OFFSETS['move_names'] + i * MOVE_NAME_LEN)
            off = ROM_OFFSETS['move_data'] + i * 12
            data = struct.unpack_from('<BBBBBBBBBBBB', self.rom, off)
            moves.append({
                'id': i, 'name': name,
                'effect': data[0], 'power': data[1], 'type': data[2],
                'accuracy': data[3], 'pp': data[4], 'effect_chance': data[5],
                'target': data[6], 'priority': struct.unpack('b', bytes([data[7]]))[0],
                'flags': data[8],
            })

        with open(os.path.join(data_dir, 'moves.json'), 'w') as f:
            json.dump(moves, f, indent=2)
        print(f"  Extracted {len(moves)} moves")

    def extract_trainer_sprites(self, out_dir):
        print("Extracting trainer sprites...")
        trainer_dir = os.path.join(out_dir, 'trainers')
        os.makedirs(trainer_dir, exist_ok=True)

        count = 0
        for i in range(NUM_TRAINERS):
            sdir = os.path.join(trainer_dir, f"{i:03d}")

            # Front sprite
            front_ptr = self.rom_ptr(ROM_OFFSETS['trainer_front_pic_table'] + i * 8)
            if front_ptr:
                data = self.lz77_decompress(front_ptr)
                if data and len(data) >= 32:
                    pal_ptr = self.rom_ptr(ROM_OFFSETS['trainer_palette_table'] + i * 8)
                    pal = self.read_palette(pal_ptr) if pal_ptr else [(j*17,j*17,j*17) for j in range(16)]
                    os.makedirs(sdir, exist_ok=True)
                    img = self.decode_4bpp_to_image(data, pal, 8)
                    img.save(os.path.join(sdir, 'front.png'))
                    count += 1

        print(f"  Extracted {count} trainer sprites")

    def extract_all(self, out_dir):
        print(f"\n=== Pokemon FireRed ROM Extractor ===")
        print(f"ROM size: {self.rom_size // 1024}KB")
        print(f"Output: {out_dir}\n")

        os.makedirs(out_dir, exist_ok=True)

        self.extract_pokemon_sprites(out_dir)
        self.extract_pokemon_data(out_dir)
        self.extract_move_data(out_dir)
        self.extract_trainer_sprites(out_dir)

        print(f"\nExtraction complete!")


def load_pkmn(path):
    with open(path, 'rb') as f:
        data = f.read()
    magic = struct.unpack_from('<I', data, 0)[0]
    if magic == PKMN_MAGIC:
        rom_size = struct.unpack_from('<I', data, 12)[0]
        return data[64:64+rom_size]
    elif data[0xAC:0xB0] == b'BPRE' or data[0xAC:0xB0] == b'BPGE':
        return data
    else:
        raise ValueError("Not a valid .pkmn or GBA ROM file")


def main():
    if len(sys.argv) < 2:
        print("Usage: rom_extractor.py <pokefirered.pkmn> [output_dir]")
        print("       rom_extractor.py <pokefirered.gba> [output_dir]")
        sys.exit(1)

    rom_path = sys.argv[1]
    out_dir = sys.argv[2] if len(sys.argv) > 2 else 'extracted_assets'

    rom = load_pkmn(rom_path)
    extractor = RomExtractor(rom)
    extractor.extract_all(out_dir)


if __name__ == '__main__':
    main()
