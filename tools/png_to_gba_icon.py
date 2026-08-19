"""Convert a PNG to GBA item icon format (24x24 4bpp tiles + 16-color palette)."""
import sys, struct
from PIL import Image

def lz77_wrap(data):
    """Wrap raw data in LZ77 format using only literal bytes."""
    out = bytearray()
    out += struct.pack('<I', 0x10 | (len(data) << 8))
    pos = 0
    while pos < len(data):
        flag = 0x00
        chunk = bytearray([flag])
        for bit in range(8):
            if pos < len(data):
                chunk.append(data[pos])
                pos += 1
            else:
                chunk.append(0)
        out += chunk
    while len(out) % 4:
        out.append(0)
    return bytes(out)

def rgb_to_gba(r, g, b):
    return ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))

def convert(png_path):
    img = Image.open(png_path).convert('RGBA')
    img = img.resize((24, 24), Image.LANCZOS)

    # Separate alpha channel, create RGB with transparent pixels as magenta
    alpha = img.split()[3]
    rgb = Image.new('RGB', (24, 24), (0, 0, 0))
    for y in range(24):
        for x in range(24):
            r, g, b, a = img.getpixel((x, y))
            if a >= 128:
                rgb.putpixel((x, y), (r, g, b))

    # Quantize to 15 colors (index 0 reserved for transparent)
    # Count actual unique opaque colors
    opaque_colors = set()
    for y in range(24):
        for x in range(24):
            r, g, b, a = img.getpixel((x, y))
            if a >= 128:
                opaque_colors.add((r & 0xF8, g & 0xF8, b & 0xF8))
    num_colors = min(len(opaque_colors), 15)
    if num_colors < 1:
        num_colors = 1
    quantized = rgb.quantize(colors=num_colors, method=Image.Quantize.MEDIANCUT)
    qpal = quantized.getpalette()[:num_colors*3]

    # Build GBA palette: index 0 = transparent (black), then quantized colors
    palette = [(0, 0, 0)]
    for i in range(num_colors):
        palette.append((qpal[i*3], qpal[i*3+1], qpal[i*3+2]))
    while len(palette) < 16:
        palette.append((0, 0, 0))

    # Map each pixel to palette index
    indexed = []
    for y in range(24):
        for x in range(24):
            r, g, b, a = img.getpixel((x, y))
            if a < 128:
                indexed.append(0)
            else:
                qi = quantized.getpixel((x, y))
                indexed.append(qi + 1)  # +1 because index 0 is transparent

    # Convert to GBA 4bpp tile format (3x3 tiles of 8x8)
    tile_data = bytearray()
    for ty in range(3):
        for tx in range(3):
            for row in range(8):
                for col in range(0, 8, 2):
                    px = tx * 8 + col
                    py = ty * 8 + row
                    if px < 24 and py < 24:
                        lo = indexed[py * 24 + px]
                        hi = indexed[py * 24 + px + 1] if px + 1 < 24 else 0
                    else:
                        lo = hi = 0
                    tile_data.append((hi << 4) | lo)

    # Build palette data
    pal_data = bytearray()
    for r, g, b in palette:
        pal_data += struct.pack('<H', rgb_to_gba(r, g, b))

    tile_lz = lz77_wrap(bytes(tile_data))
    pal_lz = lz77_wrap(bytes(pal_data))

    def to_c_array(name, data):
        words = []
        for i in range(0, len(data), 4):
            chunk = data[i:i+4]
            while len(chunk) < 4:
                chunk += b'\x00'
            words.append(struct.unpack('<I', chunk)[0])
        lines = [f"const u32 {name}[] = {{"]
        for i in range(0, len(words), 8):
            row = ", ".join(f"0x{w:08X}" for w in words[i:i+8])
            lines.append(f"    {row},")
        lines.append("};")
        return "\n".join(lines)

    name = sys.argv[2] if len(sys.argv) > 2 else "ShinyCharm"
    print(to_c_array(f"gItemIcon_{name}", tile_lz))
    print()
    print(to_c_array(f"gItemIconPalette_{name}", pal_lz))

if __name__ == "__main__":
    convert(sys.argv[1] if len(sys.argv) > 1 else "custom_assets/gen3/key_items/Key_Shiny_Charm_Sprite.png")
