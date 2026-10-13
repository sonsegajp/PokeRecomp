"""game_ranges.py MAP OUT DIR... — the exe's .data/.rdata bytes that come from the game.

Lists (rva, size) for every data section the linker took from an object under
one of the DIRs (the game's C objects and its converted ARM data), so rompack
only ever takes game content out — never the C runtime's or a library's
bytes, which are used before main() can put anything back.
"""
import re
import sys

IMAGE_BASE = 0x400000
LINE = re.compile(r'^ (\.r?data\S*)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S.*)$')
NAME_ONLY = re.compile(r'^ (\.r?data\S*)\s*$')
REST = re.compile(r'^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S.*)$')

map_path, out_path, dirs = sys.argv[1], sys.argv[2], [d.replace('\\', '/') for d in sys.argv[3:]]
ranges = []
pending = None
for line in open(map_path, encoding='utf-8', errors='replace'):
    line = line.rstrip('\n')
    m = LINE.match(line)
    if m:
        name, addr, size, obj = m.groups()
    elif pending and REST.match(line):
        name = pending
        addr, size, obj = REST.match(line).groups()
    else:
        n = NAME_ONLY.match(line)
        pending = n.group(1) if n else None
        continue
    pending = None
    addr, size = int(addr, 16), int(size, 16)
    obj = obj.replace('\\', '/')
    if size and addr >= IMAGE_BASE and any(obj.startswith(d) for d in dirs):
        ranges.append((addr - IMAGE_BASE, size))

ranges.sort()
with open(out_path, 'w') as f:
    for rva, size in ranges:
        f.write('%x %x\n' % (rva, size))
print('game_ranges: %d sections, %d bytes' % (len(ranges), sum(s for _, s in ranges)))
