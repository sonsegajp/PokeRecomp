"""gen_visitor_gfx.py — the other region's player sprites, for visitors.

A trainer visiting from another region (shared/pc_travel.c) is drawn with
their own game's player sprites in every state. This copies, from the other
decomp, exactly what those sprites are made of: the sheets and palettes
(INCBIN'd from that decomp's built graphics), the frame tables and the
animation tables, renamed with an sX_ prefix so they live beside the host
game's own. It writes:

    xgfx/visitor_emerald.h   Brendan / May, for FireRed and LeafGreen
    xgfx/visitor_firered.h   Red / Leaf (Green), for Emerald

and each ends in gPcVisitorStates / gPcVisitorPalettes (xgfx/pc_visitor_gfx.h).
The sheets and palettes are cartridge content: tools/rompack.c takes them
out of the built DLL and they come back from the player's own copy of the
other game (the "foreign" pack).

    python tools/gen_visitor_gfx.py
"""
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# per foreign game: its decomp, how its graphics paths become files, and the
# player's states (normal, bike, surf, field move, fish) for each gender
GAMES = {
    'emerald': dict(
        dir='emerald',
        out='xgfx/visitor_emerald.h',
        who='Brendan and May (Emerald)',
        states=[
            # (pic table, anim table, width, height, size)
            [('sPicTable_BrendanNormal', 'sAnimTable_BrendanMayNormal', 16, 32, 512),
             ('sPicTable_BrendanMachBike', 'sAnimTable_Standard', 32, 32, 512),
             ('sPicTable_BrendanSurfing', 'sAnimTable_Surfing', 32, 32, 512),
             ('sPicTable_BrendanFieldMove', 'sAnimTable_FieldMove', 32, 32, 512),
             ('sPicTable_BrendanFishing', 'sAnimTable_Fishing', 32, 32, 512)],
            [('sPicTable_MayNormal', 'sAnimTable_BrendanMayNormal', 16, 32, 512),
             ('sPicTable_MayMachBike', 'sAnimTable_Standard', 32, 32, 512),
             ('sPicTable_MaySurfing', 'sAnimTable_Surfing', 32, 32, 512),
             ('sPicTable_MayFieldMove', 'sAnimTable_FieldMove', 32, 32, 512),
             ('sPicTable_MayFishing', 'sAnimTable_Fishing', 32, 32, 512)],
        ],
        palettes=[('gObjectEventPal_Brendan', 'gObjectEventPal_BrendanReflection'),
                  ('gObjectEventPal_May', 'gObjectEventPal_MayReflection')],
        # battle back pics (4 frames), trainer card front pics and their palette:
        # FireRed only has Ruby/Sapphire's Brendan and May
        trainers=[('graphics/trainers/back_pics/brendan.png.4bpp', 4,
                   'graphics/trainers/front_pics/brendan.png.4bpp.lz',
                   'graphics/trainers/palettes/brendan.pal.gbapal.lz'),
                  ('graphics/trainers/back_pics/may.png.4bpp', 4,
                   'graphics/trainers/front_pics/may.png.4bpp.lz',
                   'graphics/trainers/palettes/may.pal.gbapal.lz')],
    ),
    'firered': dict(
        dir='game',
        out='xgfx/visitor_firered.h',
        who='Red and Leaf (FireRed / LeafGreen)',
        states=[
            [('sPicTable_RedNormal', 'sAnimTable_RedGreenNormal', 16, 32, 512),
             ('sPicTable_RedBike', 'sAnimTable_Standard', 32, 32, 512),
             ('sPicTable_RedSurf', 'sAnimTable_RedGreenSurf', 16, 32, 256),
             ('sPicTable_RedItem', 'sAnimTable_RedGreenFieldMove', 16, 32, 256),
             ('sPicTable_RedFish', 'sAnimTable_RedGreenFish', 32, 32, 512)],
            [('sPicTable_GreenNormal', 'sAnimTable_RedGreenNormal', 16, 32, 512),
             ('sPicTable_GreenBike', 'sAnimTable_Standard', 32, 32, 512),
             ('sPicTable_GreenSurf', 'sAnimTable_RedGreenSurf', 16, 32, 256),
             ('sPicTable_GreenItem', 'sAnimTable_RedGreenFieldMove', 16, 32, 256),
             ('sPicTable_GreenFish', 'sAnimTable_RedGreenFish', 32, 32, 512)],
        ],
        palettes=[('gObjectEventPal_Player', 'gObjectEventPal_PlayerReflection'),
                  ('gObjectEventPal_Player', 'gObjectEventPal_PlayerReflection')],
    ),
}


def read(game, name):
    with open(os.path.join(ROOT, game['dir'], 'src/data/object_events', name), encoding='utf-8') as f:
        return f.read()


def blocks(text, kind):
    """name -> definition text, for `static const <kind> NAME[] = { ... };`."""
    out = {}
    for m in re.finditer(r'(?:static )?const ' + kind + r'\s+(\w+)\[\]\s*=\s*\{(.*?)\n\};', text, re.S):
        out[m.group(1)] = m.group(2)
    return out


