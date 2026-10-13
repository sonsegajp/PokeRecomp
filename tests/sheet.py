"""tests/sheet.py DIR OUT.png [cols] — contact sheet of the BMP shots in DIR (with frame numbers)."""
import sys, glob, os
from PIL import Image, ImageDraw
files = sorted(glob.glob(os.path.join(sys.argv[1], "*.bmp")))
cols = int(sys.argv[3]) if len(sys.argv) > 3 else 6
w, h = 240, 160
rows = (len(files) + cols - 1) // cols
sheet = Image.new("RGB", (cols * w, rows * (h + 12)), "black")
d = ImageDraw.Draw(sheet)
for i, f in enumerate(files):
    x, y = (i % cols) * w, (i // cols) * (h + 12)
    sheet.paste(Image.open(f).convert("RGB").resize((w, h)), (x, y + 12))
    d.text((x + 2, y), os.path.basename(f)[1:-4], fill="yellow")
sheet.save(sys.argv[2])
