"""Export original FRLG PC assets; decode tiles/palettes, never redraw game artwork.
Requires Pillow only at asset-build time. Runtime uses the exported static PNGs.
"""
from pathlib import Path
import json
import re
import shutil
import struct
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
GAME = ROOT / 'game'
OUT = ROOT / 'launcher/assets/storage'
for folder in ('icons', 'front', 'wallpapers'):
    (OUT / folder).mkdir(parents=True, exist_ok=True)

def text(path):
    return (GAME / path).read_text(encoding='utf-8')

def palette(path):
    raw = path.read_bytes()
    return [((c & 31)*255//31, ((c>>5)&31)*255//31, ((c>>10)&31)*255//31, 255)
            for (c,) in struct.iter_unpack('<H', raw)]

def tiles_image(raw, colors, width, height, transparent=True):
    image = Image.new('RGBA', (width, height))
    for y in range(height):
        for x in range(width):
            tile = (y//8)*(width//8)+(x//8)
            offset = tile*32 + (y%8)*4 + (x%8)//2
            if offset >= len(raw):
                continue
            value = (raw[offset] >> (4*(x%2))) & 15
            image.putpixel((x,y), (0,0,0,0) if transparent and value==0 else colors[value])
    return image

ids = {name:int(value) for name,value in re.findall(r'#define SPECIES_(\w+)\s+(\d+)\b',text('include/constants/species.h'))}
graphics = {}
for name,path in re.findall(r'(gMon\w+)\[\]\s*=\s*INCBIN_U\d+\(\s*"([^"]+)"',text('src/data/graphics/pokemon.h')):
    graphics.setdefault(name,path)  # FIRERED definitions precede LEAFGREEN variants.
fronts = dict(re.findall(r'SPECIES_SPRITE\((\w+),\s*(gMonFrontPic_\w+)\)',text('src/data/pokemon_graphics/front_pic_table.h')))
palettes = dict(re.findall(r'SPECIES_PAL\((\w+),\s*(gMonPalette_\w+)\)',text('src/data/pokemon_graphics/palette_table.h')))
icon_source=text('src/pokemon_icon.c')
icons=dict(re.findall(r'\[SPECIES_(\w+)\]\s*=\s*(gMonIcon_\w+)',icon_source))
icon_indices={name:int(index) for name,index in re.findall(r'\[SPECIES_(\w+)\]\s*=\s*(\d+),',icon_source)}
icon_palettes=[palette(GAME/f'graphics/pokemon/icon_palettes/icon_palette_{i}.gbapal') for i in range(3)]
names=dict(re.findall(r'\[SPECIES_(\w+)\]\s*=\s*_\("([^"]+)"\)',text('src/data/text/species_names.h')))
stats=dict(re.findall(r'\[SPECIES_(\w+)\]\s*=\s*\{(.*?)\n\s*\},',text('src/data/pokemon/species_info.h'),re.S))
metadata={}
for name,number in ids.items():
    if name not in fronts or name not in icons or name not in palettes:
        continue
    front_path=graphics.get(fronts[name]); icon_path=graphics.get(icons[name]); pal_path=graphics.get(palettes[name])
    if not front_path or not icon_path or not pal_path:
        continue
    front_file=GAME/front_path.removesuffix('.lz')
    pal_file=GAME/pal_path.removesuffix('.lz')
    if not front_file.is_file() or not pal_file.is_file():
        continue
    tiles_image(front_file.read_bytes(),palette(pal_file),64,64).save(OUT/'front'/f'{number}.png')
    tiles_image((GAME/icon_path).read_bytes(),icon_palettes[icon_indices.get(name,0)],32,32).save(OUT/'icons'/f'{number}.png')
    info=stats.get(name,'')
    growth=re.search(r'\.growthRate\s*=\s*GROWTH_(\w+)',info)
    genders=re.search(r'\.genderRatio\s*=\s*([^,]+)',info)
    gender=255
    if genders:
        value=genders[1]
        percent=re.search(r'PERCENT_FEMALE\(([\d.]+)\)',value)
        if percent: gender=min(254,int(float(percent[1])*255/100))
        elif value.strip().isdigit(): gender=int(value.strip())
    metadata[number]={'name':names.get(name,name.replace('_',' ')),
                      'types':re.findall(r'TYPE_(\w+)',info)[:2],
                      'growthRate':growth[1] if growth else 'MEDIUM_FAST',
                      'genderRatio':gender}

wallpapers=['forest','city','desert','savanna','crag','volcano','snow','cave','beach','seafloor','river','sky','stars','pokecenter','tiles','simple']
wallpaper_manifest=[]
for number,name in enumerate(wallpapers):
    folder=GAME/'graphics/pokemon_storage/wallpapers'/name
    if not folder.is_dir():
        continue
    raw=(folder/'tiles.4bpp').read_bytes(); colors=palette(folder/'tiles.gbapal')
    entries=struct.unpack('<360H',(folder/'tilemap.bin').read_bytes())
    image=Image.new('RGBA',(160,144))
    for i,entry in enumerate(entries):
        tile=entry&1023; bank=(entry>>12)-1
        for y in range(8):
            for x in range(8):
                px=7-x if entry&1024 else x;py=7-y if entry&2048 else y
                value=(raw[tile*32+py*4+px//2]>>(4*(px%2)))&15
                color=colors[bank*16+value] if bank in (0,1) else (0,0,0,0)
                image.putpixel(((i%20)*8+x,(i//20)*8+y),color)
    image.save(OUT/'wallpapers'/f'{number}.png')
    wallpaper_manifest.append({'id':number,'name':name,'source':str(folder.relative_to(ROOT))})

storage=GAME/'graphics/pokemon_storage'
tiles_image((storage/'cursor.4bpp').read_bytes(),palette(storage/'misc1.gbapal'),32,32).save(OUT/'cursor.png')
for filename in ('box_scroll_arrow.png','cursor_shadow.png','waveform.png'):
    shutil.copy2(storage/filename,OUT/filename)
(OUT/'species.json').write_text(json.dumps(metadata,ensure_ascii=False,separators=(',',':')),encoding='utf-8')
(OUT/'manifest.json').write_text(json.dumps({'source':'Local pret pokefirered decompilation','species':len(metadata),'wallpapers':wallpaper_manifest,'method':'Original tile order, original BGR555 palettes, original tilemap flip bits. Palette index zero is transparent for sprites.'},indent=2),encoding='utf-8')
print(f'Exported {len(metadata)} original species sprites, {len(wallpaper_manifest)} original PC wallpapers, and original cursor assets.')