def asset(game, line):
    """One graphics array's file, as a path from the host game's directory."""
    m = re.search(r'INCGFX_U(\d+)\("([^"]+)",\s*"([^"]+)"(?:,\s*"([^"]*)")?\)', line)
    if m:   # Emerald: converted at build time into build/assets/<path><args><ext>
        bits, path, ext, args = m.group(1), m.group(2), m.group(3), m.group(4) or ''
        return bits, '../%s/build/assets/%s%s%s' % (game['dir'], path, args.replace('-', '_').replace(' ', '_'), ext)
    m = re.search(r'INCBIN_U(\d+)\("([^"]+)"\)', line)
    return m.group(1), '../%s/%s' % (game['dir'], m.group(2))


def gen(key):
    game = GAMES[key]
    anims_src = read(game, 'object_event_anims.h')
    pics_src = read(game, 'object_event_pic_tables.h')
    gfx_src = read(game, 'object_event_graphics.h')
    anim_cmds = blocks(anims_src, r'union AnimCmd')
    anim_tables = blocks(anims_src, r'union AnimCmd \*const')
    pic_tables = blocks(pics_src, r'struct SpriteFrameImage')
    gfx_lines = {m.group(1): m.group(0) for m in re.finditer(r'const u(?:16|32) (\w+)\[\] = .*?;', gfx_src)}

    want_pics = sorted({s[0] for g in game['states'] for s in g})
    want_tables = sorted({s[1] for g in game['states'] for s in g})
    want_cmds = set()
    for t in want_tables:
        want_cmds |= set(re.findall(r'\b(sAnim_\w+)', anim_tables[t]))
    want_gfx = set()
    for p in want_pics:
        want_gfx |= set(re.findall(r'\b(gObjectEventPic_\w+)', pic_tables[p]))

    x = lambda name: 'sX_' + name.lstrip('sg').lstrip('_')
    out = ['// Generated by tools/gen_visitor_gfx.py: %s, from the %s decomp.' % (game['who'], game['dir']),
           '// Sheets and palettes are cartridge content (taken out of the DLL by tools/rompack.c).', '']
    for g in sorted(want_gfx):
        bits, path = asset(game, gfx_lines[g])
        out.append('static const u%s %s[] = INCBIN_U%s("%s");' % (bits, x(g), bits, path))
    out.append('')
    for c in sorted(want_cmds):
        out.append('static const union AnimCmd %s[] = {%s\n};' % (x(c), anim_cmds[c]))
    out.append('')
    for t in want_tables:
        body = re.sub(r'\b(sAnim_\w+)', lambda m: x(m.group(1)), anim_tables[t])
        out.append('static const union AnimCmd *const %s[] = {%s\n};' % (x(t), body))
    out.append('')
    for p in want_pics:
        body = re.sub(r'\b(gObjectEventPic_\w+)', lambda m: x(m.group(1)), pic_tables[p])
        out.append('static const struct SpriteFrameImage %s[] = {%s\n};' % (x(p), body))
    out.append('')
    out.append('const struct PcVisitorState gPcVisitorStates[2][PC_VISITOR_STATE_COUNT] = {')
    for g in game['states']:
        out.append('    {')
        for pic, anims, w, h, size in g:
            out.append("        {%s, %s, ARRAY_COUNT(%s), %d, %d, %d}," % (x(pic), x(anims), x(anims), size, w, h))
        out.append('    },')
    out.append('};')
    # the palettes again as one array the game's palette tables can point into:
    # [gender * 2] sprite, [gender * 2 + 1] reflection
    out.append('const u16 gPcVisitorPal[4][16] = {')
    for pal, refl in game['palettes']:
        for n in (pal, refl):
            bits, path = asset(game, gfx_lines[n])
            out.append('    INCBIN_U16("%s"),' % path)
    out.append('};')
    if game.get('trainers'):
        out.append('')
        out.append('// battle back pics, trainer card pics and their palettes')
        for g, (back, frames, front, pal) in zip(('M', 'F'), game['trainers']):
            root = '../%s/build/assets/' % game['dir']
            out.append('const u8 gPcVisitorBackPic_%s[] = INCBIN_U8("%s%s");' % (g, root, back))
            out.append('const u32 gPcVisitorFrontPic_%s[] = INCBIN_U32("%s%s");' % (g, root, front))
            out.append('const u32 gPcVisitorTrainerPalette_%s[] = INCBIN_U32("%s%s");' % (g, root, pal))
        out.append('const struct SpriteFrameImage gPcVisitorBackPicFrames[2][4] = {')
        for g, (back, frames, front, pal) in zip(('M', 'F'), game['trainers']):
            out.append('    {' + ', '.join('{gPcVisitorBackPic_%s + 0x800 * %d, 0x800}' % (g, i) for i in range(frames)) + '},')
        out.append('};')
    path = os.path.join(ROOT, game['out'])
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(out) + '\n')
    print('%s: %d sheets/palettes, %d anims, %d anim tables, %d frame tables'
          % (game['out'], len(want_gfx), len(want_cmds), len(want_tables), len(want_pics)))


for k in GAMES:
    gen(k)
