#!/usr/bin/env python3
"""Convert GBA 4bpp tile graphics + gbapal palette to PNG."""
import struct, os
from PIL import Image

def read_gbapal(path):
    data = open(path, 'rb').read()
    colors = []
    for i in range(0, min(len(data), 32), 2):
        c = struct.unpack_from('<H', data, i)[0]
        r = (c & 0x1F) << 3
        g = ((c >> 5) & 0x1F) << 3
        b = ((c >> 10) & 0x1F) << 3
        colors.append((r, g, b))
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors

def convert(gfx_path, pal_path, out_path, width_tiles=2):
    palette = read_gbapal(pal_path)
    data = open(gfx_path, 'rb').read()

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

    wt = width_tiles
    ht = max(1, len(tiles) // wt)
    img = Image.new('RGBA', (wt * 8, ht * 8), (0, 0, 0, 0))
    px = img.load()
    for ty in range(ht):
        for tx in range(wt):
            ti = ty * wt + tx
            if ti >= len(tiles): continue
            tile = tiles[ti]
            for py in range(8):
                for ppx in range(8):
                    ci = tile[py * 8 + ppx]
                    x, y = tx * 8 + ppx, ty * 8 + py
                    if ci == 0:
                        px[x, y] = (0, 0, 0, 0)
                    else:
                        px[x, y] = (*palette[ci], 255)
    img.save(out_path)
    print(f"  {os.path.basename(out_path):40s} {wt*8}x{ht*8}")

if __name__ == '__main__':
    base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dest = os.path.join(base, 'deploy', 'assets', 'players', 'png')
    os.makedirs(dest, exist_ok=True)

    gfx = lambda p: os.path.join(base, p.replace('/', os.sep))
    out = lambda n: os.path.join(dest, n)
    ppl = 'graphics/object_events/pics/people'
    pal = 'graphics/object_events/palettes'
    trn = 'graphics/trainers'

    pal_player = gfx(f'{pal}/player.gbapal')
    pal_brendan = gfx(f'{ppl}/rs_brendan_full.gbapal')
    pal_may = gfx(f'{ppl}/rs_may_full.gbapal')

    # Red overworld (all variants)
    print("=== Red ===")
    for name, w in [('red_normal', 2), ('red_bike', 2), ('red_surf', 2),
                    ('red_surf_run', 2), ('red_fish', 2), ('red_item', 2),
                    ('red_vs_seeker_bike', 2)]:
        path = gfx(f'{ppl}/{name}.4bpp')
        if os.path.exists(path):
            convert(path, pal_player, out(f'{name}.png'), w)

    # Leaf overworld (all variants)
    print("=== Leaf ===")
    for name, w in [('green_normal', 2), ('green_bike', 2), ('green_surf', 2),
                    ('green_surf_run', 2), ('green_fish', 2), ('green_item', 2),
                    ('green_vs_seeker_bike', 2)]:
        path = gfx(f'{ppl}/{name}.4bpp')
        if os.path.exists(path):
            convert(path, pal_player, out(f'{name.replace("green", "leaf")}.png'), w)

    # Brendan overworld
    print("=== Brendan ===")
    for name in ['rs_brendan', 'rs_brendan_full']:
        path = gfx(f'{ppl}/{name}.4bpp')
        if os.path.exists(path):
            convert(path, pal_brendan, out(f'{name}.png'), 2)

    # May overworld
    print("=== May ===")
    for name in ['rs_may', 'rs_may_full']:
        path = gfx(f'{ppl}/{name}.4bpp')
        if os.path.exists(path):
            convert(path, pal_may, out(f'{name}.png'), 2)

    # Battle back sprites
    print("=== Battle Backs ===")
    convert(gfx(f'{trn}/back_pics/red_back_pic.4bpp'),
            gfx(f'{trn}/palettes/red.gbapal'), out('red_back.png'), 8)
    convert(gfx(f'{trn}/back_pics/leaf_back_pic.4bpp'),
            gfx(f'{trn}/palettes/leaf.gbapal'), out('leaf_back.png'), 8)
    convert(gfx(f'{trn}/back_pics/ruby_sapphire_brendan_back_pic.4bpp'),
            gfx(f'{trn}/palettes/ruby_sapphire_brendan.gbapal'), out('brendan_back.png'), 8)
    convert(gfx(f'{trn}/back_pics/ruby_sapphire_may_back_pic.4bpp'),
            gfx(f'{trn}/palettes/ruby_sapphire_may.gbapal'), out('may_back.png'), 8)

    print("\nDone!")
