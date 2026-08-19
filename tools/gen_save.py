#!/usr/bin/env python3
"""Generate a completed Pokemon FireRed save file."""
import struct, random, os, sys

SECTOR_DATA_SIZE = 3968
SECTOR_SIZE = 0x1000
NUM_SECTORS = 14
FLASH_SIZE = 0x20000
SECTOR_SIG = 0x08012025

# Exact struct sizes from compiler
SB2_SIZE = 3872; SB1_SIZE = 15592; PS_SIZE = 33744

# Sector data sizes (how many bytes of actual data per sector)
SECTOR_SIZES = [
    min(SB2_SIZE, SECTOR_DATA_SIZE),  # id 0: SaveBlock2
]
for i in range(4):  # ids 1-4: SaveBlock1
    r = SB1_SIZE - i * SECTOR_DATA_SIZE
    SECTOR_SIZES.append(min(r, SECTOR_DATA_SIZE) if r > 0 else 0)
for i in range(9):  # ids 5-13: PokemonStorage
    r = PS_SIZE - i * SECTOR_DATA_SIZE
    SECTOR_SIZES.append(min(r, SECTOR_DATA_SIZE) if r > 0 else 0)

# SaveBlock2 offsets
SB2_NAME = 0; SB2_GENDER = 8; SB2_TRID = 10; SB2_HOURS = 14; SB2_MINS = 16
SB2_POKEDEX = 24; PDEX_OWNED = 16; PDEX_SEEN = 68; PDEX_NATMAGIC = 3

# SaveBlock1 offsets
SB1_LOC = 4; SB1_HEAL = 28; SB1_PCNT = 52; SB1_PARTY = 56
SB1_MONEY = 656; SB1_COINS = 660
SB1_ITEMS = 784; SB1_KEYS = 952; SB1_BALLS = 1072; SB1_TMHM = 1124
SB1_FLAGS = 3872

def enc(text, n):
    o = bytearray()
    for c in text:
        if 'A' <= c <= 'Z': o.append(0xBB + ord(c) - ord('A'))
        elif 'a' <= c <= 'z': o.append(0xD5 + ord(c) - ord('a'))
        elif '0' <= c <= '9': o.append(0xA1 + ord(c) - ord('0'))
        else: o.append(0x00)
    o.append(0xFF)
    while len(o) < n: o.append(0x00)
    return bytes(o[:n])

SUBORDER = [
    [0,1,2,3],[0,1,3,2],[0,2,1,3],[0,3,1,2],[0,2,3,1],[0,3,2,1],
    [1,0,2,3],[1,0,3,2],[2,0,1,3],[3,0,1,2],[2,0,3,1],[3,0,2,1],
    [1,2,0,3],[1,3,0,2],[2,1,0,3],[3,1,0,2],[2,3,0,1],[3,2,0,1],
    [1,2,3,0],[1,3,2,0],[2,1,3,0],[3,1,2,0],[2,3,1,0],[3,2,1,0]]

def mkpkmn(species, level, moves, nick, ot, otid, box=False):
    pid = random.randint(1, 0xFFFFFFFE)
    g = bytearray(12)
    struct.pack_into('<H', g, 0, species)
    struct.pack_into('<I', g, 4, int(level**3 * 0.8))
    g[9] = 70
    a = bytearray(12)
    for i, m in enumerate(moves[:4]): struct.pack_into('<H', a, i*2, m)
    a[8:12] = bytes([35,20,15,10])
    e = bytearray(12)
    e[0:6] = bytes([252,252,100,100,100,100])
    m = bytearray(12)
    struct.pack_into('<H', m, 2, 5|(4<<11))
    struct.pack_into('<I', m, 4, 0x7FFF7FFF)
    subs = [g, a, e, m]
    order = SUBORDER[pid % 24]
    arr = bytearray()
    for idx in order: arr += subs[idx]
    sz = 80 if box else 100
    p = bytearray(sz)
    struct.pack_into('<I', p, 0, pid)
    struct.pack_into('<I', p, 4, otid)
    p[8:18] = enc(nick, 10)
    struct.pack_into('<H', p, 18, 0x0202)
    p[20:27] = enc(ot, 7)
    p[32:80] = arr
    cs = sum(struct.unpack_from('<H', p, i)[0] for i in range(32, 80, 2)) & 0xFFFF
    struct.pack_into('<H', p, 28, cs)
    key = pid ^ otid
    for i in range(32, 80, 4):
        val = struct.unpack_from('<I', p, i)[0]
        struct.pack_into('<I', p, i, val ^ key)
    if not box:
        p[84] = level
        hp = level*3+110
        struct.pack_into('<H', p, 86, hp)
        struct.pack_into('<H', p, 88, hp)
        for j, off in enumerate([90,92,94,96,98]):
            struct.pack_into('<H', p, off, level*2+25+j*3)
    return bytes(p)

