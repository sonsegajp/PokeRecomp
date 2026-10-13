#!/usr/bin/env python3
"""One-time edits to pret's GBA headers for the PC build (idempotent).

Every change is wrapped in `#ifdef PLATFORM_PC` so the GBA build still works."""
from pathlib import Path

GAME = Path(__file__).resolve().parent.parent / "game"


def patch(rel, old, new):
    p = GAME / rel
    s = p.read_text(encoding="utf-8")
    if new in s:
        return
    if old not in s:
        raise SystemExit(f"{rel}: anchor not found:\n{old}")
    p.write_text(s.replace(old, new, 1), encoding="utf-8")
    print("patched", rel)


patch("include/gba/defines.h",
'''#if defined(__APPLE__)
#define IWRAM_DATA __attribute__((section("__DATA,iwram_data")))''',
'''#ifdef PLATFORM_PC
#include "pc_hw.h"
#define IWRAM_DATA
#define EWRAM_DATA
#define COMMON_DATA
#elif defined(__APPLE__)
#define IWRAM_DATA __attribute__((section("__DATA,iwram_data")))''')

patch("include/gba/defines.h",
'''#define COMMON_DATA __attribute__((section("common_data")))''',
'''#ifndef PLATFORM_PC
#define COMMON_DATA __attribute__((section("common_data")))
#endif''')

patch("include/gba/defines.h",
'''#define SOUND_INFO_PTR (*(struct SoundInfo **)0x3007FF0)
#define INTR_CHECK     (*(u16 *)0x3007FF8)
#define INTR_VECTOR    (*(void **)0x3007FFC)''',
'''#ifdef PLATFORM_PC
#define SOUND_INFO_PTR (*(struct SoundInfo **)&gPcSoundInfoPtr)
#define INTR_CHECK     gPcIntrCheck
#define INTR_VECTOR    gPcIntrVector
#else
#define SOUND_INFO_PTR (*(struct SoundInfo **)0x3007FF0)
#define INTR_CHECK     (*(u16 *)0x3007FF8)
#define INTR_VECTOR    (*(void **)0x3007FFC)
#endif''')

patch("include/gba/defines.h",
'''#define PLTT          0x5000000''',
'''#ifdef PLATFORM_PC
#define PLTT          ((unsigned int)gPcPltt)
#else
#define PLTT          0x5000000
#endif''')

patch("include/gba/defines.h",
'''#define VRAM      0x6000000''',
'''#ifdef PLATFORM_PC
#define VRAM      ((unsigned int)gPcVram)
#else
#define VRAM      0x6000000
#endif''')

patch("include/gba/defines.h",
'''#define OAM      0x7000000''',
'''#ifdef PLATFORM_PC
#define OAM      ((unsigned int)gPcOam)
#else
#define OAM      0x7000000
#endif''')

patch("include/gba/io_reg.h",
'''#define REG_BASE 0x4000000 // I/O register base address''',
'''#ifdef PLATFORM_PC
#include "pc_hw.h"
#define REG_BASE ((unsigned int)gPcIo) // I/O registers live in host memory
#else
#define REG_BASE 0x4000000 // I/O register base address
#endif''')

patch("include/gba/macro.h",
'''#define DmaSet(dmaNum, src, dest, control)        \\
{                                                 \\''',
'''#ifdef PLATFORM_PC
#define DmaSet(dmaNum, src, dest, control) PcDmaSet(dmaNum, (const void *)(src), (void *)(dest), (control))
#else
#define DmaSet(dmaNum, src, dest, control)        \\
{                                                 \\''')

patch("include/gba/macro.h",
'''    dmaRegs[2];                                   \\
}
''',
'''    dmaRegs[2];                                   \\
}
#endif
''')

patch("include/gba/macro.h",
'''#define DmaStop(dmaNum)                                         \\
{                                                               \\''',
'''#ifdef PLATFORM_PC
#define DmaStop(dmaNum) PcDmaStop(dmaNum)
#else
#define DmaStop(dmaNum)                                         \\
{                                                               \\''')

patch("include/gba/macro.h",
'''    dmaRegs[5];                                                 \\
}
''',
'''    dmaRegs[5];                                                 \\
}
#endif
''')