def checksum(data, size):
    cs = 0
    for i in range(0, size & ~3, 4):
        cs = (cs + struct.unpack_from('<I', data, i)[0]) & 0xFFFFFFFF
    return ((cs >> 16) + cs) & 0xFFFF

def setflag(sb1, f):
    off = SB1_FLAGS + (f >> 3)
    if off < len(sb1): sb1[off] |= 1 << (f & 7)

def setdex(sb2, base, sp):
    off = SB2_POKEDEX + base + (sp >> 3)
    if off < len(sb2): sb2[off] |= 1 << (sp & 7)

def build():
    OT = "RED"; OTID = 12345 | (54321 << 16)
    # Moves
    FLAMETHROWER=53;FIRE_BLAST=126;DRAGON_CLAW=337;SLASH=163
    SURF=57;ICE_BEAM=58;HYDRO_PUMP=56;SKULL_BASH=130
    SOLAR_BEAM=76;SLUDGE_BOMB=188;EARTHQUAKE=89;RAZOR_LEAF=75
    REST=156;BODY_SLAM=34;HYPER_BEAM=63;SHADOW_BALL=247
    THUNDERBOLT=85;THUNDER=87;IRON_TAIL=231;QUICK_ATTACK=98
    FLY=19;AERIAL_ACE=332;RETURN=216;STEEL_WING=211

    sb2 = bytearray(SB2_SIZE)
    sb2[SB2_NAME:SB2_NAME+8] = enc(OT, 8)
    sb2[SB2_GENDER] = 0
    struct.pack_into('<I', sb2, SB2_TRID, OTID)
    struct.pack_into('<H', sb2, SB2_HOURS, 120)
    sb2[SB2_MINS] = 30
    sb2[SB2_POKEDEX + PDEX_NATMAGIC] = 0xDA
    for sp in range(1, 387):
        setdex(sb2, PDEX_OWNED, sp)
        setdex(sb2, PDEX_SEEN, sp)

    sb1 = bytearray(SB1_SIZE)
    # Pallet Town: MAP_PALLET_TOWN = (0 | (3 << 8)) = group 3, map 0
    sb1[SB1_LOC] = 3; sb1[SB1_LOC+1] = 0; sb1[SB1_LOC+2] = 0xFF
    struct.pack_into('<h', sb1, SB1_LOC+4, 5)
    struct.pack_into('<h', sb1, SB1_LOC+6, 6)
    sb1[SB1_HEAL] = 3; sb1[SB1_HEAL+1] = 0; sb1[SB1_HEAL+2] = 0xFF
    struct.pack_into('<h', sb1, SB1_HEAL+4, 5)
    struct.pack_into('<h', sb1, SB1_HEAL+6, 6)

    struct.pack_into('<I', sb1, SB1_PCNT, 6)
    team = [
        (6,75,[FLAMETHROWER,FIRE_BLAST,DRAGON_CLAW,SLASH],"CHARIZARD"),
        (9,75,[SURF,ICE_BEAM,HYDRO_PUMP,SKULL_BASH],"BLASTOISE"),
        (3,75,[SOLAR_BEAM,SLUDGE_BOMB,EARTHQUAKE,RAZOR_LEAF],"VENUSAUR"),
        (143,75,[REST,BODY_SLAM,HYPER_BEAM,SHADOW_BALL],"SNORLAX"),
        (26,75,[THUNDERBOLT,THUNDER,IRON_TAIL,QUICK_ATTACK],"RAICHU"),
        (18,75,[FLY,AERIAL_ACE,RETURN,STEEL_WING],"PIDGEOT")]
    for i,(sp,lv,mv,nk) in enumerate(team):
        sb1[SB1_PARTY+i*100:SB1_PARTY+i*100+100] = mkpkmn(sp,lv,mv,nk,OT,OTID)

    struct.pack_into('<I', sb1, SB1_MONEY, 999999)
    struct.pack_into('<H', sb1, SB1_COINS, 9999)

    off = SB1_ITEMS
    for i,(iid,cnt) in enumerate([(13,999),(14,999),(15,999),(16,999),(17,999),(18,999),(19,999),(20,999)]):
        struct.pack_into('<HH', sb1, off+i*4, iid, cnt)
    off = SB1_BALLS
    for i,(iid,cnt) in enumerate([(1,999),(2,999),(3,999),(4,999)]):
        struct.pack_into('<HH', sb1, off+i*4, iid, cnt)
    off = SB1_TMHM
    for i,iid in enumerate(range(289,347)):
        struct.pack_into('<HH', sb1, off+i*4, iid, 1)

    for b in range(0x820, 0x828): setflag(sb1, b)
    setflag(sb1, 0x829)  # POKEDEX_GET
    setflag(sb1, 0x82C)  # GAME_CLEAR
    setflag(sb1, 0x840)  # NATIONAL_DEX
    for f in range(0x800, 0x850): setflag(sb1, f)

    ps = bytearray(PS_SIZE)
    for sp in range(1, 387):
        off = 4 + (sp-1)*80
        if off+80 <= PS_SIZE:
            ps[off:off+80] = mkpkmn(sp,50,[1,0,0,0],f"MON{sp:03d}"[:10],OT,OTID,box=True)

    # Concatenate all data
    alldata = sb2 + sb1 + ps

    # Build flash image with both save slots
    flash = bytearray([0xFF] * FLASH_SIZE)
    counter = 2

    for slot in range(2):
        base_sector = slot * NUM_SECTORS
        for sid in range(NUM_SECTORS):
            phys = base_sector + sid
            sector_off = phys * SECTOR_SIZE

            # Get data for this sector ID
            if sid == 0:
                src = sb2[:SECTOR_SIZES[0]]
            elif sid <= 4:
                doff = (sid - 1) * SECTOR_DATA_SIZE
                src = sb1[doff:doff + SECTOR_SIZES[sid]]
            else:
                doff = (sid - 5) * SECTOR_DATA_SIZE
                src = ps[doff:doff + SECTOR_SIZES[sid]]

            # Clear sector
            flash[sector_off:sector_off + SECTOR_SIZE] = bytes(SECTOR_SIZE)

            # Write data
            flash[sector_off:sector_off + len(src)] = src

            # Write footer at end of sector
            # Footer: unused[116] + id(u16 @ 0xFF4) + checksum(u16 @ 0xFF6) + signature(u32 @ 0xFF8) + counter(u32 @ 0xFFC)
            struct.pack_into('<H', flash, sector_off + 0xFF4, sid)
            struct.pack_into('<H', flash, sector_off + 0xFF6, checksum(src, len(src)))
            struct.pack_into('<I', flash, sector_off + 0xFF8, SECTOR_SIG)
            struct.pack_into('<I', flash, sector_off + 0xFFC, counter if slot == 0 else counter - 1)

    return bytes(flash)

if __name__ == '__main__':
    save = build()
    out = 'pokefirered.sav' if len(sys.argv) < 2 else sys.argv[1]
    open(out, 'wb').write(save)
    print(f"Generated {out} ({len(save)} bytes)")
    print("Team: Charizard/Blastoise/Venusaur/Snorlax/Raichu/Pidgeot Lv75")
    print("National Dex 386/386, all badges, E4 clear, 999999 money")
